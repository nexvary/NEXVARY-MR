#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "CloudTracking.hpp"
#include "TrackingPortal.hpp"
#include "WorkshopBridge.hpp"
#include "ToolBridge.hpp"
#include "SecurityBridge.hpp"
#include "RecoveryBridge.hpp"
#include <QtTest>
#include <QTemporaryDir>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QDesktopServices>
#include <QUrl>
#include <cstdio>
namespace {
const QString password="Synthetic-only offline policy 2026!";
bool write(const QString &path,const QByteArray &data){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(data)==data.size();}
void admin(IdentityService &i){if(!i.bootstrap("policy-admin","Synthetic policy administrator",password)||!i.login("policy-admin",password))qFatal("Synthetic administrator fixture failed");}
int child(const QStringList &args){
 const auto marker=qEnvironmentVariable("MR_OFFLINE_MARKER");
 if(!marker.isEmpty())write(marker,"child started");
 if(qEnvironmentVariable("MR_OFFLINE_MODE")=="hang")QThread::msleep(10000);
 if(args==QStringList{"devices","-l"}){std::puts("List of devices attached\nSYNTHETIC42 device model:Fixture");return 0;}
 if(args==QStringList{"--version"}){std::puts("ClamAV synthetic policy fixture");return 0;}
 if(args==QStringList{"-V"}){std::puts("The Sleuth Kit ver synthetic policy fixture");return 0;}
 for(const auto &a:args)if(a.startsWith("--datadir=")){const auto d=a.mid(10);return write(QDir(d).filePath("main.cvd"),"synthetic-main")&&write(QDir(d).filePath("daily.cvd"),"synthetic-daily")?0:5;}
 std::puts("synthetic: OK");return 0;
}
}
class OfflineTests final:public QObject {
 Q_OBJECT
 QUrl capturedUrl;
public slots:
 void captureUrl(const QUrl &url){capturedUrl=url;}
private slots:
 void cleanup(){QDesktopServices::unsetUrlHandler("https");capturedUrl=QUrl();qunsetenv("MR_OFFLINE_MARKER");qunsetenv("MR_OFFLINE_MODE");}
 void defaultDenyAndExplicitAdministrator(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);GovernmentOperationPolicy policy(&identity);
  QVERIFY(policy.offline());QCOMPARE(policy.permissions().size(),6);
  for(const auto &cap:policy.capabilities()){QVERIFY(!policy.permit(cap));QVERIFY(!policy.grant(cap,"synthetic",true));}
  admin(identity);
  QVERIFY(!policy.grant("unknown","synthetic",true));QVERIFY(!policy.grant("device-tools"," ",true));QVERIFY(!policy.grant("device-tools",QString(257,'x'),true));QVERIFY(!policy.grant("device-tools","synthetic",false));
  QVERIFY(policy.grant("device-tools",QStringLiteral("تجربة اصطناعية مصرح بها"),true));QVERIFY(!policy.offline());QVERIFY(policy.permit("device-tools"));QVERIFY(!policy.permit("cloud-publish"));
  const auto rows=policy.permissions();QCOMPARE(rows[3].toMap().value("reason").toString(),QStringLiteral("تجربة اصطناعية مصرح بها"));QVERIFY(!rows[3].toMap().value("actor").toString().isEmpty());
  identity.logout();QVERIFY(policy.offline());QVERIFY(!policy.permit("device-tools"));
  QVERIFY(identity.login("policy-admin",password));QVERIFY(policy.offline());
  QVERIFY(!identity.createUser("reader","Synthetic investigator","investigator",password).isEmpty());QVERIFY(identity.login("reader",password));QVERIFY(!policy.grant("cloud-publish","synthetic",true));
 }
 void expiryAndExternalAccountRevocation(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);
  GovernmentOperationPolicy shortPolicy(&identity,nullptr,100);QVERIFY(shortPolicy.grant("device-tools","synthetic",true));QTRY_VERIFY_WITH_TIMEOUT(shortPolicy.offline(),2000);QVERIFY(!shortPolicy.permit("device-tools"));
  const auto other=identity.createUser("other-admin","Other synthetic administrator","administrator",password);QVERIFY(!other.isEmpty());
  GovernmentOperationPolicy policy(&identity);QVERIFY(policy.grant("cloud-publish","synthetic",true));const auto current=identity.userId();
  IdentityService independent(t.filePath("identity"),true);QVERIFY(independent.login("other-admin",password));QVERIFY(independent.setEnabled(current,false));
  QVERIFY(!policy.permit("cloud-publish"));QVERIFY(policy.offline());QVERIFY(!identity.authenticated());
 }
 void restartDoesNotRestoreExceptions(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);
  {GovernmentOperationPolicy policy(&identity);QVERIFY(policy.grant("external-analysis","synthetic",true));}
  GovernmentOperationPolicy reopened(&identity);QVERIFY(reopened.offline());QVERIFY(!reopened.permit("external-analysis"));
 }
 void portalDefaultDenyAndClientAbort(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  WorkshopBridge workshop(t.filePath("workshop.db"));TrackingPortal portal(&workshop);portal.setOperationPolicy(&policy);
  QVERIFY(!portal.start("127.0.0.1",0,true));QVERIFY(!portal.running());QVERIFY(policy.grant("cloud-publish","unrelated category",true));QVERIFY(!portal.start("127.0.0.1",0,true));
  QVERIFY(policy.grant("tracking-portal","synthetic loopback only",true));QVERIFY(portal.start("127.0.0.1",0,true));
  const QUrl url(portal.baseUrl());QTcpSocket client;client.connectToHost(url.host(),static_cast<quint16>(url.port()));QTRY_COMPARE_WITH_TIMEOUT(client.state(),QTcpSocket::ConnectedState,2000);
  client.write("GET /t/");client.flush();QTest::qWait(40);policy.revoke("tracking-portal");QVERIFY(!portal.running());QTRY_COMPARE_WITH_TIMEOUT(client.state(),QTcpSocket::UnconnectedState,2000);QVERIFY(client.readAll().isEmpty());
  QVERIFY(policy.grant("tracking-portal","synthetic restart",true));QVERIFY(!portal.running());QVERIFY(portal.start("127.0.0.1",0,true));identity.logout();QVERIFY(!portal.running());
 }
 void deviceChildDeniedPermittedAndRevoked(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  ToolBridge tool;tool.setOperationPolicy(&policy);QVERIFY(tool.configureTool("adb",QCoreApplication::applicationFilePath()));
  const auto marker=t.filePath("started");qputenv("MR_OFFLINE_MARKER",marker.toUtf8());
  tool.scanAdb();QVERIFY(!tool.busy());QVERIFY(!QFileInfo::exists(marker));tool.setAutoRefresh(true);QVERIFY(!tool.autoRefresh());
  QVERIFY(!tool.startService("inspect","SYNTHETIC42",{}));QVERIFY(!tool.startMirror("SYNTHETIC42",{}));
  QVERIFY(policy.grant("device-tools","synthetic child only",true));tool.scanAdb();QTRY_VERIFY_WITH_TIMEOUT(!tool.busy(),5000);QVERIFY(QFileInfo::exists(marker));QCOMPARE(tool.devices().size(),1);
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");tool.scanAdb();QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);QVERIFY(tool.busy());policy.revoke("device-tools");QTRY_VERIFY_WITH_TIMEOUT(!tool.busy(),5000);
  QVERIFY(tool.devices().isEmpty());QVERIFY(!tool.autoRefresh());QVERIFY(QFile::remove(marker));tool.scanAdb();QVERIFY(!tool.busy());QTest::qWait(100);QVERIFY(!QFileInfo::exists(marker));
 }
 void externalAnalyzersAndUpdaterAreSeparatePermissions(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  const auto db=t.filePath("db");QVERIFY(QDir().mkpath(db));QVERIFY(write(db+"/synthetic.ndb","N:0:*:41414141\n"));QVERIFY(write(t.filePath("source"),"ordinary synthetic sample"));
#ifdef Q_OS_WIN
  const auto engine=t.filePath("clamscan.exe"),updater=t.filePath("freshclam.exe");
#else
  const auto engine=t.filePath("clamscan"),updater=t.filePath("freshclam");
#endif
  for(const auto &p:{engine,updater}){QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),p));QVERIFY(QFile::setPermissions(p,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));}
  SecurityBridge security(t.filePath("scanner.json"));security.setOperationPolicy(&policy);QVERIFY(security.configure(engine,db));
  const auto marker=t.filePath("started");qputenv("MR_OFFLINE_MARKER",marker.toUtf8());
  QVERIFY(!security.scan(t.filePath("source"),true));QVERIFY(!security.updateDatabase(true));QVERIFY(!QFileInfo::exists(marker));
  QVERIFY(policy.grant("external-analysis","synthetic scan fixture",true));QVERIFY(!security.updateDatabase(true));QVERIFY(security.scan(t.filePath("source"),true));QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY2(security.error().isEmpty(),qPrintable(security.error()));QVERIFY(security.result().contains("verdict"));
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");QVERIFY(security.scan(t.filePath("source"),true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);policy.revoke("external-analysis");QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY(!security.result().contains("verdict"));QVERIFY(!security.error().isEmpty());
  qunsetenv("MR_OFFLINE_MODE");QVERIFY(policy.grant("signature-update","synthetic updater fixture",true));QVERIFY(security.updateDatabase(true));QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY2(security.updateResult().value("success").toBool(),qPrintable(security.error()));
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");QVERIFY(security.updateDatabase(true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);identity.logout();QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY(!security.updateResult().value("success").toBool());
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);QVERIFY(!recovery.analyzePartitions(t.filePath("source"),engine,true));QVERIFY(recovery.error().contains("offline policy"));
 }
 void externalPartitionChildPermissionAndRevoke(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);
  QVERIFY(recovery.createCase("Synthetic policy TSK","Lab only","Lab","No phone","","backup","unknown",true));QVERIFY(recovery.recordOperator("Synthetic operator","Synthetic raw fixture only",true));
  const auto image=t.filePath("image.dd");QVERIFY(write(image,QByteArray(4096,'x')));const auto exe=QCoreApplication::applicationFilePath();
  const auto marker=t.filePath("started");qputenv("MR_OFFLINE_MARKER",marker.toUtf8());QVERIFY(!recovery.analyzePartitions(image,exe,true));QVERIFY(!QFileInfo::exists(marker));
  QVERIFY(policy.grant("external-analysis","Synthetic partition fixture",true));QVERIFY(recovery.analyzePartitions(image,exe,true));QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);QVERIFY2(recovery.error().isEmpty(),qPrintable(recovery.error()));QCOMPARE(recovery.evidence().size(),1);
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");QVERIFY(recovery.analyzePartitions(image,exe,true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);policy.revoke("external-analysis");QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);QVERIFY(!recovery.error().isEmpty());QCOMPARE(recovery.evidence().size(),1);
 }
 void browserHandoffBlockedAndProtocolsRestricted(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  QVERIFY(!policy.openExternal("https://example.invalid/"));QVERIFY(policy.grant("external-links","synthetic scheme rejection only",true));
  for(const auto &url:{"file:///tmp/test","javascript:alert(1)","mailto:test@example.invalid","https://user:secret@example.invalid/","https:///","not a URL"})QVERIFY(!policy.openExternal(url));
  // Register an in-process receiver to verify the permitted native handoff
  // without launching a browser or contacting any public endpoint.
  QDesktopServices::setUrlHandler("https",this,"captureUrl");
  QVERIFY(policy.openExternal("https://example.invalid/approved-synthetic"));QCOMPARE(capturedUrl,QUrl("https://example.invalid/approved-synthetic"));
  policy.revoke("external-links");QVERIFY(!policy.openExternal("https://example.invalid/denied-synthetic"));QCOMPARE(capturedUrl,QUrl("https://example.invalid/approved-synthetic"));
 }
};
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);const auto args=app.arguments().mid(1);
 if(args==QStringList{"devices","-l"}||args.contains("--version")||args.contains("-V")||args.value(0)=="--stdout"||args.value(0)=="-i")return child(args);
 OfflineTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "offline_tests.moc"
