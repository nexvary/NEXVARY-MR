#include "SecurityBridge.hpp"
#include "GovernmentOperationPolicy.hpp"
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
#include <QLockFile>
namespace {
QString lockPath(const QString &directory){return QDir(directory).filePath(".nexvary-security.lock");}
bool localDirectory(const QString &path){return QDir::isAbsolutePath(path)&&!path.startsWith("//")&&!path.startsWith("\\\\")&&QFileInfo(path).isDir()&&!QFileInfo(path).isSymLink();}
QFileInfoList databaseFiles(const QString &path){
 auto files=QDir(path).entryInfoList(QDir::Files|QDir::Hidden,QDir::Name);
 files.removeIf([](const QFileInfo &file){return file.fileName()==QStringLiteral(".nexvary-security.lock");});
 return files;
}
bool localFile(const QString &p){const QFileInfo f(p);return QDir::isAbsolutePath(p)&&!p.startsWith("//")&&!p.startsWith("\\\\")&&f.isFile()&&!f.isSymLink();}
QByteArray digest(const QString &p,qint64 limit,const std::shared_ptr<std::atomic_bool> &cancel){
 QFile f(p);if(!localFile(p)||!mr::recovery::openRegularInput(f)||f.size()>limit)return {};
 QCryptographicHash hash(QCryptographicHash::Sha256);qint64 size=0;
 while(!f.atEnd()){if(cancel&&cancel->load())return {};const auto b=f.read(1024*1024);size+=b.size();if((b.isEmpty()&&f.error()!=QFileDevice::NoError)||size>limit)return {};hash.addData(b);}
 return hash.result().toHex();
}
struct ProcessResult {QByteArray out;int exit=-1;QString error;};
ProcessResult run(const QString &engine,const QStringList &args,const QString &work,const std::shared_ptr<std::atomic_bool> &cancel,const SecurityLimits &limits){
 ProcessResult r;if(cancel&&cancel->load()){r.error="Cancelled before child launch; no scan verdict.";return r;}QProcess p;p.setProgram(engine);p.setArguments(args);p.setWorkingDirectory(work);p.setProcessChannelMode(QProcess::MergedChannels);
 const int budget=qMax(1,limits.timeoutMs);QElapsedTimer timer;timer.start();p.start();
 auto stop=[&]{if(p.state()!=QProcess::NotRunning){p.kill();p.waitForFinished(3000);}};
 if(!p.waitForStarted(qMin(3000,budget))){r.error=timer.elapsed()>=budget?QStringLiteral("Scanner timed out; no scan verdict."):QStringLiteral("Cannot start the selected ClamAV executable.");stop();return r;}
 p.closeWriteChannel();
 auto collect=[&]{r.out+=p.read(qMax<qsizetype>(0,limits.maximumOutput-r.out.size())+1);if(r.out.size()>limits.maximumOutput){r.error="Scanner output limit exceeded.";p.kill();}};
 auto check=[&]{if(!r.error.isEmpty())return;if(cancel&&cancel->load())r.error="Cancelled; no scan verdict.";else if(timer.elapsed()>=budget)r.error="Scanner timed out; no scan verdict.";};
 QObject::connect(&p,&QProcess::readyReadStandardOutput,&p,collect);
 // The clock includes startup and is checked even if the child has already
 // exited before the parent resumes; a late result must not become a verdict.
 check();
 while(r.error.isEmpty()&&p.state()!=QProcess::NotRunning){p.waitForFinished(50);collect();check();}
 collect();check();if(!r.error.isEmpty()){stop();return r;}
 if(p.exitStatus()!=QProcess::NormalExit)r.error="Scanner crashed; no scan verdict.";
 r.exit=p.exitCode();return r;
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
 QLockFile databaseLock(lockPath(database));databaseLock.setStaleLockTime(0);
 if(!databaseLock.tryLock())return fail("Signature directory is busy or not writable; close the other NEXVARY scan/update and retry.");
 const auto files=databaseFiles(database);
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
 // Version startup and file analysis have separate deadlines. A short scan
 // budget must not accidentally test Windows DLL startup instead of scanning.
 auto versionLimits=limits;versionLimits.timeoutMs=qBound(1,limits.versionTimeoutMs,3000);
 const auto version=run(engine,{"--version"},work.path(),cancel,versionLimits);
 if(!version.error.isEmpty())return fail("ClamAV version check: "+version.error);
 if(version.exit!=0||!version.out.trimmed().startsWith("ClamAV ")||version.out.size()>1000)return fail("Selected executable did not return a ClamAV version.");
 r.insert("engineVersion",QString::fromUtf8(version.out).trimmed());
 const QStringList args={"--stdout","--database="+database,"--recursive=no","--max-filesize=128M","--max-scansize=256M","--max-files=1000","--max-recursion=16","--max-scantime=60000","--alert-exceeds-max=yes","--alert-encrypted=yes","--bytecode=no","--fail-if-cvd-older-than=7","--",copy};
 const auto result=run(engine,args,work.path(),cancel,limits);if(!result.error.isEmpty())return fail(result.error);r.insert("output",QString::fromUtf8(result.out));
 if(digest(source,limits.maximumFile,cancel)!=sourceHash||digest(copy,limits.maximumFile,cancel)!=sourceHash||digest(engine,256LL*1024*1024,cancel)!=engineHash)return fail("Source, working copy or scanner changed during the scan; verdict refused.");
 const auto after=databaseFiles(database);if(after.size()!=files.size())return fail("Signatures changed during the scan; retry after database update completes.");
 for(int i=0;i<after.size();++i)if(after[i].fileName()!=files[i].fileName()||digest(after[i].absoluteFilePath(),512LL*1024*1024,cancel)!=manifest[i].toMap().value("sha256").toString().toLatin1())return fail("Signature database changed during the scan; verdict refused.");
 r.insert("sourceUnchanged",true);r.insert("exitCode",result.exit);
 if(result.exit==1&&result.out.contains(" FOUND"))r.insert("verdict",result.out.contains("Heuristics.Limits.")||result.out.contains("Heuristics.Encrypted.")?"Limited/encrypted content — review scanner alert":"Signature alert — technician review required");
 else if(result.exit==0&&result.out.contains(": OK")&&!result.out.contains("ERROR"))r.insert("verdict",officialContainer?"No signature matched this file; not proof the phone is clean":"No custom signature matched; test/custom coverage only");
 else return fail("ClamAV reported a scan/database error or no recognized result; no clean verdict.");
 return r;
}
QVariantMap updateSecurityDatabase(const QString &updater,const QString &database,const std::shared_ptr<std::atomic_bool> &cancel,int timeoutMs){
 QVariantMap report{{"reportType","nexvary-signature-update"},{"atUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"database",database},{"success",false},{"scope","Official ClamAV database download only; no customer file or APK is passed to FreshClam. This is not a scan verdict."}};
 auto fail=[&](const QString &message){report.insert("error",message);return report;};
 if(!localFile(updater)||!QFileInfo(updater).isExecutable()||!localDirectory(database))return fail("Install trusted FreshClam and choose an existing writable local signature directory.");
#ifdef Q_OS_WIN
 if(QFileInfo(updater).suffix().compare("exe",Qt::CaseInsensitive)!=0)return fail("Choose a native FreshClam EXE.");
#endif
 QLockFile databaseLock(lockPath(database));databaseLock.setStaleLockTime(0);
 if(!databaseLock.tryLock())return fail("Signature directory is busy or not writable; close the other NEXVARY scan/update and retry.");
 for(const auto &entry:QDir(database).entryInfoList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot))if(entry.isSymLink())return fail("Signature directory contains a symlink; choose a dedicated local directory.");
 const auto engineHash=digest(updater,256LL*1024*1024,cancel);if(engineHash.isEmpty())return fail("Updater hash failed or operation cancelled.");
 report.insert("updaterSha256",QString::fromLatin1(engineHash));
 QTemporaryDir work(QDir::tempPath()+"/nexvary-update-XXXXXX");if(!work.isValid())return fail("Cannot create a private update workspace.");
 QFile::setPermissions(work.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
 const auto configuration=work.filePath("freshclam.conf");QFile config(configuration);
 // No imported configuration, custom mirrors, shell hooks, daemon or telemetry
 // arguments. Paths are passed as separate QProcess arguments, never shell text.
 const QByteArray settings="DatabaseMirror database.clamav.net\nDNSDatabaseInfo current.cvd.clamav.net\nMaxAttempts 1\nConnectTimeout 30\nReceiveTimeout 60\nTestDatabases yes\nBytecode no\n";
 if(!config.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail("Cannot create the update configuration.");
 config.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
 if(config.write(settings)!=settings.size()||!config.flush())return fail("Cannot write the update configuration.");
 config.close();
 SecurityLimits limits;limits.timeoutMs=3000;
 const auto version=run(updater,{"--version"},work.path(),cancel,limits);
 if(!version.error.isEmpty())return fail("FreshClam version check: "+version.error);
 if(version.exit!=0||!version.out.trimmed().startsWith("ClamAV ")||version.out.size()>1000)return fail("Selected updater did not return a ClamAV version.");
 report.insert("updaterVersion",QString::fromUtf8(version.out).trimmed());limits.timeoutMs=qBound(1,timeoutMs,600000);
 const auto result=run(updater,{"--stdout","--config-file="+configuration,"--datadir="+database},work.path(),cancel,limits);
 report.insert("output",QString::fromUtf8(result.out));report.insert("exitCode",result.exit);
 if(!result.error.isEmpty())return fail("Update incomplete: "+result.error+" Retry before scanning; already replaced databases are not rolled back.");
 if(result.exit!=0)return fail("FreshClam update failed. Review its log for network, permissions, signature or CDN rate-limit errors; no automatic retry was scheduled.");
 if(digest(updater,256LL*1024*1024,cancel)!=engineHash)return fail("Updater changed or operation cancelled; update result refused.");
 QVariantList manifest;
 for(const auto &base:{QStringLiteral("main"),QStringLiteral("daily")}){
  QString path;
  for(const auto &suffix:{QStringLiteral(".cvd"),QStringLiteral(".cld")}){const auto candidate=QDir(database).filePath(base+suffix);if(QFileInfo::exists(candidate)){if(!path.isEmpty())return fail("Both CVD and CLD copies exist; review the database directory.");path=candidate;}}
  const auto hash=digest(path,512LL*1024*1024,cancel);if(hash.isEmpty()||QFileInfo(path).size()==0)return fail("FreshClam exited without a complete readable main/daily database set.");
  manifest.append(QVariantMap{{"name",QFileInfo(path).fileName()},{"bytes",QFileInfo(path).size()},{"sha256",QString::fromLatin1(hash)}});
 }
 report.insert("signatureFiles",manifest);report.insert("success",true);report.insert("completedAtUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
 report.insert("status","FreshClam completed and main/daily database hashes were recorded. Signature verification is delegated to the installed FreshClam; scan files separately.");return report;
}
QString SecurityBridge::updater() const {
 if(executable_.isEmpty())return {};
#ifdef Q_OS_WIN
 const auto candidate=QFileInfo(executable_).dir().filePath("freshclam.exe");
#else
 const auto candidate=QFileInfo(executable_).dir().filePath("freshclam");
#endif
 return localFile(candidate)&&QFileInfo(candidate).isExecutable()?candidate:QString();
}
bool SecurityBridge::updateDatabase(bool networkConsent){
 if(policy_&&!policy_->permit("signature-update")){error_=policy_->error();emit changed();return false;}
 if(busy())return false;
 if(!networkConsent||updater().isEmpty()||!localDirectory(database_)){error_="Approve official database downloads and install FreshClam beside the selected clamscan executable.";emit changed();return false;}
 updateResult_.clear();error_.clear();cancelled_=std::make_shared<std::atomic_bool>(false);
 const auto engine=updater(),directory=database_;const auto token=cancelled_;active_=true;updating_=true;
 watcher_.setFuture(QtConcurrent::run([=]{return updateSecurityDatabase(engine,directory,token);}));emit changed();return true;
}
SecurityBridge::SecurityBridge(const QString &config,QObject *parent):QObject(parent),config_(config){
 QFile f(config);if(f.size()<16384&&f.open(QIODevice::ReadOnly)){const auto o=QJsonDocument::fromJson(f.readAll()).object();executable_=o.value("executable").toString();database_=o.value("database").toString();}
 connect(&watcher_,&QFutureWatcher<QVariantMap>::finished,this,[this]{auto completed=watcher_.result();if((cancelled_&&cancelled_->load())||(policy_&&!policy_->permit(updating_?"signature-update":"external-analysis"))){completed.remove("verdict");completed.insert("success",false);completed.insert("error","Cancelled or permission revoked; result refused. Already started external effects cannot be undone.");}if(updating_)updateResult_=completed;else result_=completed;active_=false;updating_=false;error_=completed.value("error").toString();emit changed();});
}
SecurityBridge::~SecurityBridge(){cancel();watcher_.waitForFinished();}
bool SecurityBridge::configure(const QString &executable,const QString &database){
 if(busy()){return false;}
 const auto e=WorkshopBridge::localPath(executable),d=WorkshopBridge::localPath(database);
 if(!localFile(e)||!QFileInfo(e).isExecutable()||!QDir::isAbsolutePath(d)||!QFileInfo(d).isDir()){error_="Select installed clamscan and a signature directory.";emit changed();return false;}
 QSaveFile f(config_);const auto b=QJsonDocument(QJsonObject{{"executable",e},{"database",d}}).toJson();if(!f.open(QIODevice::WriteOnly)||f.write(b)!=b.size()||!f.commit()){error_="Cannot save scanner configuration.";emit changed();return false;}executable_=e;database_=d;updateResult_.clear();error_.clear();emit changed();return true;
}
bool SecurityBridge::scan(const QString &source,bool authorized){
 if(policy_&&!policy_->permit("external-analysis")){error_=policy_->error();emit changed();return false;}
 if(busy()){return false;}
 if(!authorized){error_="Explicit authorization is required to scan the selected file.";emit changed();return false;}result_.clear();error_.clear();cancelled_=std::make_shared<std::atomic_bool>(false);
 const auto path=WorkshopBridge::localPath(source),e=executable_,d=database_;const auto token=cancelled_;active_=true;watcher_.setFuture(QtConcurrent::run([=]{return scanSecurityFile(path,e,d,token);}));emit changed();return true;
}
void SecurityBridge::cancel(){if(cancelled_)cancelled_->store(true);}
bool SecurityBridge::exportReport(const QString &path,bool consent){
 const auto target=WorkshopBridge::localPath(path);if(busy()||!consent||result_.isEmpty()||target.isEmpty()||QFileInfo::exists(target)){error_="Choose a new report filename and approve exporting source paths/results.";emit changed();return false;}
 QFile f(target);const auto b=QJsonDocument::fromVariant(result_).toJson();if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly)||f.write(b)!=b.size()){error_="Could not save security report.";emit changed();return false;}f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);error_.clear();emit changed();return true;
}

void SecurityBridge::setOperationPolicy(GovernmentOperationPolicy *policy){
 policy_=policy;
 connect(policy,&GovernmentOperationPolicy::changed,this,[this]{
  if(active_&&!policy_->granted(updating_?"signature-update":"external-analysis"))cancel();
 });
}
