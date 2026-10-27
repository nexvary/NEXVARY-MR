#include "TskImage.hpp"
#include "RecoveryFile.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QScopeGuard>
namespace mr::forensic {
namespace {
bool cancelled(const std::shared_ptr<std::atomic_bool> &token){return token&&token->load();}
QByteArray hashFile(const QString &path,qint64 maximum,QString &error,const std::shared_ptr<std::atomic_bool> &token,
 const std::function<void(qint64)> &progress={}) {
 QFile file(path);const QFileInfo before(path);
 if(!before.isFile()||before.isSymLink()||before.size()>maximum||!mr::recovery::openRegularInput(file)){error="Select a bounded regular local file, not a link or device.";return {};}
 QCryptographicHash hash(QCryptographicHash::Sha256);qint64 count=0;
 while(!file.atEnd()){
  if(cancelled(token)){error="Operation cancelled.";return {};}
  const auto bytes=file.read(1024*1024);if(bytes.isEmpty()&&file.error()!=QFileDevice::NoError){error="Source could not be read completely.";return {};}
  count+=bytes.size();if(count>maximum){error="Source grew beyond the input limit.";return {};}hash.addData(bytes);if(progress)progress(count);
 }
 const QFileInfo after(path);
 if(count!=before.size()||after.size()!=before.size()||after.lastModified()!=before.lastModified()){error="Source changed while hashing.";return {};}
 return hash.result();
}
QString run(const QString &exe,const QStringList &args,const QString &directory,const std::shared_ptr<std::atomic_bool> &token,const TskLimits &limits,QString &error){
 if(token&&token->load()){error="Cancelled before external child launch.";return {};}
 QProcess process;process.setProgram(exe);process.setArguments(args);process.setWorkingDirectory(directory);process.setProcessChannelMode(QProcess::MergedChannels);process.start();
 if(!process.waitForStarted(3000)){error="Could not start the selected mmls executable.";return {};}
 process.closeWriteChannel();QElapsedTimer timer;timer.start();QByteArray output;
 auto collect=[&]{const auto room=qMax<qsizetype>(0,limits.maximumOutput-output.size());output+=process.read(room+1);if(output.size()>limits.maximumOutput){error="Tool output exceeded the bounded capture limit.";process.kill();}};
 QObject::connect(&process,&QProcess::readyReadStandardOutput,&process,collect);
 while(process.state()!=QProcess::NotRunning){
  process.waitForFinished(50);collect();
  if(!error.isEmpty())break;
  if(cancelled(token)){error="Operation cancelled.";break;}
  if(output.size()>limits.maximumOutput){error="Tool output exceeded the bounded capture limit.";break;}
  if(timer.elapsed()>limits.processTimeoutMs){error="Tool execution timed out.";break;}
 }
 if(!error.isEmpty()){process.kill();process.waitForFinished(3000);return {};}
 collect();
 if(output.size()>limits.maximumOutput){error="Tool output exceeded the bounded capture limit.";return {};}
 if(process.exitStatus()!=QProcess::NormalExit||process.exitCode()!=0){error=QStringLiteral("mmls failed with exit code %1; no result committed.").arg(process.exitCode());return {};}
 return QString::fromUtf8(output).trimmed();
}
}
QByteArray partitionToolDigest(const QString &executable,QString &error){
 error.clear();const QFileInfo tool(executable);
 if(!QDir::isAbsolutePath(executable)||executable.startsWith("//")||executable.startsWith("\\\\")||!tool.isFile()||tool.isSymLink()||!tool.isExecutable()
#ifdef Q_OS_WIN
 ||tool.suffix().compare("exe",Qt::CaseInsensitive)!=0
#endif
 ){error="Select a trusted regular local native executable.";return {};}
 return hashFile(executable,256*1024*1024,error,{});
}
TskResult analyzePartitions(const QString &image,const QString &executable,const std::shared_ptr<std::atomic_bool> &token,
 const std::function<void(qint64,qint64)> &progress,const TskLimits &limits){
 TskResult result;
 auto stop=[&]{result.cancelled=cancelled(token);return result;};
 if(cancelled(token)){result.error="Operation cancelled.";return stop();}
 const QFileInfo source(image),tool(executable);
#ifdef Q_OS_WIN
 if(tool.suffix().compare(QStringLiteral("exe"),Qt::CaseInsensitive)!=0){result.error="Choose a native Windows EXE; shell scripts are not accepted.";return stop();}
#endif
 auto local=[](const QString &path){return QDir::isAbsolutePath(path)&&!path.startsWith("//")&&!path.startsWith("\\\\");};
 if(!local(image)||!local(executable)||!source.isFile()||source.isSymLink()||source.size()<512||source.size()>limits.maximumImage||
    !QStringList{"dd","img","raw","bin"}.contains(source.suffix().toLower())||!tool.isFile()||tool.isSymLink()||!tool.isExecutable()){
  result.error="Choose a regular local raw image (512 bytes to 2 GiB) and a trusted installed mmls executable.";return stop();
 }
 result.toolSha256=hashFile(executable,256*1024*1024,result.error,token);if(!result.error.isEmpty())return stop();
 QTemporaryDir work(QDir::tempPath()+QStringLiteral("/nexvary-mr-tsk-XXXXXX"));
 if(!work.isValid()){result.error="Could not create a private temporary analysis directory.";return stop();}
 QFile::setPermissions(work.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
 const auto stagedPath=work.filePath(QStringLiteral("working-copy.img"));
 const auto cleanup=qScopeGuard([&]{if(QFileInfo::exists(stagedPath))QFile::setPermissions(stagedPath,QFileDevice::ReadOwner|QFileDevice::WriteOwner);work.remove();});
 QFile staged(stagedPath),input(image);
 if(!mr::recovery::openRegularInput(input)||!staged.open(QIODevice::WriteOnly|QIODevice::NewOnly)){result.error="Could not prepare the evidence working copy.";return stop();}
 staged.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);QCryptographicHash hash(QCryptographicHash::Sha256);qint64 count=0;const auto total=source.size()*3;
 while(!input.atEnd()){
  if(cancelled(token)){result.error="Operation cancelled.";return stop();}
  const auto bytes=input.read(1024*1024);count+=bytes.size();
  if((bytes.isEmpty()&&input.error()!=QFileDevice::NoError)||count>limits.maximumImage||staged.write(bytes)!=bytes.size()){result.error="Source disconnected or working-copy write failed.";return stop();}
  hash.addData(bytes);if(progress)progress(count,total);
 }
 const QFileInfo afterCopy(image);
 if(count!=source.size()||afterCopy.size()!=source.size()||afterCopy.lastModified()!=source.lastModified()||!staged.flush()){result.error="Source changed or the working copy could not be flushed.";return stop();}
 input.close();staged.close();staged.setPermissions(QFileDevice::ReadOwner);result.sourceSha256=hash.result();
 const auto stageHash=hashFile(stagedPath,limits.maximumImage,result.error,token,[&](qint64 done){if(progress)progress(source.size()+done,total);});
 if(!result.error.isEmpty()||stageHash!=result.sourceSha256){if(result.error.isEmpty())result.error="Working-copy SHA256 mismatch.";return stop();}
 result.version=run(executable,{QStringLiteral("-V")},work.path(),token,limits,result.error);
 if(!result.error.isEmpty())return stop();
 if(!result.version.startsWith(QStringLiteral("The Sleuth Kit ver "))||result.version.size()>240){result.error="The selected executable did not report a supported mmls version.";return stop();}
 result.output=run(executable,{QStringLiteral("-i"),QStringLiteral("raw"),stagedPath},work.path(),token,limits,result.error);
 if(!result.error.isEmpty())return stop();
 if(result.output.isEmpty()){result.error="mmls returned an empty partition report.";return stop();}
 const auto finalStage=hashFile(stagedPath,limits.maximumImage,result.error,token);
 if(!result.error.isEmpty()||finalStage!=result.sourceSha256){if(result.error.isEmpty())result.error="Tool changed its working copy; report refused.";return stop();}
 const auto finalSource=hashFile(image,limits.maximumImage,result.error,token,[&](qint64 done){if(progress)progress(2*source.size()+done,total);});
 if(!result.error.isEmpty()||finalSource!=result.sourceSha256){if(result.error.isEmpty())result.error="Original source changed; report refused.";return stop();}
 if(hashFile(executable,256*1024*1024,result.error,token)!=result.toolSha256){if(result.error.isEmpty())result.error="Tool executable changed during analysis.";return stop();}
 QFile::setPermissions(stagedPath,QFileDevice::ReadOwner|QFileDevice::WriteOwner);
 if(!work.remove()){result.error="Temporary working-copy cleanup failed; secure the system temporary directory.";return stop();}
 result.sourceUnchanged=true;return stop();
}
}
