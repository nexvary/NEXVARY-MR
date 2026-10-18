#include "ToolBridge.hpp"
#include "DeviceProtocol.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdio>

// Controlled child processes only. No adb/fastboot installation or phone is used.
static int child(QStringList args, bool fast) {
 QFile journal(qEnvironmentVariable("MR_SERVICE_JOURNAL"));
 if (!journal.open(QIODevice::Append)) return 98;
 journal.write(QJsonDocument(QJsonArray::fromStringList(args)).toJson(QJsonDocument::Compact)+"\n");journal.close();
 const auto mode=qEnvironmentVariable("MR_SERVICE_MODE");
 auto out=[](const QString &value,bool error=false){const auto bytes=value.toUtf8();std::fwrite(bytes.constData(),1,static_cast<size_t>(bytes.size()),error?stderr:stdout);};
 if(args==QStringList{"devices","-l"}||args==QStringList{"devices"}) {
  if(mode=="disconnected") {out("List of devices attached\n");return 0;}
  out(fast?"TEST42\tfastboot\n":mode=="sideload"?"List of devices attached\nTEST42\tsideload\n":"List of devices attached\nTEST42\tdevice\nLOCKED\tunauthorized\n");return 0;
 }
 if(args.size()<3||args[0]!="-s"||args[1]!="TEST42")return 99;
 args=args.mid(2);
 if(fast) {
  if(args.size()==2&&args[0]=="getvar") {
   const auto key=args[1];QString value;
   if(key=="product")value="training-model";
   else if(key=="unlocked")value=mode=="locked"?"no":"yes";
   else if(key=="version-bootloader")value="fixture-1";
   else if(key=="is-userspace")value="no";
   else if(key.startsWith("partition-size:"))value=mode=="no-partition"?"0x0":"0x100000";
   else if(key=="partition-type:userdata")value="ext4";
   else return 99;
   out("(bootloader) "+key+": "+value+"\nFinished. Total time: 0.001s\n",true);return 0;
  }
  if(args==QStringList{"erase","userdata"}||args==QStringList{"erase","cache"}||args==QStringList{"flashing","get_unlock_ability"}||args==QStringList{"reboot"}||args==QStringList{"reboot-bootloader"}){out("OKAY\n",true);return 0;}
  return 99;
 }
 if(args==QStringList{"shell","getprop"}) {
  const auto manufacturer=qEnvironmentVariable("MR_SERVICE_BRAND","samsung");
  out("[ro.product.manufacturer]: ["+manufacturer+"]\n[ro.product.model]: ["+(mode=="changed"?QString("other-model"):QString("training-model"))+"]\n[ro.build.version.sdk]: ["+(mode=="old"?QString("23"):QString("34"))+"]\n[ro.build.version.release]: [14]\n[ro.build.fingerprint]: [synthetic/test]\n[gsm.sim.operator.alpha]: [PRIVATE]\n");return 0;
 }
 if(args==QStringList{"shell","cmd","uimode","help"}){out("night [yes|no|auto]\n");return 0;}
 if(args==QStringList{"shell","cmd","uimode","night"}){out(mode=="bad-readback"?"Night mode: no\n":"Night mode: yes\n");return 0;}
 if(args.size()==5&&args.mid(0,4)==QStringList{"shell","cmd","uimode","night"}) {
  if(mode=="denied"){out("SecurityException: permission denied\n",true);return 0;}
  if(mode=="hang")QThread::msleep(30000);
  out("Night mode set\n");return 0;
 }
 if(args.size()==7&&args.mid(0,6)==QStringList{"shell","cmd","package","resolve-activity","--brief","-a"}){out(mode=="no-activity"?"No activity found\n":"com.android.settings/.Settings\n");return 0;}
 if(args==QStringList{"shell","dumpsys","battery"}){out("level: 85\n");return 0;}
 if(args==QStringList{"shell","df","-k","/data"}){out("/dev/test 100 20 80 20% /data\n");return 0;}
 if(args==QStringList{"shell","cat","/proc/partitions"}){out("Permission denied\n",true);return 1;}
 if(args.size()==6&&args.mid(0,5)==QStringList{"shell","am","start","-W","-a"}){out("Status: ok\n");return 0;}
 if(args[0]=="reboot"&&args.size()<=2){out("\n");return 0;}
 if((args[0]=="install"&&(args.size()==2||(args.size()==3&&args[1]=="-r")))||(args[0]=="sideload"&&args.size()==2)) {
  QFile package(args.last());if(!package.open(QIODevice::ReadOnly)||!package.readAll().startsWith("PK\003\004"))return 97;
  out(mode=="install-failure"?"Failure [INSTALL_FAILED_TEST]\n":mode=="no-success"?"Performing streamed install\n":args[0]=="install"?"Success\n":"Total xfer: 1.00x\n");return 0;
 }
 return 99;
}

class ServiceTests: public QObject {
 Q_OBJECT
 QTemporaryDir dir_;
 QByteArray oldPath_;
 QString journal_;
 bool waitFor(ToolBridge &bridge,const std::function<void()> &start,bool success=true) {
  QSignalSpy spy(&bridge,&ToolBridge::operationFinished);start();
  if(spy.isEmpty()&&!spy.wait(15000))return false;
  return !bridge.busy()&&!spy.isEmpty()&&spy.last()[0].toBool()==success;
 }
 QVariantMap approval(bool destructive=false)const{return {{"authorized",true},{"challenge","TEST42"},{"destructive",destructive}};}
 bool probe(ToolBridge &bridge,bool fast=false) {
  if(!waitFor(bridge,[&]{if(fast)bridge.scanFastboot();else bridge.scanAdb();}))return false;
  bridge.selectDevice("TEST42");
  return waitFor(bridge,[&]{bridge.startService("probe","TEST42",approval());});
 }
 bool available(const ToolBridge &bridge,const QString &id)const{for(const auto &v:bridge.serviceActions())if(v.toMap().value("id").toString()==id)return v.toMap().value("available").toBool();return false;}
 QByteArray journal()const{QFile f(journal_);if(!f.open(QIODevice::ReadOnly))return {};return f.readAll();}
 QString package(const QString &name="fixture.apk")const{const auto path=dir_.filePath(name);QFile f(path);if(!f.open(QIODevice::WriteOnly))return {};f.write(QByteArray("PK\003\004SYNTHETIC PACKAGE FIXTURE",29));f.close();return path;}
 QString hash(const QString &path)const{QFile f(path);if(!f.open(QIODevice::ReadOnly))return {};return QString::fromLatin1(QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex());}
private slots:
 void adapterCommandAllowlist(){
  for(const auto &id:QStringList{"adb","fastboot"}){const auto *a=mr::service::adapter(id);QVERIFY(a);QCOMPARE(a->id(),id);QCOMPARE(a->addressed("TEST42",{"version"}),QStringList({"-s","TEST42","version"}));for(const auto &blocked:QStringList{"frp","lockscreen","shell","bootloader_unlock","edl"})QVERIFY(a->command(blocked,"file",true).isEmpty());}
  QVERIFY(!mr::service::adapter("edl"));
 }
 void initTestCase(){
  QVERIFY(dir_.isValid());oldPath_=qgetenv("PATH");journal_=dir_.filePath("commands.jsonl");
  for(const auto &base:{QString("adb"),QString("fastboot")}){
#ifdef Q_OS_WIN
   const auto name=base+".exe";
#else
   const auto name=base;
#endif
   const auto path=dir_.filePath(name);QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),path));QVERIFY(QFile::setPermissions(path,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));
  }
 }
 void init(){qputenv("PATH",QFile::encodeName(dir_.path())+QDir::listSeparator().toLatin1()+oldPath_);qputenv("MR_SERVICE_JOURNAL",QFile::encodeName(journal_));qunsetenv("MR_SERVICE_MODE");qunsetenv("MR_SERVICE_BRAND");QFile::remove(journal_);}
 void cleanup(){qputenv("PATH",oldPath_);qunsetenv("MR_SERVICE_JOURNAL");qunsetenv("MR_SERVICE_MODE");qunsetenv("MR_SERVICE_BRAND");}
 void capabilityIdentity_data(){QTest::addColumn<QString>("reported");QTest::addColumn<QString>("canonical");for(const auto &p:QList<QPair<QString,QString>>{{"samsung","Samsung"},{"LGE","LG"},{"HUAWEI","Huawei"},{"OPPO","Oppo"},{"vivo","Vivo"},{"ZTE","ZTE"}})QTest::newRow(qPrintable(p.first))<<p.first<<p.second;}
 void capabilityIdentity(){QFETCH(QString,reported);QFETCH(QString,canonical);qputenv("MR_SERVICE_BRAND",reported.toUtf8());ToolBridge b;QVERIFY(probe(b));QCOMPARE(b.serviceProfile().value("vendor").toString(),canonical);QVERIFY(available(b,"dark_on"));QVERIFY(available(b,"languages"));QVERIFY(!b.serviceProfile().contains("gsm.sim.operator.alpha"));QVERIFY(b.serviceProfile().value("observation:partitions").toString().startsWith("Unavailable"));QCOMPARE(b.requestedModels().size(),42);for(const auto &r:b.requestedModels())QVERIFY(!r.toMap().value("validated").toBool());}
 void authorizationAndBlockedActions(){ToolBridge b;QVERIFY(probe(b));const auto before=journal();for(const auto &id:{"frp","lockscreen","edl","efs","bootloader_unlock","sim_unlock"})QVERIFY(!b.startService(id,"TEST42",approval(true)));QVERIFY(!b.startService("dark_on","other",approval()));QVERIFY(!b.startService("dark_on","TEST42",{}));auto bad=approval();bad.insert("shell","rm -rf");QVERIFY(!b.startService("dark_on","TEST42",bad));QCOMPARE(journal(),before);}
 void darkModeVerification(){ToolBridge b;QVERIFY(probe(b));QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());}));QVERIFY(journal().contains("\"night\",\"yes\""));QVERIFY(b.serviceProfile().isEmpty());}
 void deniedAndUnconfirmedReadback_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("permission")<<QByteArray("denied");QTest::newRow("readback")<<QByteArray("bad-readback");}
 void deniedAndUnconfirmedReadback(){QFETCH(QByteArray,mode);ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE",mode);QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());},false));QVERIFY(b.serviceProfile().isEmpty());}
 void missingSettingsAndOldAndroid_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("missing-activity")<<QByteArray("no-activity");QTest::newRow("old-sdk")<<QByteArray("old");}
 void missingSettingsAndOldAndroid(){QFETCH(QByteArray,mode);qputenv("MR_SERVICE_MODE",mode);ToolBridge b;QVERIFY(probe(b));QVERIFY(!available(b,"languages"));QVERIFY(!available(b,"keyboard"));if(mode=="old")QVERIFY(!available(b,"dark_on"));}
 void disconnectAndIdentityChange_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("usb")<<QByteArray("disconnected");QTest::newRow("identity")<<QByteArray("changed");}
 void disconnectAndIdentityChange(){QFETCH(QByteArray,mode);ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE",mode);QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());},false));QVERIFY(!journal().contains("\"night\",\"yes\""));}
 void apkPrivateCopyIntegrity(){ToolBridge b;QVERIFY(probe(b));const auto path=package(),expected=hash(path);auto a=approval();a.insert("file",path);a.insert("sha256",expected);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);}));QCOMPARE(hash(path),expected);QVERIFY(journal().contains("\"install\""));QVERIFY(!journal().contains("\"-r\""));QVERIFY(!journal().contains(path.toUtf8()));QCOMPARE(b.serviceHistory()[0].toMap().value("sha256").toString(),expected);}
 void invalidPackageNeverReachesDevice(){
  for(const auto &path:QStringList{"https://example.invalid/package.apk","//server/share/file.apk",dir_.filePath("missing.apk")}){ToolBridge b;QVERIFY(probe(b));const auto before=journal();auto a=approval();a.insert("file",path);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);},false));QCOMPARE(journal(),before);}
 }
 void explicitApkReplace(){ToolBridge b;QVERIFY(probe(b));auto a=approval();a.insert("file",package());a.insert("replace",true);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);}));QVERIFY(journal().contains("\"install\",\"-r\""));}
 void ambiguousTransportAndRescanInvalidate(){ToolBridge b;QVERIFY(probe(b));QVERIFY(waitFor(b,[&]{b.scanAdb();}));QVERIFY(b.serviceProfile().isEmpty());QVERIFY(!available(b,"dark_on"));QVERIFY(waitFor(b,[&]{b.scanFastboot();}));QVERIFY(!available(b,"probe"));}
 void apkHashAndManagerFailure_data(){QTest::addColumn<QByteArray>("mode");QTest::addColumn<bool>("wrongHash");QTest::newRow("hash")<<QByteArray()<<true;QTest::newRow("manager")<<QByteArray("install-failure")<<false;QTest::newRow("no-confirmation")<<QByteArray("no-success")<<false;}
 void apkHashAndManagerFailure(){QFETCH(QByteArray,mode);QFETCH(bool,wrongHash);ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE",mode);auto a=approval();a.insert("file",package());if(wrongHash)a.insert("sha256",QString(64,'0'));QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);},false));if(wrongHash)QVERIFY(!journal().contains("\"install\""));}
 void sideloadConsentAndMode(){qputenv("MR_SERVICE_MODE","sideload");ToolBridge b;QVERIFY(waitFor(b,[&]{b.scanAdb();}));b.selectDevice("TEST42");QVERIFY(!available(b,"probe"));QVERIFY(available(b,"sideload_zip"));const auto path=package("fixture.zip");auto a=approval(true);a.insert("file",path);a.insert("sha256",hash(path));QVERIFY(!b.startService("sideload_zip","TEST42",a));a.insert("compatible",true);QVERIFY(waitFor(b,[&]{b.startService("sideload_zip","TEST42",a);}));QVERIFY(b.serviceOutput().contains("not proof"));QVERIFY(!journal().contains("\"getprop\""));}
 void fastbootFreshEraseGuard(){ToolBridge b;QVERIFY(probe(b,true));QVERIFY(available(b,"erase_userdata"));QVERIFY(!b.startService("erase_userdata","TEST42",approval()));qputenv("MR_SERVICE_MODE","locked");QVERIFY(waitFor(b,[&]{b.startService("erase_userdata","TEST42",approval(true));},false));QVERIFY(!journal().contains("\"erase\""));}
 void syntheticErase(){ToolBridge b;QVERIFY(probe(b,true));QVERIFY(waitFor(b,[&]{b.startService("erase_cache","TEST42",approval(true));}));QVERIFY(journal().contains("\"erase\",\"cache\""));}
 void noPartitionNoErase(){qputenv("MR_SERVICE_MODE","no-partition");ToolBridge b;QVERIFY(probe(b,true));QVERIFY(!available(b,"erase_userdata"));QVERIFY(!available(b,"erase_cache"));}
 void rebootInvalidatesDiscovery(){ToolBridge b;QVERIFY(probe(b));QVERIFY(waitFor(b,[&]{b.startService("reboot_recovery","TEST42",approval());}));QVERIFY(b.selectedSerial().isEmpty());QVERIFY(b.serviceProfile().isEmpty());QVERIFY(journal().contains("\"reboot\",\"recovery\""));}
 void cancellation(){ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE","hang");QSignalSpy spy(&b,&ToolBridge::operationFinished);QVERIFY(b.startService("dark_on","TEST42",approval()));QTRY_VERIFY_WITH_TIMEOUT(journal().contains("\"night\",\"yes\""),5000);b.cancel();if(spy.isEmpty())QVERIFY(spy.wait(5000));QVERIFY(!b.busy());QCOMPARE(spy.last()[0].toBool(),false);QVERIFY(!journal().contains("\"uimode\",\"night\"]"));}
 void reportConsentAndNoOverwrite(){ToolBridge b;QVERIFY(probe(b));const auto path=dir_.filePath("service-report.json");QFile::remove(path);QVERIFY(!b.exportServiceReport(path,false));QVERIFY(b.exportServiceReport(path,true));QVERIFY(!b.exportServiceReport(path,true));QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));const auto bytes=f.readAll();QVERIFY(bytes.contains("Pending Hardware Validation"));QVERIFY(!bytes.contains("PRIVATE"));}
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);const auto base=QFileInfo(app.applicationFilePath()).completeBaseName();if(base=="adb"||base=="fastboot")return child(app.arguments().mid(1),base=="fastboot");ServiceTests tests;return QTest::qExec(&tests,argc,argv);}
#include "service_tests.moc"
