#include "SyntheticPolicyExercise.hpp"
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
#include <QThreadPool>
#include <QProcess>
#include <QElapsedTimer>
#include <QCryptographicHash>
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
 if(qEnvironmentVariable("MR_OFFLINE_MODE")=="descendant"){
  if(!QProcess::startDetached(QCoreApplication::applicationFilePath(),{"--synthetic-descendant",qEnvironmentVariable("MR_OFFLINE_DESCENDANT_DIR")}))return 96;
  QThread::msleep(10000);
 }
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
 void cleanup(){QDesktopServices::unsetUrlHandler("https");capturedUrl=QUrl();qunsetenv("MR_OFFLINE_MARKER");qunsetenv("MR_OFFLINE_MODE");qunsetenv("MR_OFFLINE_DESCENDANT_DIR");}
 void defaultDenyAndExplicitAdministrator(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);GovernmentOperationPolicy policy(&identity);
  QVERIFY(policy.offline());QCOMPARE(policy.permissions().size(),6);
  for(const auto &cap:policy.capabilities()){QVERIFY(!policy.permit(cap));QVERIFY(!policy.grant(cap,"synthetic",true));}
  admin(identity);
  QVERIFY(!policy.grant("unknown","synthetic",true));QVERIFY(!policy.grant("device-tools"," ",true));QVERIFY(!policy.grant("device-tools",QString(257,'x'),true));QVERIFY(!policy.grant("device-tools","synthetic",false));
  QVERIFY(performSyntheticReviewedException(identity,policy,"device-tools",QStringLiteral("تجربة اصطناعية مصرح بها"),password));QVERIFY(!policy.offline());QVERIFY(policy.permit("device-tools"));QVERIFY(!policy.permit("cloud-publish"));
  const auto rows=policy.permissions();QCOMPARE(rows[3].toMap().value("reason").toString(),QStringLiteral("تجربة اصطناعية مصرح بها"));QVERIFY(!rows[3].toMap().value("actor").toString().isEmpty());
  identity.logout();QVERIFY(policy.offline());QVERIFY(!policy.permit("device-tools"));
  QVERIFY(identity.login("policy-admin",password));QVERIFY(policy.offline());
  QVERIFY(!identity.createUser("reader","Synthetic investigator","investigator",password).isEmpty());QVERIFY(identity.login("reader",password));QVERIFY(!policy.grant("cloud-publish","synthetic",true));
 }
 void expiryAndExternalAccountRevocation(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);
  GovernmentOperationPolicy shortPolicy(&identity,nullptr,100);QVERIFY(performSyntheticReviewedException(identity,shortPolicy,"device-tools","synthetic",password));QTRY_VERIFY_WITH_TIMEOUT(shortPolicy.offline(),2000);QVERIFY(!shortPolicy.permit("device-tools"));
  const auto other=identity.createUser("other-admin","Other synthetic administrator","administrator",password);QVERIFY(!other.isEmpty());
  GovernmentOperationPolicy policy(&identity);QVERIFY(performSyntheticReviewedException(identity,policy,"cloud-publish","synthetic",password));const auto current=identity.userId();
  IdentityService independent(t.filePath("identity"),true);QVERIFY(independent.login("other-admin",password));QVERIFY(independent.setEnabled(current,false));
  QVERIFY(!policy.permit("cloud-publish"));QVERIFY(policy.offline());QVERIFY(!identity.authenticated());
 }
 void restartDoesNotRestoreExceptions(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);
  {GovernmentOperationPolicy policy(&identity);QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","synthetic",password));}
  GovernmentOperationPolicy reopened(&identity);QVERIFY(reopened.offline());QVERIFY(!reopened.permit("external-analysis"));
 }
 void portalDefaultDenyAndClientAbort(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  WorkshopBridge workshop(t.filePath("workshop.db"));TrackingPortal portal(&workshop);portal.setOperationPolicy(&policy);
  QVERIFY(!portal.start("127.0.0.1",0,true));QVERIFY(!portal.running());QVERIFY(performSyntheticReviewedException(identity,policy,"cloud-publish","unrelated category",password));QVERIFY(!portal.start("127.0.0.1",0,true));
  QVERIFY(performSyntheticReviewedException(identity,policy,"tracking-portal","synthetic loopback only",password));QVERIFY(portal.start("127.0.0.1",0,true));
  const QUrl url(portal.baseUrl());QTcpSocket client;client.connectToHost(url.host(),static_cast<quint16>(url.port()));QTRY_COMPARE_WITH_TIMEOUT(client.state(),QTcpSocket::ConnectedState,2000);
  client.write("GET /t/");client.flush();QTest::qWait(40);policy.revoke("tracking-portal");QVERIFY(!portal.running());QTRY_COMPARE_WITH_TIMEOUT(client.state(),QTcpSocket::UnconnectedState,2000);QVERIFY(client.readAll().isEmpty());
  QVERIFY(performSyntheticReviewedException(identity,policy,"tracking-portal","synthetic restart",password));QVERIFY(!portal.running());QVERIFY(portal.start("127.0.0.1",0,true));identity.logout();QVERIFY(!portal.running());
 }
 void deviceChildDeniedPermittedAndRevoked(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  ToolBridge tool;tool.setOperationPolicy(&policy);QVERIFY(tool.configureTool("adb",QCoreApplication::applicationFilePath()));
  const auto marker=t.filePath("started");qputenv("MR_OFFLINE_MARKER",marker.toUtf8());
  tool.scanAdb();QVERIFY(!tool.busy());QVERIFY(!QFileInfo::exists(marker));tool.setAutoRefresh(true);QVERIFY(!tool.autoRefresh());
  QVERIFY(!tool.startService("inspect","SYNTHETIC42",{}));QVERIFY(!tool.startMirror("SYNTHETIC42",{}));
  QVERIFY(performSyntheticReviewedException(identity,policy,"device-tools","synthetic child only",password));tool.scanAdb();QTRY_VERIFY_WITH_TIMEOUT(!tool.busy(),5000);QVERIFY(QFileInfo::exists(marker));QCOMPARE(tool.devices().size(),1);
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
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","synthetic scan fixture",password));QVERIFY(!security.updateDatabase(true));QVERIFY(security.scan(t.filePath("source"),true));QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY2(security.error().isEmpty(),qPrintable(security.error()));QVERIFY(security.result().contains("verdict"));
  QCOMPARE(security.operationReceipt().value("status").toString(),QString("completed"));
  const auto report=t.filePath("scan-report.json");QVERIFY(security.exportReport(report,true));QFile exported(report);QVERIFY(exported.open(QIODevice::ReadOnly));
  QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(exported.readAll(),QCryptographicHash::Sha256).toHex()),security.operationReceipt().value("outputSha256").toString());
  QVERIFY(security.scan(t.filePath("missing"),true));QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QCOMPARE(security.operationReceipt().value("status").toString(),QString("failed"));
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");QVERIFY(security.scan(t.filePath("source"),true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);policy.revoke("external-analysis");QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY(!security.result().contains("verdict"));QVERIFY(!security.error().isEmpty());
  QCOMPARE(security.operationReceipt().value("status").toString(),QString("revoked"));
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic operator cancellation",password));
  QVERIFY(QFile::remove(marker));QVERIFY(security.scan(t.filePath("source"),true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);security.cancel();QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);
  QCOMPARE(security.operationReceipt().value("failureClass").toString(),QString("operator-cancelled"));
  qunsetenv("MR_OFFLINE_MODE");QVERIFY(performSyntheticReviewedException(identity,policy,"signature-update","synthetic updater fixture",password));QVERIFY(security.updateDatabase(true));QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY2(security.updateResult().value("success").toBool(),qPrintable(security.error()));
  QCOMPARE(security.operationReceipt().value("status").toString(),QString("completed"));
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");QVERIFY(security.updateDatabase(true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);identity.logout();QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),5000);QVERIFY(!security.updateResult().value("success").toBool());
  QCOMPARE(security.operationReceipt().value("failureClass").toString(),QString("session-ended"));
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);QVERIFY(!recovery.analyzePartitions(t.filePath("source"),engine,true));QVERIFY(recovery.error().contains("offline policy"));
 }
 void scannerExpiryResult(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity,nullptr,2000);
  const auto db=t.filePath("db");QVERIFY(QDir().mkpath(db));QVERIFY(write(db+"/synthetic.ndb","N:0:*:41414141\n"));QVERIFY(write(t.filePath("source"),"synthetic only"));
#ifdef Q_OS_WIN
  const auto engine=t.filePath("clamscan.exe");
#else
  const auto engine=t.filePath("clamscan");
#endif
  QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),engine));QVERIFY(QFile::setPermissions(engine,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));
  SecurityBridge security(t.filePath("scanner.json"));security.setOperationPolicy(&policy);QVERIFY(security.configure(engine,db));
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic running-worker expiry",password));
  const auto marker=t.filePath("started");qputenv("MR_OFFLINE_MARKER",marker.toUtf8());qputenv("MR_OFFLINE_MODE","hang");
  QVERIFY(security.scan(t.filePath("source"),true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);QTRY_VERIFY_WITH_TIMEOUT(!security.busy(),8000);
  QCOMPARE(security.operationReceipt().value("status").toString(),QString("expired"));QVERIFY(!security.result().contains("verdict"));
 }
 void externalPartitionChildPermissionAndRevoke(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);
  QVERIFY(recovery.createCase("Synthetic policy TSK","Lab only","Lab","No phone","","backup","unknown",true));QVERIFY(recovery.recordOperator("Synthetic operator","Synthetic raw fixture only",true));
  const auto image=t.filePath("image.dd");QVERIFY(write(image,QByteArray(4096,'x')));const auto exe=QCoreApplication::applicationFilePath();
  const auto marker=t.filePath("started");qputenv("MR_OFFLINE_MARKER",marker.toUtf8());QVERIFY(!recovery.analyzePartitions(image,exe,true));QVERIFY(!QFileInfo::exists(marker));
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic partition fixture",password));QVERIFY(recovery.analyzePartitions(image,exe,true));QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);QVERIFY2(recovery.error().isEmpty(),qPrintable(recovery.error()));QCOMPARE(recovery.evidence().size(),1);
  const auto receipt=recovery.operationReceipt();QCOMPARE(receipt.value("kind").toString(),QString("partition-analysis"));QCOMPARE(receipt.value("status").toString(),QString("completed"));
  QCOMPARE(receipt.value("sourceSha256").toString(),QString::fromLatin1(QCryptographicHash::hash(QByteArray(4096,'x'),QCryptographicHash::Sha256).toHex()));
  const auto exported=t.filePath("partition.json");QVERIFY(recovery.exportEvidence(0,exported,true));QFile report(exported);QVERIFY(report.open(QIODevice::ReadOnly));
  QCOMPARE(receipt.value("outputSha256").toString(),QString::fromLatin1(QCryptographicHash::hash(report.readAll(),QCryptographicHash::Sha256).toHex()));
  QVERIFY(QFile::remove(marker));qputenv("MR_OFFLINE_MODE","hang");QVERIFY(recovery.analyzePartitions(image,exe,true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);policy.revoke("external-analysis");QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);QVERIFY(!recovery.error().isEmpty());QCOMPARE(recovery.evidence().size(),1);
  QCOMPARE(recovery.operationReceipt().value("status").toString(),QString("revoked"));
 }
 void partitionRunningClosure_data(){
  QTest::addColumn<QString>("action");QTest::addColumn<QString>("status");
  QTest::newRow("operator-cancel")<<QString("cancel")<<QString("cancelled");
  QTest::newRow("logout")<<QString("logout")<<QString("cancelled");
  QTest::newRow("expiry")<<QString("expire")<<QString("expired");
 }
 void partitionRunningClosure(){
  QFETCH(QString,action);QFETCH(QString,status);
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity,nullptr,action=="expire"?1500:600000);
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);
  QVERIFY(recovery.createCase("Synthetic partition closure","Lab","Lab","No phone","","backup","unknown",true));QVERIFY(recovery.recordOperator("Synthetic operator","Synthetic fixture",true));
  const auto image=t.filePath("image.dd"),marker=t.filePath("started");QVERIFY(write(image,QByteArray(4096,'x')));
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic running partition closure",password));
  qputenv("MR_OFFLINE_MARKER",marker.toUtf8());qputenv("MR_OFFLINE_MODE","hang");
  QVERIFY(recovery.analyzePartitions(image,QCoreApplication::applicationFilePath(),true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(marker),3000);
  if(action=="cancel")recovery.cancel();else if(action=="logout")identity.logout();
  QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),8000);QCOMPARE(recovery.operationReceipt().value("status").toString(),status);QCOMPARE(recovery.evidence().size(),0);
 }
 void partitionCompletionQueuedRevocation(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);
  QVERIFY(recovery.createCase("Synthetic queued callback","Lab","Lab","No phone","","backup","unknown",true));QVERIFY(recovery.recordOperator("Synthetic operator","Synthetic fixture",true));
  const auto image=t.filePath("image.dd");QVERIFY(write(image,QByteArray(4096,'x')));
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic queued result",password));
  QFile beforeFile(recovery.casePath());QVERIFY(beforeFile.open(QIODevice::ReadOnly));const auto before=beforeFile.readAll();beforeFile.close();
  QVERIFY(recovery.analyzePartitions(image,QCoreApplication::applicationFilePath(),true));
  // Let the worker finish without dispatching its queued GUI callback. No
  // timing guess or automatic retry: the pool barrier proves job completion.
  QVERIFY(QThreadPool::globalInstance()->waitForDone(5000));QVERIFY(recovery.busy());
  QFile pending(recovery.casePath());QVERIFY(pending.open(QIODevice::ReadOnly));QCOMPARE(pending.readAll(),before);pending.close();
  policy.revoke("external-analysis");QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);
  QCOMPARE(recovery.operationReceipt().value("status").toString(),QString("revoked"));QCOMPARE(recovery.evidence().size(),0);
  QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic failed input",password));
  QVERIFY(recovery.analyzePartitions(t.filePath("missing.dd"),QCoreApplication::applicationFilePath(),true));QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);
  QCOMPARE(recovery.operationReceipt().value("status").toString(),QString("failed"));QCOMPARE(recovery.evidence().size(),0);
 }
 void browserHandoffBlockedAndProtocolsRestricted(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  QVERIFY(!policy.openExternal("https://example.invalid/"));QVERIFY(performSyntheticReviewedException(identity,policy,"external-links","synthetic scheme rejection only",password));
  for(const auto &url:{"file:///tmp/test","javascript:alert(1)","mailto:test@example.invalid","https://user:secret@example.invalid/","https:///","not a URL"})QVERIFY(!policy.openExternal(url));
  // Register an in-process receiver to verify the permitted native handoff
  // without launching a browser or contacting any public endpoint.
  QDesktopServices::setUrlHandler("https",this,"captureUrl");
  QVERIFY(policy.openExternal("https://example.invalid/approved-synthetic"));QCOMPARE(capturedUrl,QUrl("https://example.invalid/approved-synthetic"));
  policy.revoke("external-links");QVERIFY(!policy.openExternal("https://example.invalid/denied-synthetic"));QCOMPARE(capturedUrl,QUrl("https://example.invalid/approved-synthetic"));
 }
 void partitionStoreFailure_data(){
  QTest::addColumn<bool>("caseStore");QTest::newRow("case-write-and-restore-failure")<<true;QTest::newRow("policy-journal-corruption")<<false;
 }
 void externalAnalyzerDescendantBoundary(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);
  QVERIFY(recovery.createCase("Synthetic descendant boundary","Lab","Lab","No phone","","backup","unknown",true));QVERIFY(recovery.recordOperator("Synthetic operator","Bounded marker-only fixture",true));
  const auto image=t.filePath("image.dd");QVERIFY(write(image,QByteArray(4096,'x')));QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic descendant observation",password));
  qputenv("MR_OFFLINE_MODE","descendant");qputenv("MR_OFFLINE_DESCENDANT_DIR",t.path().toUtf8());
  QVERIFY(recovery.analyzePartitions(image,QCoreApplication::applicationFilePath(),true));QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(t.filePath("descendant-ready")),5000);
  policy.revoke("external-analysis");QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);QCOMPARE(recovery.operationReceipt().value("status").toString(),QString("revoked"));QCOMPARE(recovery.evidence().size(),0);
  // The descendant waits for this marker, so its later write proves survival
  // beyond native worker termination/refusal. No network traffic or payload.
  QVERIFY(write(t.filePath("parent-revoked"),"synthetic parent worker has returned"));
  QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(t.filePath("descendant-after-revoke")),3000);
  qInfo("Boundary observation: controlled detached descendant survived revoked external analyzer; application gate is not process-tree containment.");
 }
 void partitionStoreFailure(){
  QFETCH(bool,caseStore);
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);admin(identity);GovernmentOperationPolicy policy(&identity);
  RecoveryBridge recovery(t.filePath("cases"),true);recovery.setOperationPolicy(&policy);
  QVERIFY(recovery.createCase("Synthetic store failure","Lab","Lab","No phone","","backup","unknown",true));QVERIFY(recovery.recordOperator("Synthetic operator","Synthetic fixture",true));
  const auto image=t.filePath("image.dd");QVERIFY(write(image,QByteArray(4096,'x')));QVERIFY(performSyntheticReviewedException(identity,policy,"external-analysis","Synthetic store failure",password));
  QVERIFY(recovery.analyzePartitions(image,QCoreApplication::applicationFilePath(),true));QVERIFY(QThreadPool::globalInstance()->waitForDone(5000));
  const auto path=recovery.casePath(),backup=path+".preserved";
  if(caseStore){QVERIFY(QFile::rename(path,backup));QVERIFY(QDir().mkdir(path));}
  else QVERIFY(write(identity.policyJournalPath(),"synthetic corrupted journal"));
  QTRY_VERIFY_WITH_TIMEOUT(!recovery.busy(),5000);QVERIFY(!recovery.error().isEmpty());QCOMPARE(recovery.evidence().size(),0);
  if(caseStore){QCOMPARE(recovery.operationReceipt().value("status").toString(),QString("failed"));QVERIFY(recovery.error().contains("Case restore/audit failed"));QVERIFY(QDir().rmdir(path));QVERIFY(QFile::rename(backup,path));}
  else QVERIFY(recovery.operationReceipt().isEmpty());
 }
};
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);const auto args=app.arguments().mid(1);
 if(args.size()==2&&args[0]=="--synthetic-descendant"){
  const QDir root(args[1]);if(!root.isAbsolute()||!root.exists()||!write(root.filePath("descendant-ready"),"synthetic marker-only descendant"))return 95;
  QElapsedTimer deadline;deadline.start();while(!QFileInfo::exists(root.filePath("parent-revoked"))&&deadline.elapsed()<10000)QThread::msleep(25);
  return QFileInfo::exists(root.filePath("parent-revoked"))&&write(root.filePath("descendant-after-revoke"),"synthetic descendant survived parent revocation")?0:94;
 }
 if(args==QStringList{"devices","-l"}||args.contains("--version")||args.contains("-V")||args.value(0)=="--stdout"||args.value(0)=="-i")return child(args);
 OfflineTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "offline_tests.moc"
