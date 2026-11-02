#include "OwnedProcess.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <vector>
#endif

struct OwnedProcess::Native {
#ifdef Q_OS_WIN
 HANDLE job=nullptr;
 STARTUPINFOEXW startup{};
 std::vector<unsigned char> attributes;
 ~Native(){if(startup.lpAttributeList)DeleteProcThreadAttributeList(startup.lpAttributeList);if(job)CloseHandle(job);}
#endif
};
OwnedProcess::OwnedProcess(QObject *parent):QProcess(parent){
 connect(this,&QProcess::finished,this,[this]{collectReceipt();});
}
OwnedProcess::~OwnedProcess(){
 if(state()!=NotRunning){stop("owner-destruction");waitForFinished(3000);}
}
void OwnedProcess::start(OpenMode mode){
 if(state()!=NotRunning)return;
 receipt_={{"schema","nexvary-process-cleanup/v1"},{"cleanupComplete",false},{"reason","launch-unproved"}};
 reason_.clear();directory_.reset();native_=std::make_unique<Native>();
#ifdef Q_OS_WIN
 native_->job=CreateJobObjectW(nullptr,nullptr);
 JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
 limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE|JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
 limits.BasicLimitInformation.ActiveProcessLimit=512;
 SIZE_T bytes=0;InitializeProcThreadAttributeList(nullptr,1,0,&bytes);
 native_->attributes.resize(bytes);
 auto attributes=reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(native_->attributes.data());
 const bool initialized=InitializeProcThreadAttributeList(attributes,1,0,&bytes)!=FALSE;
 if(initialized)native_->startup.lpAttributeList=attributes;
 const bool ready=native_->job&&SetInformationJobObject(native_->job,JobObjectExtendedLimitInformation,&limits,sizeof limits)&&initialized&&
  UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_JOB_LIST,&native_->job,sizeof native_->job,nullptr,nullptr);
 setCreateProcessArgumentsModifier([this,ready](QProcess::CreateProcessArguments *args){
  if(!ready){args->applicationName=L"";return;} // Refuse, never start uncontained.
  const auto attributes=native_->startup.lpAttributeList;
  native_->startup.StartupInfo=*args->startupInfo;
  native_->startup.StartupInfo.cb=sizeof(STARTUPINFOEXW);
  native_->startup.lpAttributeList=attributes;
  args->startupInfo=&native_->startup.StartupInfo;
  args->flags|=EXTENDED_STARTUPINFO_PRESENT;
 });
 QProcess::setProgram(program_);QProcess::setArguments(arguments_);
#elif defined(Q_OS_LINUX)
 directory_=std::make_unique<QTemporaryDir>(QDir::tempPath()+"/nexvary-process-XXXXXX");
 const bool privateDirectory=directory_->isValid()&&QFile::setPermissions(directory_->path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
 const auto supervisor=QDir(QCoreApplication::applicationDirPath()).filePath("mr_process_supervisor");
 const QFileInfo executable(supervisor);
 QProcess::setProgram(privateDirectory&&executable.isFile()&&!executable.isSymLink()&&executable.isExecutable()?supervisor:QString());
 QStringList args{directory_->filePath("receipt.json"),"--",program_};args.append(arguments_);QProcess::setArguments(args);
#else
 QProcess::setProgram(QString()); // No unsupported-platform fallback.
#endif
 QProcess::start(mode|WriteOnly);
}
void OwnedProcess::stop(const QString &reason){
 if(state()==NotRunning)return;
 if(reason_.isEmpty())reason_=reason.left(64);
#ifdef Q_OS_WIN
 if(native_&&native_->job)TerminateJobObject(native_->job,125);
#elif defined(Q_OS_LINUX)
 QProcess::closeWriteChannel();
#else
 QProcess::kill();
#endif
}
void OwnedProcess::collectReceipt(){
#ifdef Q_OS_WIN
 DWORD remaining=1,before=0;bool observed=false,terminated=false;
 if(native_&&native_->job){
  JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting{};
  observed=QueryInformationJobObject(native_->job,JobObjectBasicAccountingInformation,&accounting,sizeof accounting,nullptr)!=FALSE;
  before=accounting.ActiveProcesses;terminated=TerminateJobObject(native_->job,125)!=FALSE;
  QElapsedTimer deadline;deadline.start();
  do {if(!QueryInformationJobObject(native_->job,JobObjectBasicAccountingInformation,&accounting,sizeof accounting,nullptr)){observed=false;break;}remaining=accounting.ActiveProcesses;if(!remaining)break;QThread::msleep(10);}while(deadline.elapsed()<2000);
 }
 receipt_={{"schema","nexvary-process-cleanup/v1"},{"mechanism","windows-creation-job"},{"cleanupComplete",observed&&terminated&&remaining==0},{"observedAtShutdown",static_cast<quint32>(before)},{"remaining",static_cast<quint32>(remaining)}};
#elif defined(Q_OS_LINUX)
 if(directory_){QFile file(directory_->filePath("receipt.json"));if(!file.symLinkTarget().isEmpty()||file.size()>4096||!file.open(QIODevice::ReadOnly))return;
  const auto object=QJsonDocument::fromJson(file.read(4097)).object();
  if(object.value("schema")=="nexvary-process-cleanup/v1"&&object.value("mechanism")=="linux-subreaper-groups"&&object.value("cleanupComplete").isBool())receipt_=object.toVariantMap();
 }
#endif
 receipt_.insert("reason",reason_.isEmpty()?"worker-exit":reason_);
}
