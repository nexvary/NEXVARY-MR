#include "MirrorSession.hpp"
#include "ToolBridge.hpp"
#include "ServiceOperationPolicy.hpp"
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>

namespace {
constexpr qsizetype outputLimit=64*1024;
QString executable(const QString &path){
 const QFileInfo file(path);
 if(!file.isAbsolute()||!file.isFile()||!file.isReadable()||path.startsWith("//")||path.startsWith("\\\\"))return {};
#ifdef Q_OS_WIN
 if(file.suffix().compare("exe",Qt::CaseInsensitive)!=0)return {};
#else
 if(!file.isExecutable())return {};
#endif
 return file.canonicalFilePath();
}
}
MirrorSession::MirrorSession(QObject *parent):QObject(parent){
 deadline_.setSingleShot(true);deadline_.setInterval(3000);
 monitor_.setInterval(2000);kill_.setSingleShot(true);kill_.setInterval(1000);
 connect(&deadline_,&QTimer::timeout,this,[this]{end("Screen-view preflight/connection check timed out; host process stopped.",true);});
 connect(&monitor_,&QTimer::timeout,this,[this]{if(active_&&state_=="running")query({"devices","-l"},"monitor-discovery");});
 connect(&kill_,&QTimer::timeout,this,[this]{if(check_.state()!=QProcess::NotRunning)check_.kill();if(viewer_.state()!=QProcess::NotRunning)viewer_.kill();finalize();});
 connect(&check_,&QProcess::readyReadStandardOutput,this,&MirrorSession::collectCheck);
 connect(&check_,&QProcess::readyReadStandardError,this,&MirrorSession::collectCheck);
 connect(&check_,&QProcess::finished,this,&MirrorSession::checkFinished);
 connect(&check_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){if(active_&&state_!="stopping"&&error==QProcess::FailedToStart)end("Could not start the selected screen-view preflight tool.",true);});
 connect(&viewer_,&QProcess::readyReadStandardOutput,this,&MirrorSession::collectView);
 connect(&viewer_,&QProcess::readyReadStandardError,this,&MirrorSession::collectView);
 connect(&viewer_,&QProcess::started,this,[this]{
  if(state_!="starting")return;
  deadline_.stop();state_="running";message("scrcpy host process running. Verify the screen in its window; frame delivery and device acceptance are not established here.");monitor_.start();
 });
 connect(&viewer_,&QProcess::finished,this,[this](int code,QProcess::ExitStatus exit){
  collectView();if(!active_)return;
  if(state_=="stopping"){finalize();return;}
  end(exit==QProcess::NormalExit&&code==0?"scrcpy window closed; no automatic restart.":"scrcpy exited with an error. Review local output and USB authorization.",exit!=QProcess::NormalExit||code!=0);
 });
 connect(&viewer_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){if(active_&&state_!="stopping"&&error==QProcess::FailedToStart)end("Could not start the selected scrcpy executable.",true);});
}
MirrorSession::~MirrorSession(){shutdown();}
void MirrorSession::message(const QString &text){status_=text;emit changed();}
bool MirrorSession::supportedVersion(const QString &output){
 // Deliberate exact reviewed CLI boundary. A version string is not a signature
 // or an authenticity check on this user-selected external executable.
 const auto first=output.section('\n',0,0).trimmed();
 return QRegularExpression("^scrcpy 5\\.0\\.1(?: <https://github\\.com/Genymobile/scrcpy>)?$").match(first).hasMatch();
}
bool MirrorSession::start(const QString &scrcpy,const QString &adb,const QString &serial,const QVariantMap &identity,bool owner,bool component){
 if(active_)return false;
 if(!mr::service::screenViewAllowed()||!owner||!component||!ToolBridge::isValidSerial(serial)){
  message("Screen view requires explicit owner authorization and device-component consent.");return false;
 }
 for(const auto &field:{"ro.product.manufacturer","ro.product.model","ro.build.fingerprint"})if(identity.value(field).toString().isEmpty()){
  message("Read complete manufacturer, model and build fingerprint before screen view.");return false;
 }
 scrcpy_=executable(scrcpy);adb_=executable(adb);
 if(scrcpy_.isEmpty()||adb_.isEmpty()){message("Select valid local official scrcpy and ADB executables.");return false;}
 serial_=serial;identity_=identity;viewOutput_.clear();version_.clear();failed_=false;active_=true;
 auto environment=QProcessEnvironment::systemEnvironment();
 for(const auto &key:{"ADB","ANDROID_SERIAL","ADB_SERVER_SOCKET","ANDROID_ADB_SERVER_PORT","ADB_SERVER_PORT","SCRCPY_SERVER_PATH"})environment.remove(key);
 environment.insert("ADB",adb_);environment.insert("ANDROID_SERIAL",serial_);
 check_.setProcessEnvironment(environment);viewer_.setProcessEnvironment(environment);
 check_.setWorkingDirectory(QFileInfo(scrcpy_).absolutePath());viewer_.setWorkingDirectory(QFileInfo(scrcpy_).absolutePath());
 query({"--version"},"version");return true;
}
void MirrorSession::query(const QStringList &arguments,const QString &state){
 if(!active_||check_.state()!=QProcess::NotRunning)return;
 state_=state;checkOutput_.clear();check_.setProgram(state=="version"?scrcpy_:adb_);check_.setArguments(arguments);deadline_.start();emit changed();check_.start();
}
void MirrorSession::collectCheck(){
 const auto bytes=check_.readAllStandardOutput()+check_.readAllStandardError();
 if(!active_||state_=="stopping")return;
 if(bytes.size()>outputLimit-checkOutput_.size()){end("Screen-view check exceeded its output limit.",true);return;}
 checkOutput_+=bytes;
}
void MirrorSession::checkFinished(int code,QProcess::ExitStatus exit){
 if(!active_)return;
 if(state_=="stopping"){finalize();return;}
 collectCheck();if(state_=="stopping"||!active_)return;
 deadline_.stop();
 if(exit!=QProcess::NormalExit||code!=0){end("Screen-view version/ADB check failed or device disconnected.",true);return;}
 const auto text=QString::fromUtf8(checkOutput_).trimmed();
 if(state_=="version"){
  if(!supportedVersion(text)){end("Unreviewed scrcpy CLI version; this integration accepts official 5.0.1 only.",true);return;}
  version_="5.0.1";query({"devices","-l"},"discovery");return;
 }
 const bool monitor=state_.startsWith("monitor-");
 if(state_=="discovery"||state_=="monitor-discovery"){
  int count=0;bool ready=false;
  for(const auto &line:text.split('\n')){const auto fields=line.trimmed().split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);if(fields.size()>=2&&fields[0]==serial_){++count;ready=fields[1]=="device";}}
  if(count!=1||!ready){end("Screen-view target missing, unauthorized or ambiguous; no alternate device selected.",true);return;}
  query({"-s",serial_,"shell","getprop"},monitor?"monitor-identity":"identity");return;
 }
 const auto properties=ToolBridge::parseDeviceProperties(text);
 for(const auto &field:{"ro.product.manufacturer","ro.product.model","ro.build.fingerprint"})if(properties.value(field)!=identity_.value(field)){
  end("Screen-view device identity/build changed; session stopped.",true);return;
 }
 if(monitor){state_="running";emit changed();return;}
 launchView();
}
void MirrorSession::launchView(){
 if(!active_||!mr::service::screenViewAllowed()){end("Screen view denied by compiled operation policy.",true);return;}
 state_="starting";viewer_.setProgram(scrcpy_);
 viewer_.setArguments({"--serial",serial_,"--no-control","--no-audio","--no-clipboard-autosync","--no-power-on","--max-size=1600","--max-fps=30","--verbosity=warn","--window-title=NEXVARY MR Screen View"});
 deadline_.start();emit changed();viewer_.start();
}
void MirrorSession::collectView(){
 const auto bytes=viewer_.readAllStandardOutput()+viewer_.readAllStandardError();
 if(!active_||state_=="stopping")return;
 if(bytes.size()>outputLimit-viewOutput_.size()){end("scrcpy output exceeded its bounded session limit.",true);return;}
 viewOutput_+=bytes;emit changed();
}
void MirrorSession::end(const QString &reason,bool failed){
 if(!active_)return;
 if(state_=="stopping")return;
 failed_=failed;state_="stopping";deadline_.stop();monitor_.stop();message(reason);
 if(check_.state()!=QProcess::NotRunning)check_.terminate();
 if(viewer_.state()!=QProcess::NotRunning)viewer_.terminate();
 kill_.start();finalize();
}
void MirrorSession::finalize(){
 if(!active_||state_!="stopping"||check_.state()!=QProcess::NotRunning||viewer_.state()!=QProcess::NotRunning)return;
 kill_.stop();active_=false;state_=failed_?"failed":"stopped";emit changed();emit finished(!failed_);
}
void MirrorSession::stop(const QString &reason){end(reason,false);}
void MirrorSession::shutdown(){
 deadline_.stop();monitor_.stop();kill_.stop();check_.disconnect(this);viewer_.disconnect(this);
 for(auto *process:{&check_,&viewer_})if(process->state()!=QProcess::NotRunning){process->terminate();if(!process->waitForFinished(500)){process->kill();process->waitForFinished(1000);}}
 active_=false;
}
