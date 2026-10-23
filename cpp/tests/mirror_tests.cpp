#include "ToolBridge.hpp"
#include "MirrorSession.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
 if(args==QStringList{"-s","MIRROR42","shell","getprop"}){
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
 void versionBoundary(){QVERIFY(MirrorSession::supportedVersion("scrcpy 5.0.1 <https://github.com/Genymobile/scrcpy>\n"));for(const auto &s:{"scrcpy 5.0.10","scrcpy 999.0","scrcpy 5.0.1; reboot","junk\nscrcpy 5.0.1"})QVERIFY(!MirrorSession::supportedVersion(s));}
 void productionAndConsentRefuseBeforeProcess(){ToolBridge b;QVERIFY(ready(b));const auto before=journal();QVERIFY(!b.startMirror());QVERIFY(!b.startMirror("OTHER",approval()));auto a=approval();a["authorized"]="true";QVERIFY(!b.startMirror("MIRROR42",a));a=approval();a["component"]=false;QVERIFY(!b.startMirror("MIRROR42",a));if(!b.laboratoryBuild())QVERIFY(!b.startMirror("MIRROR42",approval()));QCOMPARE(journal(),before);QVERIFY(!b.mirrorActive());}
 void ownedProcessAndPinnedEnvironment(){
  ToolBridge b;if(!b.laboratoryBuild())QSKIP("Screen-view lifecycle runs in Laboratory; production denial is tested separately.");QVERIFY(ready(b));
  qputenv("ADB_SERVER_SOCKET","tcp:wrong.invalid:9999");qputenv("SCRCPY_SERVER_PATH","unreviewed-server");
  const bool started=b.startMirror("MIRROR42",approval());qunsetenv("ADB_SERVER_SOCKET");qunsetenv("SCRCPY_SERVER_PATH");QVERIFY(started);
  QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);QVERIFY(b.busy());QVERIFY(b.mirrorActive());
  for(const auto &line:journal().split('\n')){const auto row=QJsonDocument::fromJson(line).object();if(row.value("tool")!="scrcpy"||row.value("args").toArray().size()<2)continue;
   const auto args=row.value("args").toArray();QCOMPARE(args,QJsonArray::fromStringList({"--serial","MIRROR42","--no-control","--no-audio","--no-clipboard-autosync","--no-power-on","--max-size=1600","--max-fps=30","--verbosity=warn","--window-title=NEXVARY MR Laboratory"}));
   QCOMPARE(row.value("adb").toString(),QFileInfo(adb_).canonicalFilePath());QCOMPARE(row.value("target").toString(),QString("MIRROR42"));QVERIFY(row.value("socket").toString().isEmpty());QVERIFY(row.value("serverOverride").toString().isEmpty());
  }
  QVERIFY(!b.configureTool("adb",adb_));const auto before=journal();QVERIFY(!b.startMirror("MIRROR42",approval()));QCOMPARE(journal(),before);b.stopMirror();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!b.busy());QCOMPARE(b.mirrorState(),QString("stopped"));
 }
 void cancelDuringVersionStartsNoViewer(){ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));mode("version-timeout");QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(journal().contains("--version"),3000);b.cancel();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!viewerStarted());}
 void cleanWindowClose(){ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));mode("viewer-close");QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),5000);QCOMPARE(b.mirrorState(),QString("stopped"));}
 void preflightFailure_data(){QTest::addColumn<QByteArray>("value");for(const auto &s:{"bad-version","version-timeout","duplicate","offline","disconnected","identity-changed"})QTest::newRow(s)<<QByteArray(s);}
 void preflightFailure(){ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));mode(QTest::currentDataTag());QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),6000);QCOMPARE(b.mirrorState(),QString("failed"));QVERIFY(!viewerStarted());}
 void disconnectAndIdentityDuringView_data(){QTest::addColumn<QByteArray>("value");for(const auto &s:{"disconnected","identity-changed","duplicate"})QTest::newRow(s)<<QByteArray(s);}
 void disconnectAndIdentityDuringView(){QFETCH(QByteArray,value);ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);mode(value);QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),7000);QCOMPARE(b.mirrorState(),QString("failed"));}
 void viewerErrorAndOutputLimit_data(){QTest::addColumn<QByteArray>("value");QTest::newRow("error")<<QByteArray("viewer-error");QTest::newRow("bounded-output")<<QByteArray("viewer-flood");}
 void viewerErrorAndOutputLimit(){QFETCH(QByteArray,value);ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));mode(value);QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),5000);QCOMPARE(b.mirrorState(),QString("failed"));QVERIFY(b.mirrorOutput().size()<=64*1024);}
 void selectionChangeStopsSession(){ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);b.selectDevice("OTHER42");QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QCOMPARE(b.selectedSerial(),QString("OTHER42"));}
 void cancelAndEscalatedTermination(){ToolBridge b;if(!b.laboratoryBuild())QSKIP("Laboratory-only lifecycle.");QVERIFY(ready(b));mode("ignore-terminate");QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);b.cancel();QTRY_VERIFY_WITH_TIMEOUT(!b.mirrorActive(),4000);QVERIFY(!b.busy());}
 void destructorStopsChild(){if(!mrLaboratory())QSKIP("Laboratory-only lifecycle.");qint64 pid=0;{ToolBridge b;QVERIFY(ready(b));QVERIFY(b.startMirror("MIRROR42",approval()));QTRY_VERIFY_WITH_TIMEOUT(viewerStarted(),5000);for(const auto &line:journal().split('\n')){const auto row=QJsonDocument::fromJson(line).object();if(row.value("tool")=="scrcpy"&&row.value("args").toArray().size()>1)pid=row.value("pid").toVariant().toLongLong();}QVERIFY(pid>0);}
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
