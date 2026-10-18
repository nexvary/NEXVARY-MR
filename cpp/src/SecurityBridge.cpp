#include "SecurityBridge.hpp"
#include "WorkshopBridge.hpp"
#include "RecoveryFile.hpp"
#include <QtConcurrent>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QProcess>
#include <QElapsedTimer>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
namespace {
bool localFile(const QString &p){const QFileInfo f(p);return QDir::isAbsolutePath(p)&&!p.startsWith("//")&&!p.startsWith("\\\\")&&f.isFile()&&!f.isSymLink();}
QByteArray digest(const QString &p,qint64 limit,const std::shared_ptr<std::atomic_bool> &cancel){
 QFile f(p);if(!localFile(p)||!mr::recovery::openRegularInput(f)||f.size()>limit)return {};
 QCryptographicHash hash(QCryptographicHash::Sha256);qint64 size=0;
 while(!f.atEnd()){if(cancel&&cancel->load())return {};const auto b=f.read(1024*1024);size+=b.size();if((b.isEmpty()&&f.error()!=QFileDevice::NoError)||size>limit)return {};hash.addData(b);}
 return hash.result().toHex();
}
struct ProcessResult {QByteArray out;int exit=-1;QString error;};
ProcessResult run(const QString &engine,const QStringList &args,const QString &work,const std::shared_ptr<std::atomic_bool> &cancel,const SecurityLimits &limits){
 ProcessResult r;QProcess p;p.setProgram(engine);p.setArguments(args);p.setWorkingDirectory(work);p.setProcessChannelMode(QProcess::MergedChannels);p.start();
 if(!p.waitForStarted(3000)){r.error="Cannot start the selected ClamAV executable.";return r;}p.closeWriteChannel();QElapsedTimer timer;timer.start();
 auto collect=[&]{r.out+=p.read(qMax<qsizetype>(0,limits.maximumOutput-r.out.size())+1);if(r.out.size()>limits.maximumOutput){r.error="Scanner output limit exceeded.";p.kill();}};
 QObject::connect(&p,&QProcess::readyReadStandardOutput,&p,collect);
 while(p.state()!=QProcess::NotRunning){p.waitForFinished(50);collect();if(cancel&&cancel->load()){r.error="Cancelled; no scan verdict.";break;}if(timer.elapsed()>limits.timeoutMs){r.error="Scanner timed out; no scan verdict.";break;}if(!r.error.isEmpty())break;}
 if(!r.error.isEmpty()){p.kill();p.waitForFinished(3000);return r;}collect();if(p.exitStatus()!=QProcess::NormalExit)r.error="Scanner crashed; no scan verdict.";r.exit=p.exitCode();return r;
}
}
QVariantMap scanSecurityFile(const QString &source,const QString &engine,const QString &database,const std::shared_ptr<std::atomic_bool> &cancel,const SecurityLimits &limits){
 QVariantMap r{{"reportType","nexvary-file-security-scan"},{"atUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"source",source},{"scope","One authorized local file/Android APK copy only. No device-wide, memory or encrypted-data scan. Bytecode disabled. No remediation, quarantine or upload."}};
 auto fail=[&](const QString &message){r.insert("error",message);r.insert("verdict","Unavailable / incomplete");return r;};
 if(!localFile(source)||!localFile(engine)||!QFileInfo(engine).isExecutable()||QFileInfo(source).size()>limits.maximumFile||QFileInfo(source).size()==0)return fail("Select a nonempty regular local file up to 128 MiB and a trusted installed clamscan executable.");
#ifdef Q_OS_WIN
 if(QFileInfo(engine).suffix().compare("exe",Qt::CaseInsensitive)!=0)return fail("Choose a native clamscan EXE, not a shell script.");
#endif
 if(!QDir::isAbsolutePath(database)||database.startsWith("//")||database.startsWith("\\\\")||!QFileInfo(database).isDir()||QFileInfo(database).isSymLink())return fail("Choose a local ClamAV signature directory.");
 const auto files=QDir(database).entryInfoList(QDir::Files|QDir::Hidden,QDir::Name);
 const auto signatures=QDir(database).entryList({"*.cvd","*.cld","*.ndb","*.hdb","*.ldb","*.yar","*.yara"},QDir::Files);
 if(signatures.isEmpty()||files.size()>32)return fail("Signature directory must contain 1–32 supported ClamAV database files.");
 QVariantList manifest;bool officialContainer=false;qint64 dbSize=0;
 for(const auto &f:files){dbSize+=f.size();if(dbSize>1024LL*1024*1024)return fail("Signature manifest exceeds 1 GiB.");const auto h=digest(f.absoluteFilePath(),512LL*1024*1024,cancel);if(h.isEmpty())return fail("Could not hash signature database or operation cancelled.");manifest.append(QVariantMap{{"name",f.fileName()},{"sha256",QString::fromLatin1(h)},{"modifiedAtUtc",f.lastModified().toUTC().toString(Qt::ISODate)}});officialContainer=officialContainer||f.suffix()=="cvd"||f.suffix()=="cld";}
 r.insert("signatureFiles",manifest);r.insert("databaseScope",officialContainer?"CVD/CLD containers present; authenticity/freshness checked by ClamAV, not by filenames.":"Custom signatures only; not the official threat database. Test/custom coverage only.");
 const auto sourceHash=digest(source,limits.maximumFile,cancel),engineHash=digest(engine,256LL*1024*1024,cancel);if(sourceHash.isEmpty()||engineHash.isEmpty())return fail("Input/tool hash failed or operation cancelled.");
 r.insert("sha256",QString::fromLatin1(sourceHash));r.insert("engineSha256",QString::fromLatin1(engineHash));
 QTemporaryDir work(QDir::tempPath()+"/nexvary-security-XXXXXX");if(!work.isValid())return fail("Cannot create private scan workspace.");QFile::setPermissions(work.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
 const auto copy=work.filePath("scan-input."+QFileInfo(source).suffix().left(12));
 // Only the private copy is ever given to the scanner; no remove/move/quarantine flags.
 QFile input(source),output(copy);if(!mr::recovery::openRegularInput(input)||!output.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail("Cannot stage the authorized file.");output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);qint64 count=0;
 while(!input.atEnd()){if(cancel&&cancel->load())return fail("Cancelled; no scan verdict.");const auto b=input.read(1024*1024);count+=b.size();if((b.isEmpty()&&input.error()!=QFileDevice::NoError)||count>limits.maximumFile||output.write(b)!=b.size())return fail("Source disconnected or staging failed.");}
 if(!output.flush()){return fail("Cannot flush working copy.");}
 input.close();output.close();if(digest(copy,limits.maximumFile,cancel)!=sourceHash)return fail("Working-copy integrity mismatch.");
 auto version=run(engine,{"--version"},work.path(),cancel,limits);if(!version.error.isEmpty()||version.exit!=0||!version.out.trimmed().startsWith("ClamAV ")||version.out.size()>1000)return fail("Selected executable did not return a ClamAV version.");r.insert("engineVersion",QString::fromUtf8(version.out).trimmed());
 const QStringList args={"--stdout","--database="+database,"--recursive=no","--max-filesize=128M","--max-scansize=256M","--max-files=1000","--max-recursion=16","--max-scantime=60000","--alert-exceeds-max=yes","--alert-encrypted=yes","--bytecode=no","--fail-if-cvd-older-than=7","--",copy};
 const auto result=run(engine,args,work.path(),cancel,limits);if(!result.error.isEmpty())return fail(result.error);r.insert("output",QString::fromUtf8(result.out));
 if(digest(source,limits.maximumFile,cancel)!=sourceHash||digest(copy,limits.maximumFile,cancel)!=sourceHash||digest(engine,256LL*1024*1024,cancel)!=engineHash)return fail("Source, working copy or scanner changed during the scan; verdict refused.");
 const auto after=QDir(database).entryInfoList(QDir::Files|QDir::Hidden,QDir::Name);if(after.size()!=files.size())return fail("Signatures changed during the scan; retry after database update completes.");
 for(int i=0;i<after.size();++i)if(after[i].fileName()!=files[i].fileName()||digest(after[i].absoluteFilePath(),512LL*1024*1024,cancel)!=manifest[i].toMap().value("sha256").toString().toLatin1())return fail("Signature database changed during the scan; verdict refused.");
 r.insert("sourceUnchanged",true);r.insert("exitCode",result.exit);
 if(result.exit==1&&result.out.contains(" FOUND"))r.insert("verdict",result.out.contains("Heuristics.Limits.")||result.out.contains("Heuristics.Encrypted.")?"Limited/encrypted content — review scanner alert":"Signature alert — technician review required");
 else if(result.exit==0&&result.out.contains(": OK")&&!result.out.contains("ERROR"))r.insert("verdict",officialContainer?"No signature matched this file; not proof the phone is clean":"No custom signature matched; test/custom coverage only");
 else return fail("ClamAV reported a scan/database error or no recognized result; no clean verdict.");
 return r;
}
SecurityBridge::SecurityBridge(const QString &config,QObject *parent):QObject(parent),config_(config){
 QFile f(config);if(f.size()<16384&&f.open(QIODevice::ReadOnly)){const auto o=QJsonDocument::fromJson(f.readAll()).object();executable_=o.value("executable").toString();database_=o.value("database").toString();}
 connect(&watcher_,&QFutureWatcher<QVariantMap>::finished,this,[this]{result_=watcher_.result();active_=false;error_=result_.value("error").toString();emit changed();});
}
SecurityBridge::~SecurityBridge(){cancel();watcher_.waitForFinished();}
bool SecurityBridge::configure(const QString &executable,const QString &database){
 if(busy()){return false;}
 const auto e=WorkshopBridge::localPath(executable),d=WorkshopBridge::localPath(database);
 if(!localFile(e)||!QFileInfo(e).isExecutable()||!QDir::isAbsolutePath(d)||!QFileInfo(d).isDir()){error_="Select installed clamscan and a signature directory.";emit changed();return false;}
 QSaveFile f(config_);const auto b=QJsonDocument(QJsonObject{{"executable",e},{"database",d}}).toJson();if(!f.open(QIODevice::WriteOnly)||f.write(b)!=b.size()||!f.commit()){error_="Cannot save scanner configuration.";emit changed();return false;}executable_=e;database_=d;error_.clear();emit changed();return true;
}
bool SecurityBridge::scan(const QString &source,bool authorized){
 if(busy()){return false;}
 if(!authorized){error_="Explicit authorization is required to scan the selected file.";emit changed();return false;}result_.clear();error_.clear();cancelled_=std::make_shared<std::atomic_bool>(false);
 const auto path=WorkshopBridge::localPath(source),e=executable_,d=database_;const auto token=cancelled_;active_=true;watcher_.setFuture(QtConcurrent::run([=]{return scanSecurityFile(path,e,d,token);}));emit changed();return true;
}
void SecurityBridge::cancel(){if(cancelled_)cancelled_->store(true);}
bool SecurityBridge::exportReport(const QString &path,bool consent){
 const auto target=WorkshopBridge::localPath(path);if(busy()||!consent||result_.isEmpty()||target.isEmpty()||QFileInfo::exists(target)){error_="Choose a new report filename and approve exporting source paths/results.";emit changed();return false;}
 QFile f(target);const auto b=QJsonDocument::fromVariant(result_).toJson();if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly)||f.write(b)!=b.size()){error_="Could not save security report.";emit changed();return false;}f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);error_.clear();emit changed();return true;
}
