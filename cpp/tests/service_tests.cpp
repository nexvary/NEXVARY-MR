#include "ToolBridge.hpp"
#include "DeviceProtocol.hpp"
#include "DeviceHealth.hpp"
#include "ServiceCommandPolicy.hpp"
#include "PackagePreflight.hpp"
#include "ServiceReport.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
 auto out=[&mode](const QString &value,bool error=false){const auto bytes=(mode=="crlf"&&error?QString(value).replace("\n","\r\n"):value).toUtf8();std::fwrite(bytes.constData(),1,static_cast<size_t>(bytes.size()),error?stderr:stdout);};
 if(args==QStringList{"devices","-l"}||args==QStringList{"devices"}) {
  if(mode=="disconnected") {out("List of devices attached\n");return 0;}
  if(mode=="duplicate"){out(fast?"TEST42\tfastboot\nTEST42\tfastboot\n":"List of devices attached\nTEST42\tdevice\nTEST42\tdevice\n");return 0;}
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
   else if(key=="is-userspace"){if(mode=="duplicate-fastboot-mode"){out("(bootloader) is-userspace: no\n(bootloader) is-userspace: yes\n",true);return 0;}if(mode=="unknown-fastboot-mode")return 1;value=mode=="fastbootd"?"yes":"no";}
   else if(key.startsWith("partition-size:"))value=mode=="no-partition"?"0x0":"0x100000";
   else if(key=="partition-type:userdata")value="ext4";
   else return 99;
   out("(bootloader) "+key+": "+value+"\nFinished. Total time: 0.001s\n",true);return 0;
  }
  if(args==QStringList{"erase","userdata"}||args==QStringList{"erase","cache"}||args==QStringList{"flashing","get_unlock_ability"}||args==QStringList{"reboot"}||args==QStringList{"reboot-bootloader"}){out("OKAY\n",true);return 0;}
  return 99;
 }
 if(args==QStringList{"shell","getprop"}) {
  if(mode=="modern"||mode=="modern-no-page"){out("[ro.product.manufacturer]: [google]\n[ro.product.model]: [Synthetic API37 phone]\n[ro.build.version.sdk]: [37]\n[ro.build.version.codename]: [REL]\n[ro.build.version.preview_sdk]: [0]\n[ro.product.first_api_level]: [37]\n[ro.build.version.release]: [17]\n[ro.build.fingerprint]: [synthetic/api37]\n[ro.boot.dynamic_partitions]: [true]\n[ro.virtual_ab.enabled]: [true]\n[ro.crypto.state]: [encrypted]\n[ro.crypto.type]: [file]\n[ro.boot.verifiedbootstate]: [green]\n");return 0;}
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
 if(args==QStringList{"shell","getconf","PAGE_SIZE"}){if(mode=="modern-no-page")return 1;out("16384\n");return 0;}
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
 QString package(const QString &name="fixture.apk")const{const auto path=dir_.filePath(name);QFile f(path);if(!f.open(QIODevice::WriteOnly))return {};f.write(QByteArray::fromHex("504b030414000000000068b5565df0e731382b0000002b00000013000000416e64726f69644d616e69666573742e786d6c53594e544845544943205041434b4147452046495854555245202d204e4f5420494e5354414c4c41424c45504b0102140314000000000068b5565df0e731382b0000002b000000130000000000000000000000800100000000416e64726f69644d616e69666573742e786d6c504b05060000000001000100410000005c0000000000"));f.close();return path;}
 QString hash(const QString &path)const{QFile f(path);if(!f.open(QIODevice::ReadOnly))return {};return QString::fromLatin1(QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex());}
private slots:
 void modernProbeAndReport(){
  qputenv("MR_SERVICE_MODE","modern");ToolBridge b;QVERIFY(probe(b));
  const auto compatibility=b.serviceProfile().value("compatibility").toMap();
  QCOMPARE(compatibility.value("platform").toMap().value("apiLevel").toInt(),37);
  QCOMPARE(compatibility.value("platform").toMap().value("pageSizeBytes").toInt(),16384);
  QCOMPARE(compatibility.value("layout").toMap().value("virtualAb").toString(),QString("reported-enabled"));
  QCOMPARE(b.serviceProfile().value("vendor").toString(),QString("Google"));
  const auto path=dir_.filePath("modern-report.json");QVERIFY(b.exportServiceReport(path,true));QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));
  const auto exported=QJsonDocument::fromJson(f.readAll()).object().value("profile").toObject().value("compatibility").toObject();
  QCOMPARE(exported.value("platform").toObject().value("apiLevel").toInt(),37);QVERIFY(!exported.value("attestationVerified").toBool());
  QVERIFY(journal().contains("\"getconf\",\"PAGE_SIZE\""));
 }
 void unsupportedOptionalPageQuery(){qputenv("MR_SERVICE_MODE","modern-no-page");ToolBridge b;QVERIFY(probe(b));const auto c=b.serviceProfile().value("compatibility").toMap();QCOMPARE(c.value("platform").toMap().value("apiLevel").toInt(),37);QVERIFY(!c.value("platform").toMap().contains("pageSizeBytes"));}
 void fastbootdAndUnknownDoNotGrantOperations_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("userspace")<<QByteArray("fastbootd");QTest::newRow("unknown")<<QByteArray("unknown-fastboot-mode");QTest::newRow("duplicate")<<QByteArray("duplicate-fastboot-mode");}
 void fastbootdAndUnknownDoNotGrantOperations(){QFETCH(QByteArray,mode);qputenv("MR_SERVICE_MODE",mode);ToolBridge b;QVERIFY(probe(b,true));const auto before=journal();for(const auto &action:{"unlock_ability","erase_cache","erase_userdata","fastboot_reboot","fastboot_bootloader"}){QVERIFY(!available(b,action));QVERIFY(!b.startService(action,"TEST42",approval(true)));}QCOMPARE(journal(),before);}
 void freshFastbootModeChangeBlocksExecution(){ToolBridge b;QVERIFY(probe(b,true));qputenv("MR_SERVICE_MODE","fastbootd");QVERIFY(waitFor(b,[&]{b.startService("unlock_ability","TEST42",approval());},false));QVERIFY(!journal().contains("\"flashing\""));QVERIFY(b.serviceOutput().contains("Fastboot mode changed"));}
 void approvalTypesFailClosedBeforeProcess(){
  ToolBridge b;QVERIFY(probe(b));const auto before=journal();
  for(const auto &key:{"authorized","destructive","compatible","replace"})
   for(const auto &invalid:QVariantList{QStringLiteral("true"),1,QVariantMap{}}){
    auto a=approval(true);a.insert(key,invalid);QVERIFY(!b.startService("probe","TEST42",a));QVERIFY(!b.busy());QCOMPARE(journal(),before);
   }
  for(const auto &key:{"challenge","file","sha256"}){
   auto a=approval();a.insert(key,true);QVERIFY(!b.startService("probe","TEST42",a));QCOMPARE(journal(),before);
  }
 }
 void compiledPolicyCannotBeOverridden() {
  ToolBridge b;QVERIFY(probe(b));const auto before=journal();
  qputenv("MR_ENABLE_LAB_OPERATIONS","1");
  if(!b.laboratoryBuild()) {
   for(const auto &row:b.serviceActions()) {
    const auto a=row.toMap();const auto id=a.value("id").toString();
    if(id=="probe"||id=="unlock_ability")continue;
    QVERIFY(!a.value("available").toBool());
    auto consent=approval(true);consent.insert("file",package());consent.insert("compatible",true);
    QVERIFY(!b.startService(id,"TEST42",consent));QVERIFY(!b.busy());QCOMPARE(journal(),before);
   }
   b.startMirror();QCOMPARE(journal(),before);QVERIFY(b.status().contains("Laboratory"));
  }
  for(const auto &id:{"unknown","frp","lockscreen","shell","bootloader_unlock"})
   QVERIFY(!b.startService(id,"TEST42",approval(true)));
  QCOMPARE(journal(),before);qunsetenv("MR_ENABLE_LAB_OPERATIONS");
 }
 void exportPrivacyProjection(){
  const QVariantMap profile{{"serial","PRIVATE-SERIAL"},{"observation:battery","PRIVATE-CONTENT"},{"file","PRIVATE-PATH"},
   {"diagnostics",QVariantMap{{"usbDriver",QVariantMap{{"state","serial-correlated"},{"serial","PRIVATE-USB"},{"nodes",QVariantList{QVariantMap{{"name","PRIVATE-CUSTOMER"}}}}}},{"battery",QVariantMap{{"state","observed"},{"chargePercent",50},{"extra","PRIVATE-EXTRA"}}}}}};
  const auto safe=mr::service::reportProfile(profile);
  const auto bytes=QJsonDocument(QJsonObject::fromVariantMap(safe)).toJson();QVERIFY(!bytes.contains("PRIVATE-"));
  QCOMPARE(safe.value("diagnostics").toMap().value("battery").toMap().value("chargePercent").toInt(),50);
  const QVariantList h{QVariantMap{{"serial","PRIVATE-SERIAL"},{"result","PRIVATE-CONTENT"},{"path","PRIVATE-PATH"},{"action","probe"},{"success",false}}};
  const auto operations=QJsonDocument(QJsonArray::fromVariantList(mr::service::reportHistory(h))).toJson();
  QVERIFY(!operations.contains("PRIVATE-"));QVERIFY(operations.contains("probe"));
 }
 void packageStructureRejectsCorruption(){
  const auto path=package("preflight.apk");QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));
  QVERIFY(mr::service::validPackageStructure(f,true));const auto good=f.readAll();f.close();
  for(const auto &bad:QList<QByteArray>{QByteArray("PK\003\004fake",8),good.left(good.size()-4),good.left(6)+QByteArray(2,'\x01')+good.mid(8)}){
   QVERIFY(f.open(QIODevice::WriteOnly|QIODevice::Truncate));f.write(bad);f.close();QVERIFY(f.open(QIODevice::ReadOnly));
   QVERIFY(!mr::service::validPackageStructure(f,true));f.close();
  }
 }
 void corruptPackageStartsNoDeviceProcess(){
  if(!mr::service::laboratoryBuild())QSKIP("Package transport fixture is Laboratory-only.");
  ToolBridge b;QVERIFY(probe(b));const auto before=journal();const auto path=package("corrupt.apk");
  QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write("PK\003\004fake",8);file.close();
  auto a=approval();a.insert("file",path);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);},false));QCOMPARE(journal(),before);
 }
 void exactCommandPlanOnly() {
  using mr::service::serviceCommandAllowed;
  QVERIFY(serviceCommandAllowed("adb",{"devices","-l"},"probe","TEST42","state"));
  QVERIFY(serviceCommandAllowed("adb",{"-s","TEST42","shell","getprop"},"probe","TEST42","identity"));
  QVERIFY(serviceCommandAllowed("adb",{"-s","TEST42","shell","getconf","PAGE_SIZE"},"probe","TEST42","observation:page-size"));
  QVERIFY(!serviceCommandAllowed("adb",{"-s","TEST42","shell","getconf","PAGE_SIZE; reboot"},"probe","TEST42","observation:page-size"));
  QVERIFY(!serviceCommandAllowed("adb",{"-s","OTHER","shell","getprop"},"probe","TEST42","identity"));
  QVERIFY(!serviceCommandAllowed("adb",{"-s","TEST42","shell","getprop; reboot"},"probe","TEST42","identity"));
  QVERIFY(!serviceCommandAllowed("adb",{"-s","TEST42","shell","rm","-rf","/data"},"probe","TEST42","execute"));
  QVERIFY(!serviceCommandAllowed("adb",{"devices","-l","extra"},"probe","TEST42","state"));
  QVERIFY(!serviceCommandAllowed("adb",{"devices","-l"},"unknown","TEST42","state"));
  QVERIFY(!serviceCommandAllowed("shell",{"devices"},"probe","TEST42","state"));
  QCOMPARE(serviceCommandAllowed("adb",{"-s","TEST42","reboot"},"reboot_normal","TEST42","execute"),mr::service::laboratoryBuild());
  QCOMPARE(serviceCommandAllowed("fastboot",{"-s","TEST42","erase","userdata"},"erase_userdata","TEST42","execute"),mr::service::laboratoryBuild());
 }
 void structuredDiagnostics(){
  const QVariantMap profile{{"transport","adb"},{"observation:battery","level: 42\nscale: 50\ntemperature: 315\nstatus: 2\nhealth: 2\n"},{"observation:storage","Filesystem 1K-blocks Used Available Use% Mounted on\n/dev/block/dm-1 1000 700 250 74% /data\n"},{"observation:partitions","major minor  #blocks  name\n   8  0  1048576 sda\n   8  1  512 sda1\n"}};
  const auto report=mr::service::deviceHealth(profile);
  QCOMPARE(report.value("battery").toMap().value("chargePercent").toDouble(),84.0);
  QCOMPARE(report.value("battery").toMap().value("temperatureC").toDouble(),31.5);
  QCOMPARE(report.value("storage").toMap().value("availableKiB").toLongLong(),250);
  QCOMPARE(report.value("partitions").toMap().value("entries").toList().size(),2);
  QCOMPARE(report.value("assessment").toString(),QString("observations-only"));
 }
 void incompleteDiagnosticsDoNotClaimHealth(){
  QVariantMap p{{"transport","adb"},{"observation:battery","level: 85\n"},{"observation:storage","Permission denied"}};
  auto r=mr::service::deviceHealth(p);
  QCOMPARE(r.value("battery").toMap().value("state").toString(),QString("unavailable"));
  QVERIFY(!r.value("battery").toMap().contains("chargePercent"));
  p.insert("observation:battery","level: 50\nlevel: 99\nscale: 100\ntemperature: 999999999999999\n");
  p.insert("observation:storage","/dev/a 100 110 10 90% /data\n");
  r=mr::service::deviceHealth(p);
  QCOMPARE(r.value("battery").toMap().value("state").toString(),QString("unavailable"));
  QCOMPARE(r.value("storage").toMap().value("state").toString(),QString("unavailable"));
  p.insert("observation:storage","/dev/a 100 20 80 20% /data\n/dev/b 100 20 80 20% /data\n");
  QCOMPARE(mr::service::deviceHealth(p).value("storage").toMap().value("state").toString(),QString("unavailable"));
  p.insert("transport","fastboot");p.insert("observation:battery","level: 50\nscale: 100\n");
  QCOMPARE(mr::service::deviceHealth(p).value("battery").toMap().value("state").toString(),QString("unavailable"));
 }
 void diagnosticsIncludedInProbeAndExport(){
  ToolBridge b;QVERIFY(probe(b));
  const auto d=b.serviceProfile().value("diagnostics").toMap();
  QCOMPARE(d.value("storage").toMap().value("availableKiB").toLongLong(),80);
  QCOMPARE(d.value("partitions").toMap().value("state").toString(),QString("unavailable"));
  const auto path=dir_.filePath("diagnostics-report.json");QVERIFY(b.exportServiceReport(path,true));
  QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));
  const auto exported=QJsonDocument::fromJson(f.readAll()).object().value("profile").toObject().value("diagnostics").toObject();
  QCOMPARE(exported.value("storage").toObject().value("availableKiB").toInt(),80);
 }
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
 void capabilityIdentity_data(){QTest::addColumn<QString>("reported");QTest::addColumn<QString>("canonical");for(const auto &p:QList<QPair<QString,QString>>{{"samsung","Samsung"},{"LGE","LG"},{"HUAWEI","Huawei"},{"OPPO","Oppo"},{"vivo","Vivo"},{"ZTE","ZTE"},{"GOOGLE","Google"},{"OnePlus","OnePlus"},{"Nothing","Nothing"},{"HONOR","Honor"},{"xiaomi","Xiaomi / Redmi / POCO"}})QTest::newRow(qPrintable(p.first))<<p.first<<p.second;}
 void capabilityIdentity(){QFETCH(QString,reported);QFETCH(QString,canonical);qputenv("MR_SERVICE_BRAND",reported.toUtf8());ToolBridge b;QVERIFY(probe(b));QCOMPARE(b.serviceProfile().value("vendor").toString(),canonical);QCOMPARE(available(b,"dark_on"),b.laboratoryBuild());QCOMPARE(available(b,"languages"),b.laboratoryBuild());QVERIFY(!b.serviceProfile().contains("gsm.sim.operator.alpha"));QVERIFY(b.serviceProfile().value("observation:partitions").toString().startsWith("Unavailable"));QCOMPARE(b.requestedModels().size(),42);for(const auto &r:b.requestedModels())QVERIFY(!r.toMap().value("validated").toBool());}
 void authorizationAndBlockedActions(){ToolBridge b;QVERIFY(probe(b));const auto before=journal();for(const auto &id:{"frp","lockscreen","edl","efs","bootloader_unlock","sim_unlock"})QVERIFY(!b.startService(id,"TEST42",approval(true)));QVERIFY(!b.startService("dark_on","other",approval()));QVERIFY(!b.startService("dark_on","TEST42",{}));auto bad=approval();bad.insert("shell","rm -rf");QVERIFY(!b.startService("dark_on","TEST42",bad));QCOMPARE(journal(),before);}
 void darkModeVerification(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b));QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());}));QVERIFY(journal().contains("\"night\",\"yes\""));QVERIFY(b.serviceProfile().isEmpty());}
 void deniedAndUnconfirmedReadback_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("permission")<<QByteArray("denied");QTest::newRow("readback")<<QByteArray("bad-readback");}
 void deniedAndUnconfirmedReadback(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");QFETCH(QByteArray,mode);ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE",mode);QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());},false));QVERIFY(b.serviceProfile().isEmpty());}
 void missingSettingsAndOldAndroid_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("missing-activity")<<QByteArray("no-activity");QTest::newRow("old-sdk")<<QByteArray("old");}
 void missingSettingsAndOldAndroid(){QFETCH(QByteArray,mode);qputenv("MR_SERVICE_MODE",mode);ToolBridge b;QVERIFY(probe(b));QVERIFY(!available(b,"languages"));QVERIFY(!available(b,"keyboard"));if(mode=="old")QVERIFY(!available(b,"dark_on"));}
 void disconnectAndIdentityChange_data(){QTest::addColumn<QByteArray>("mode");QTest::newRow("usb")<<QByteArray("disconnected");QTest::newRow("identity")<<QByteArray("changed");}
 void disconnectAndIdentityChange(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");QFETCH(QByteArray,mode);ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE",mode);QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());},false));QVERIFY(!journal().contains("\"night\",\"yes\""));}
 void apkPrivateCopyIntegrity(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b));const auto path=package(),expected=hash(path);auto a=approval();a.insert("file",path);a.insert("sha256",expected);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);}));QCOMPARE(hash(path),expected);QVERIFY(journal().contains("\"install\""));QVERIFY(!journal().contains("\"-r\""));QVERIFY(!journal().contains(path.toUtf8()));QCOMPARE(b.serviceHistory()[0].toMap().value("sha256").toString(),expected);}
 void invalidPackageNeverReachesDevice(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");
  for(const auto &path:QStringList{"https://example.invalid/package.apk","//server/share/file.apk",dir_.filePath("missing.apk")}){ToolBridge b;QVERIFY(probe(b));const auto before=journal();auto a=approval();a.insert("file",path);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);},false));QCOMPARE(journal(),before);}
 }
 void explicitApkReplace(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b));auto a=approval();a.insert("file",package());a.insert("replace",true);QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);}));QVERIFY(journal().contains("\"install\",\"-r\""));}
 void ambiguousTransportAndRescanInvalidate(){ToolBridge b;QVERIFY(probe(b));QVERIFY(waitFor(b,[&]{b.scanAdb();}));QVERIFY(b.serviceProfile().isEmpty());QVERIFY(!available(b,"dark_on"));QVERIFY(waitFor(b,[&]{b.scanFastboot();}));QVERIFY(!available(b,"probe"));}
 void duplicateSerialRejectedBeforeWrite_data(){QTest::addColumn<bool>("fast");QTest::newRow("adb")<<false;QTest::newRow("fastboot")<<true;}
 void duplicateSerialRejectedBeforeWrite(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");QFETCH(bool,fast);ToolBridge b;QVERIFY(probe(b,fast));qputenv("MR_SERVICE_MODE","duplicate");QVERIFY(waitFor(b,[&]{b.startService(fast?"erase_cache":"dark_on","TEST42",approval(true));},false));QVERIFY(!journal().contains("\"erase\""));QVERIFY(!journal().contains("\"night\",\"yes\""));QVERIFY(b.serviceOutput().contains("ambiguous"));}
 void apkHashAndManagerFailure_data(){QTest::addColumn<QByteArray>("mode");QTest::addColumn<bool>("wrongHash");QTest::newRow("hash")<<QByteArray()<<true;QTest::newRow("manager")<<QByteArray("install-failure")<<false;QTest::newRow("no-confirmation")<<QByteArray("no-success")<<false;}
 void apkHashAndManagerFailure(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");QFETCH(QByteArray,mode);QFETCH(bool,wrongHash);ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE",mode);auto a=approval();a.insert("file",package());if(wrongHash)a.insert("sha256",QString(64,'0'));QVERIFY(waitFor(b,[&]{b.startService("install_apk","TEST42",a);},false));if(wrongHash)QVERIFY(!journal().contains("\"install\""));}
 void sideloadConsentAndMode(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");qputenv("MR_SERVICE_MODE","sideload");ToolBridge b;QVERIFY(waitFor(b,[&]{b.scanAdb();}));b.selectDevice("TEST42");QVERIFY(!available(b,"probe"));QVERIFY(available(b,"sideload_zip"));const auto path=package("fixture.zip");auto a=approval(true);a.insert("file",path);a.insert("sha256",hash(path));QVERIFY(!b.startService("sideload_zip","TEST42",a));a.insert("compatible",true);QVERIFY(waitFor(b,[&]{b.startService("sideload_zip","TEST42",a);}));QVERIFY(b.serviceOutput().contains("not proof"));QVERIFY(!journal().contains("\"getprop\""));}
 void fastbootFreshEraseGuard(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b,true));QVERIFY(available(b,"erase_userdata"));QVERIFY(!b.startService("erase_userdata","TEST42",approval()));qputenv("MR_SERVICE_MODE","locked");QVERIFY(waitFor(b,[&]{b.startService("erase_userdata","TEST42",approval(true));},false));QVERIFY(!journal().contains("\"erase\""));}
 void fastbootCrLfQueries(){qputenv("MR_SERVICE_MODE","crlf");ToolBridge b;QVERIFY(probe(b,true));QCOMPARE(b.serviceProfile().value("product").toString(),QStringLiteral("training-model"));QCOMPARE(available(b,"erase_cache"),b.laboratoryBuild());}
 void syntheticErase(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b,true));QVERIFY(waitFor(b,[&]{b.startService("erase_cache","TEST42",approval(true));}));QVERIFY(journal().contains("\"erase\",\"cache\""));}
 void noPartitionNoErase(){qputenv("MR_SERVICE_MODE","no-partition");ToolBridge b;QVERIFY(probe(b,true));QVERIFY(!available(b,"erase_userdata"));QVERIFY(!available(b,"erase_cache"));}
 void rebootInvalidatesDiscovery(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b));QVERIFY(waitFor(b,[&]{b.startService("reboot_recovery","TEST42",approval());}));QVERIFY(b.selectedSerial().isEmpty());QVERIFY(b.serviceProfile().isEmpty());QVERIFY(journal().contains("\"reboot\",\"recovery\""));}
 void cancellation(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");ToolBridge b;QVERIFY(probe(b));qputenv("MR_SERVICE_MODE","hang");QSignalSpy spy(&b,&ToolBridge::operationFinished);QVERIFY(b.startService("dark_on","TEST42",approval()));QTRY_VERIFY_WITH_TIMEOUT(journal().contains("\"night\",\"yes\""),5000);b.cancel();if(spy.isEmpty())QVERIFY(spy.wait(5000));QVERIFY(!b.busy());QCOMPARE(spy.last()[0].toBool(),false);QVERIFY(!journal().contains("\"uimode\",\"night\"]"));}
 void historyRetainsTargetAndFailureAfterAnotherOperation(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");
  ToolBridge b;QVERIFY(probe(b));
  qputenv("MR_SERVICE_MODE","disconnected");
  QVERIFY(waitFor(b,[&]{b.startService("dark_on","TEST42",approval());},false));
  const auto failed=b.serviceHistory().first().toMap();
  QCOMPARE(failed.value("serial").toString(),QStringLiteral("TEST42"));
  QCOMPARE(failed.value("transport").toString(),QStringLiteral("adb"));
  QVERIFY(!failed.value("success").toBool());
  QVERIFY(failed.value("result").toString().contains("missing"));
  QVERIFY(b.serviceProfile().isEmpty());
  // Refresh the disconnected ADB transport before discovering the same serial
  // in Fastboot; simultaneous serials are deliberately rejected as ambiguous.
  QVERIFY(waitFor(b,[&]{b.scanAdb();}));
  qunsetenv("MR_SERVICE_MODE");QVERIFY(probe(b,true));
  QCOMPARE(b.serviceHistory().first().toMap().value("transport").toString(),QStringLiteral("fastboot"));
  QCOMPARE(b.serviceHistory().at(1).toMap(),failed);
  const auto path=dir_.filePath("history-report.json");QFile::remove(path);
  QVERIFY(b.exportServiceReport(path,true));QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));
  const auto report=QJsonDocument::fromJson(file.readAll()).object();
  QCOMPARE(report.value("schemaVersion").toInt(),3);
  const auto operations=report.value("operations").toArray();
  QCOMPARE(operations.size(),3);
  QCOMPARE(operations.at(1).toObject().toVariantMap(),mr::service::reportHistory({failed}).first().toMap());
 }
 void rebootHistoryKeepsTargetAfterDiscoveryCleared(){if(!mr::service::laboratoryBuild())QSKIP("Write fixture runs only in the separate Laboratory build; production denial is tested independently.");
  ToolBridge b;QVERIFY(probe(b));
  QVERIFY(waitFor(b,[&]{b.startService("reboot_recovery","TEST42",approval());}));
  QVERIFY(b.selectedSerial().isEmpty());QVERIFY(b.serviceProfile().isEmpty());
  const auto record=b.serviceHistory().first().toMap();
  QCOMPARE(record.value("serial").toString(),QStringLiteral("TEST42"));
  QCOMPARE(record.value("action").toString(),QStringLiteral("reboot_recovery"));
  QCOMPARE(record.value("result").toString(),b.serviceOutput());
 }
 void reportConsentAndNoOverwrite(){ToolBridge b;QVERIFY(probe(b));const auto path=dir_.filePath("service-report.json");QFile::remove(path);QVERIFY(!b.exportServiceReport(path,false));QVERIFY(b.exportServiceReport(path,true));QVERIFY(!b.exportServiceReport(path,true));QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));const auto bytes=f.readAll();QVERIFY(bytes.contains("Pending Hardware Validation"));QVERIFY(!bytes.contains("PRIVATE"));}
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);const auto base=QFileInfo(app.applicationFilePath()).completeBaseName();if(base=="adb"||base=="fastboot")return child(app.arguments().mid(1),base=="fastboot");ServiceTests tests;return QTest::qExec(&tests,argc,argv);}
#include "service_tests.moc"

