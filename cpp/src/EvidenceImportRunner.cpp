#include "EvidenceImportRunner.hpp"
#include "ImportAdapters.hpp"
#include "RecoveryFile.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
namespace {
QString hash(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
}
EvidenceImportRunner::EvidenceImportRunner(QObject *parent,const QString &testWorker):QObject(parent),testWorker_(testWorker){
 deadline_.setSingleShot(true);deadline_.setInterval(5000);
 connect(&deadline_,&QTimer::timeout,this,[this]{terminal_="deadline";process_.kill();});
 connect(&process_,&QProcess::readyReadStandardOutput,this,&EvidenceImportRunner::collect);
 connect(&process_,&QProcess::readyReadStandardError,this,&EvidenceImportRunner::collect);
 connect(&process_,&QProcess::finished,this,&EvidenceImportRunner::finish);
 connect(&process_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError e){if(e==QProcess::FailedToStart)finish(-1,QProcess::CrashExit);});
}
EvidenceImportRunner::~EvidenceImportRunner(){process_.disconnect(this);deadline_.stop();if(process_.state()!=QProcess::NotRunning){process_.kill();process_.waitForFinished(1000);}if(directory_)QFile::setPermissions(directory_->filePath("working.bin"),QFileDevice::ReadOwner|QFileDevice::WriteOwner);}
bool EvidenceImportRunner::start(const QByteArray &bytes,const QString &expectedSha256,const QString &adapter){
 if(active_||bytes.isEmpty()||bytes.size()>mr::forensic::importInputLimit||hash(bytes)!=expectedSha256||!mr::forensic::importAdapterSupported(adapter))return false;
 const auto temporaryRoot=mr::recovery::localFilePath(QDir::tempPath());
 if(temporaryRoot.isEmpty())return false;
 directory_=std::make_unique<QTemporaryDir>(temporaryRoot+"/nexvary-import-XXXXXX");
 if(!directory_->isValid()||!QFile::setPermissions(directory_->path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner)){directory_.reset();return false;}
 const auto path=directory_->filePath("working.bin");QFile copy(path);
 if(!copy.open(QIODevice::WriteOnly|QIODevice::NewOnly)||!copy.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||copy.write(bytes)!=bytes.size()||!copy.flush()){copy.close();directory_.reset();return false;}
 copy.close();if(!copy.setPermissions(QFileDevice::ReadOwner)||!mr::recovery::openRegularInput(copy)||hash(copy.readAll())!=expectedSha256){copy.close();directory_.reset();return false;}copy.close();
 hash_=expectedSha256;adapter_=adapter;sourceSize_=bytes.size();errorBytes_=0;terminal_.clear();output_.clear();active_=true;
 const auto worker=QDir(QCoreApplication::applicationDirPath()).filePath(
#ifdef Q_OS_WIN
 "mr_evidence_import_worker.exe"
#else
 "mr_evidence_import_worker"
#endif
 );
 auto environment=QProcessEnvironment::systemEnvironment();environment.remove("SQLITE_TMPDIR");environment.insert("TMPDIR",directory_->path());environment.insert("TEMP",directory_->path());environment.insert("TMP",directory_->path());
 process_.setProcessEnvironment(environment);process_.setProgram(testWorker_.isEmpty()?worker:testWorker_);process_.setWorkingDirectory(directory_->path());process_.setArguments({"--input",path,"--sha256",hash_,"--adapter",adapter_});
 deadline_.start();emit changed();process_.start();return true;
}
void EvidenceImportRunner::collect(){
 const auto out=process_.readAllStandardOutput(),err=process_.readAllStandardError();
 if(out.size()+err.size()>64*1024-output_.size()-errorBytes_){terminal_="output-limit";process_.kill();return;}
 output_+=out;errorBytes_+=err.size();
}
void EvidenceImportRunner::cancel(){if(active_){terminal_="cancelled";process_.kill();}}
void EvidenceImportRunner::finish(int code,QProcess::ExitStatus status){
 if(!active_)return;
 deadline_.stop();collect();QVariantMap report;
 if(terminal_.isEmpty()&&status==QProcess::NormalExit&&code==0){
  QJsonParseError parse;const auto doc=QJsonDocument::fromJson(output_,&parse);report=doc.object().toVariantMap();
  if(parse.error!=QJsonParseError::NoError||report.value("schema")!="nexvary-authorized-import/v1"||report.value("status")!="imported"||report.value("adapter")!=adapter_||report.value("sourceSha256")!=hash_||report.value("sourceSize").toLongLong()!=sourceSize_||!doc.object().value("findings").isArray()||report.value("findings").toList().size()>mr::forensic::importFindingLimit||output_.size()>mr::forensic::importReportLimit)terminal_="invalid-worker-report";
 }else if(terminal_.isEmpty())terminal_="parser-rejected";
 QFile copy(directory_->filePath("working.bin"));
 if(!mr::recovery::openRegularInput(copy)||copy.size()!=sourceSize_||hash(copy.read(mr::forensic::importInputLimit+1))!=hash_)terminal_="working-copy-integrity";
 copy.close();if(!terminal_.isEmpty())report.clear();
 // Removing a read-only file requires restoring owner-write on Windows.
 QFile::setPermissions(directory_->filePath("working.bin"),QFileDevice::ReadOwner|QFileDevice::WriteOwner);
 const bool cleaned=directory_->remove();directory_.reset();if(!cleaned){report.clear();terminal_="working-copy-cleanup";}
 active_=false;const auto error=terminal_;emit changed();emit completed(report,error);
}
