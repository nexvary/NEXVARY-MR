#include "SyntheticPolicyExercise.hpp"
#include "ToolBridge.hpp"
#include "MirrorSession.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdio>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#ifdef Q_OS_UNIX
#include <csignal>
#endif
namespace {
int child(const QString &tool,const QStringList &args){
 QFile modeFile(qEnvironmentVariable("MR_MIRROR_MODE"));modeFile.open(QIODevice::ReadOnly);const auto mode=modeFile.readAll().trimmed();modeFile.close();
 QFile log(qEnvironmentVariable("MR_MIRROR_LOG"));if(!log.open(QIODevice::Append))return 90;
 log.write(QJsonDocument(QJsonObject{{"pid",QCoreApplication::applicationPid()},{"tool",tool},{"args",QJsonArray::fromStringList(args)},{"adb",qEnvironmentVariable("ADB")},{"target",qEnvironmentVariable("ANDROID_SERIAL")},{"serverOverride",qEnvironmentVariable("SCRCPY_SERVER_PATH")},{"socket",qEnvironmentVariable("ADB_SERVER_SOCKET")}}).toJson(QJsonDocument::Compact)+"\n");log.close();
 auto out=[](const char *s){std::fputs(s,stdout);std::fflush(stdout);};
 if(tool=="scrcpy"){
  if(args==QStringList{"--version"}){if(mode=="version-timeout")QThread::msleep(30000);out(mode=="bad-version"?"scrcpy 999.0\n":"scrcpy 5.0.1 <https://github.com/Genymobile/scrcpy>\n");return 0;}
  if(mode=="viewer-close")return 0;
  if(mode=="viewer-error"){std::fputs("Synthetic screen encoder failure\n",stderr);return 2;}
  if(mode=="viewer-flood"){const QByteArray bytes(100000,'x');std::fwrite(bytes.constData(),1,static_cast<size_t>(bytes.size()),stdout);std::fflush(stdout);}
#ifdef Q_OS_UNIX
  if(mode=="ignore-terminate")std::signal(SIGTERM,SIG_IGN);
#endif
  QThread::msleep(30000);return 0;
 }
 if(args==QStringList{"devices","-l"}){
  out(mode=="duplicate"?"MIRROR42 device\nMIRROR42 device\n":mode=="offline"?"MIRROR42 offline\n":mode=="disconnected"?"List of devices attached\n":"List of devices attached\nMIRROR42 device\nOTHER42 device\n");return 0;
 }
 if(args.size()==5&&args.mid(0,4)==QStringList{"-s","MIRROR42","shell","getprop"}){
  if(mode=="property-flood"){const QByteArray bytes(100000,'x');std::fwrite(bytes.constData(),1,static_cast<size_t>(bytes.size()),stdout);std::fflush(stdout);return 0;}
  if(mode=="property-timeout")QThread::msleep(30000);
  if(mode=="property-error")return 3;
  if(mode=="property-missing")return 0;
  if(mode=="property-multiline"){out("Mirror Fixture\nUnexpected extra row\n");return 0;}
  const auto key=args.last();
  if(key=="ro.product.manufacturer")out(mode=="manufacturer-changed"?"Other\n":"Synthetic\n");
  else if(key=="ro.product.model")out(mode=="identity-changed"?"Other\n":"Mirror Fixture\n");
  else if(key=="ro.build.fingerprint")out(mode=="fingerprint-changed"?"synthetic/other\n":"synthetic/mirror\n");
  else return 98;
  return 0;
 }
 if(args==QStringList{"-s","MIRROR42","shell","getprop"}){
  if(mode=="large-property-catalogue")for(int i=0;i<1800;++i)std::fprintf(stdout,"[vendor.synthetic.padding.%d]: [Synthetic unrelated firmware property padding]\n",i);
  out(mode=="identity-changed"?"[ro.product.manufacturer]: [Synthetic]\n[ro.product.model]: [Other]\n[ro.build.fingerprint]: [synthetic/other]\n":"[ro.product.manufacturer]: [Synthetic]\n[ro.product.model]: [Mirror Fixture]\n[ro.build.fingerprint]: [synthetic/mirror]\n");return 0;
 }
 return 99;
}
}
class MirrorTests:public QObject {
 Q_OBJECT
 QTemporaryDir dir_;
 QString adb_,scrcpy_,mode_,log_;
 QByteArray oldPath_;
 void mode(const QByteArray &value){QFile f(mode_);QVERIFY(f.open(QIODevice::WriteOnly));f.write(value);}
 QByteArray journal()const{QFile f(log_);if(!f.open(QIODevice::ReadOnly))return {};return f.readAll();}
 bool ready(ToolBridge &b){
  if(!b.configureTool("adb",adb_)||!b.configureTool("scrcpy",scrcpy_))return false;
  QSignalSpy spy(&b,&ToolBridge::operationFinished);b.scanAdb();if(!spy.wait(5000))return false;
  b.selectDevice("MIRROR42");spy.clear();b.readDeviceInfo();return spy.wait(5000)&&spy.last()[0].toBool();
 }
 QVariantMap approval()const{return {{"authorized",true},{"component",true},{"challenge","MIRROR42"}};}
 bool viewerStarted()const{for(const auto &line:journal().split('\n')){const auto row=QJsonDocument::fromJson(line).object();if(row.value("tool")=="scrcpy"&&row.value("args").toArray().size()>1)return true;}return false;}
private slots:
 void initTestCase(){
  QVERIFY(dir_.isValid());oldPath_=qgetenv("PATH");mode_=dir_.filePath("mode.txt");log_=dir_.filePath("process.jsonl");
  for(const auto &name:{QString("adb"),QString("scrcpy")}){
#ifdef Q_OS_WIN
   const auto path=dir_.filePath(name+".exe");
#else
   const auto path=dir_.filePath(name);
#endif
   QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),path));QVERIFY(QFile::setPermissions(path,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));if(name=="adb")adb_=path;else scrcpy_=path;
  }
 }
 void init(){qputenv("MR_MIRROR_MODE",QFile::encodeName(mode_));qputenv("MR_MIRROR_LOG",QFile::encodeName(log_));QFile::remove(log_);mode("");}
 void cleanup(){qunsetenv("MR_MIRROR_MODE");qunsetenv("MR_MIRROR_LOG");qputenv("PATH",oldPath_);}
 void governmentPermissionStopsOwnedViewerAndDoesNotResume(){
  QTemporaryDir state;IdentityService identity(state.filePath("identity"),true);const QString pass="Synthetic mirror policy 2026!";
  QVERIFY(identity.bootstrap("policy-admin","Synthetic administrator",pass));QVERIFY(identity.login("policy-admin",pass));GovernmentOperationPolicy policy(&identity);
  ToolBridge b;b.setOperationPolicy(&policy);QVERIFY(b.configureTool("adb",adb_));QVERIFY(b.configureTool("scrcpy",scrcpy_));
  b.scanAdb();QVERIFY(!b.busy());QVERIFY(journal().isEmpty());QVERIFY(!b.startMirror("MIRROR42",approval()));
  QVERIFY(performSyntheticReviewedException(identity,policy,"device-tools","Synthetic owned mirror fixture",pass));QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);QVERIFY(b.mirrorActive());
  policy.revoke("device-tools");QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),5000);QVERIFY(!b.busy());const auto before=journal();
  QVERIFY(performSyntheticReviewedException(identity,policy,"device-tools","Synthetic regrant does not reconnect",pass));QTest::qWait(100);QCOMPARE(journal(),before);QVERIFY(!b.mirrorActive());identity.logout();QVERIFY(!b.startMirror("MIRROR42",approval()));QCOMPARE(journal(),before);
 }
 void versionBoundary(){QVERIFY(MirrorSession::supportedVersion("scrcpy 5.0.1 <https://github.com/Genymobile/scrcpy>\n"));for(const auto &s:{"scrcpy 5.0.10","scrcpy 999.0","scrcpy 5.0.1; reboot","junk\nscrcpy 5.0.1"})QVERIFY(!MirrorSession::supportedVersion(s));}
 void consentRefusesBeforeProcess(){ToolBridge b;QVERIFY(ready(b));const auto before=journal();QVERIFY(!b.startMirror());QVERIFY(!b.startMirror("OTHER",approval()));auto a=approval();a["authorized"]="true";QVERIFY(!b.startMirror("MIRROR42",a));a=approval();a["component"]=false;QVERIFY(!b.startMirror("MIRROR42",a));QCOMPARE(journal(),before);QVERIFY(!b.mirrorActive());}
 void ownedProcessAndPinnedEnvironment(){
  ToolBridge b;QVERIFY(ready(b));
  qputenv("ADB_SERVER_SOCKET","tcp:wrong.invalid:9999");qputenv("SCRCPY_SERVER_PATH","unreviewed-server");
  const bool started=b.startMirror("MIRROR42",approval());qunsetenv("ADB_SERVER_SOCKET");qunsetenv("SCRCPY_SERVER_PATH");QVERIFY(started);
  QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);QVERIFY(b.busy());QVERIFY(b.mirrorActive());
  for(const auto &line:journal().split('\n')){const auto row=QJsonDocument::fromJson(line).object();if(row.value("tool")!="scrcpy"||row.value("args").toArray().size()<2)continue;
   const auto args=row.value("args").toArray();QCOMPARE(args,QJsonArray::fromStringList({"--serial","MIRROR42","--no-control","--no-audio","--no-clipboard-autosync","--no-power-on","--max-size=1600","--max-fps=30","--verbosity=warn","--window-title=NEXVARY MR Screen View"}));
   QCOMPARE(row.value("adb").toString(),QFileInfo(adb_).canonicalFilePath());QCOMPARE(row.value("target").toString(),QString("MIRROR42"));QVERIFY(row.value("socket").toString().isEmpty());QVERIFY(row.value("serverOverride").toString().isEmpty());
  }
  QVERIFY(!b.configureTool("adb",adb_));const auto before=journal();QVERIFY(!b.startMirror("MIRROR42",approval()));QCOMPARE(journal(),before);b.stopMirror();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!b.busy());QCOMPARE(b.mirrorState(),QString("stopped"));
 }
 void largeCatalogueUsesOnlyPinnedIdentityValues(){
  ToolBridge b;QVERIFY(ready(b));QFile::remove(log_);mode("large-property-catalogue");
  QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);
  // Allow multiple monitor rounds: the large catalogue must not be dumped
  // either before launch or during the periodic connection/identity checks.
  QTest::qWait(4500);QVERIFY(b.mirrorActive());
  QMap<QString,int> reads;
  for(const auto &line:journal().split('\n')){
   const auto row=QJsonDocument::fromJson(line).object();if(row.value("tool")!="adb")continue;
   const auto args=row.value("args").toArray();
   if(args==QJsonArray::fromStringList({"devices","-l"}))continue;
   QCOMPARE(args.size(),5);QCOMPARE(args.at(0).toString(),QString("-s"));QCOMPARE(args.at(1).toString(),QString("MIRROR42"));
   QCOMPARE(args.at(2).toString(),QString("shell"));QCOMPARE(args.at(3).toString(),QString("getprop"));
   const auto key=args.at(4).toString();const QStringList allowed{"ro.product.manufacturer","ro.product.model","ro.build.fingerprint"};QVERIFY(allowed.contains(key));++reads[key];
  }
  QCOMPARE(reads.size(),3);for(auto count:reads)QVERIFY(count>=2);
  b.stopMirror();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);
 }
 void cancelDuringIdentityStartsNoViewer(){
  ToolBridge b;QVERIFY(ready(b));mode("property-timeout");QFile::remove(log_);
  QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(journal().contains("ro.product.manufacturer"),3000);
  b.cancel();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!viewerStarted());
 }
 void cancelDuringVersionStartsNoViewer(){ToolBridge b;QVERIFY(ready(b));mode("version-timeout");QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(journal().contains("--version"),3000);b.cancel();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!viewerStarted());}
 void cleanWindowClose(){ToolBridge b;QVERIFY(ready(b));mode("viewer-close");QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),5000);QCOMPARE(b.mirrorState(),QString("stopped"));}
 void preflightFailure_data(){QTest::addColumn<QByteArray>("value");for(const auto &s:{"bad-version","version-timeout","duplicate","offline","disconnected","identity-changed","manufacturer-changed","fingerprint-changed","property-flood","property-timeout","property-error","property-missing","property-multiline"})QTest::newRow(s)<<QByteArray(s);}
 void preflightFailure(){ToolBridge b;QVERIFY(ready(b));mode(QTest::currentDataTag());QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),6000);QCOMPARE(b.mirrorState(),QString("failed"));QVERIFY(!viewerStarted());}
 void disconnectAndIdentityDuringView_data(){QTest::addColumn<QByteArray>("value");for(const auto &s:{"disconnected","identity-changed","duplicate","manufacturer-changed","fingerprint-changed","property-flood"})QTest::newRow(s)<<QByteArray(s);}
 void disconnectAndIdentityDuringView(){QFETCH(QByteArray,value);ToolBridge b;QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);mode(value);QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),7000);QCOMPARE(b.mirrorState(),QString("failed"));}
 void viewerErrorAndOutputLimit_data(){QTest::addColumn<QByteArray>("value");QTest::newRow("error")<<QByteArray("viewer-error");QTest::newRow("bounded-output")<<QByteArray("viewer-flood");}
 void viewerErrorAndOutputLimit(){QFETCH(QByteArray,value);ToolBridge b;QVERIFY(ready(b));mode(value);QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),5000);QCOMPARE(b.mirrorState(),QString("failed"));QVERIFY(b.mirrorOutput().size()<=64*1024);}
 void selectionChangeStopsSession(){ToolBridge b;QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);b.selectDevice("OTHER42");QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QCOMPARE(b.selectedSerial(),QString("OTHER42"));}
 void cancelAndEscalatedTermination(){ToolBridge b;QVERIFY(ready(b));mode("ignore-terminate");QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);b.cancel();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!b.busy());}
 void destructorStopsChild(){qint64 pid=0;{ToolBridge b;QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);for(const auto &line:journal().split('\n')){const auto row=QJsonDocument::fromJson(line).object();if(row.value("tool")=="scrcpy"&&row.value("args").toArray().size()>1)pid=row.value("pid").toVariant().toLongLong();}QVERIFY(pid>0);}
#ifdef Q_OS_WIN
  const auto handle=OpenProcess(SYNCHRONIZE,FALSE,static_cast<DWORD>(pid));if(handle){QCOMPARE(WaitForSingleObject(handle,0),static_cast<DWORD>(WAIT_OBJECT_0));CloseHandle(handle);}
#else
  QVERIFY(::kill(static_cast<pid_t>(pid),0)!=0);
#endif
 }

private:
 bool mrLaboratory()const{ToolBridge b;return b.laboratoryBuild();}
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);const auto name=QFileInfo(app.applicationFilePath()).completeBaseName();if(name=="adb"||name=="scrcpy")return child(name,app.arguments().mid(1));MirrorTests tests;return QTest::qExec(&tests,argc,argv);}
#include "mirror_tests.moc"
