#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QCoreApplication>
#include <QThread>
#include <QTextStream>
#include <QLockFile>
#include "SecurityBridge.hpp"
namespace {
void write(const QString &path,const QByteArray &bytes){QFile f(path);if(f.open(QIODevice::WriteOnly))f.write(bytes);}
QByteArray read(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
}
class SecurityTests:public QObject {
 Q_OBJECT
private slots:
 void cleanup(){qunsetenv("MR_TEST_SECURITY_VERSION_DELAY");qunsetenv("MR_TEST_UPDATE_MODE");}
 void adapterAndIntegrity(){
  QTemporaryDir t;const auto source=t.filePath("synthetic.apk"),db=t.filePath("db");QVERIFY(QDir().mkpath(db));write(db+"/test.ndb","NexvarySynthetic:0:*:4e45585641525953594e544845544943\n");write(source,"NEXVARYSYNTHETIC\n");const auto original=read(source);const auto token=std::make_shared<std::atomic_bool>(false);
  auto r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token);QVERIFY2(r.value("error").toString().isEmpty(),qPrintable(r.value("error").toString()));QCOMPARE(r.value("exitCode").toInt(),1);QVERIFY(r.value("sourceUnchanged").toBool());QCOMPARE(read(source),original);QVERIFY(r.value("databaseScope").toString().contains("Custom"));
  write(source,"ordinary synthetic sample\n");r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token);QVERIFY(r.value("error").toString().isEmpty());QVERIFY(r.value("verdict").toString().contains("custom"));
  write(source,"FAKE-ERROR");r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token);QVERIFY(!r.value("error").toString().isEmpty());QVERIFY(!r.value("verdict").toString().contains("clean"));
  write(source,"FAKE-TAMPER");r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token);QVERIFY(r.value("error").toString().contains("changed"));QCOMPARE(read(source),QByteArray("FAKE-TAMPER"));
 }
 void cancellationAndBounds(){
  QTemporaryDir t;const auto source=t.filePath("test.bin"),db=t.filePath("db");QDir().mkpath(db);write(db+"/test.ndb","N:0:*:41414141\n");write(source,"FAKE-SLEEP");auto token=std::make_shared<std::atomic_bool>(true);auto r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token);QVERIFY(!r.value("error").toString().isEmpty());token->store(false);SecurityLimits limits;limits.timeoutMs=100;r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token,limits);QVERIFY2(r.value("error").toString().contains("timed out"),qPrintable(r.value("error").toString()));
  // Exercise output bounds independently of the short timeout assertion.
  write(source,"FAKE-LOUD");limits.timeoutMs=5000;limits.maximumOutput=2048;r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token,limits);QVERIFY2(r.value("error").toString().contains("output limit"),qPrintable(r.value("error").toString()));
  limits.maximumFile=1;r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,token,limits);QVERIFY(!r.value("error").toString().isEmpty());
 }
 void versionDeadlineIsReported(){
  QTemporaryDir t;const auto source=t.filePath("test.bin"),db=t.filePath("db");QVERIFY(QDir().mkpath(db));write(db+"/test.ndb","N:0:*:41414141\n");write(source,"ordinary synthetic sample");
  qputenv("MR_TEST_SECURITY_VERSION_DELAY","1");SecurityLimits limits;limits.versionTimeoutMs=100;limits.timeoutMs=5000;
  const auto r=scanSecurityFile(source,QCoreApplication::applicationFilePath(),db,std::make_shared<std::atomic_bool>(false),limits);
  QVERIFY2(r.value("error").toString().contains("version check")&&r.value("error").toString().contains("timed out"),qPrintable(r.value("error").toString()));QVERIFY(!r.contains("output"));QCOMPARE(r.value("verdict").toString(),QStringLiteral("Unavailable / incomplete"));
 }
 void bridgeConsentAndExport(){
  QTemporaryDir t;const auto source=t.filePath("test.bin"),db=t.filePath("db");QDir().mkpath(db);write(db+"/test.ndb","N:0:*:41414141\n");write(source,"ordinary sample");SecurityBridge s(t.filePath("scanner.json"));QVERIFY(s.configure(QCoreApplication::applicationFilePath(),db));QVERIFY(!s.scan(source,false));QVERIFY(s.scan(source,true));QTRY_VERIFY_WITH_TIMEOUT(!s.busy(),5000);QVERIFY2(s.error().isEmpty(),qPrintable(s.error()));QVERIFY(!s.exportReport(t.filePath("result.json"),false));QVERIFY(s.exportReport(t.filePath("result.json"),true));QVERIFY(!s.exportReport(t.filePath("result.json"),true));QCOMPARE(read(source),QByteArray("ordinary sample"));
 }
 void databaseUpdaterOutcomes_data(){
  QTest::addColumn<QByteArray>("mode");QTest::addColumn<QString>("expected");
  QTest::newRow("success")<<QByteArray()<<QString();
  QTest::newRow("network-failure")<<QByteArray("failure")<<QString("update failed");
  QTest::newRow("empty-result")<<QByteArray("empty")<<QString("complete readable");
  QTest::newRow("timeout")<<QByteArray("sleep")<<QString("timed out");
  QTest::newRow("output-bound")<<QByteArray("loud")<<QString("output limit");
 }
 void databaseUpdaterOutcomes(){
  QFETCH(QByteArray,mode);QFETCH(QString,expected);QTemporaryDir t;
  qputenv("MR_TEST_UPDATE_MODE",mode);
  const auto result=updateSecurityDatabase(QCoreApplication::applicationFilePath(),t.path(),std::make_shared<std::atomic_bool>(false),mode=="sleep"?100:5000);
  QCOMPARE(result.value("reportType").toString(),QStringLiteral("nexvary-signature-update"));
  QVERIFY(!result.contains("verdict"));
  if(expected.isEmpty()){QVERIFY2(result.value("success").toBool(),qPrintable(result.value("error").toString()));QCOMPARE(result.value("signatureFiles").toList().size(),2);}
  else {QVERIFY(!result.value("success").toBool());QVERIFY2(result.value("error").toString().contains(expected),qPrintable(result.value("error").toString()));}
 }
 void databaseLockAndPreCancellation(){
  QTemporaryDir t;const auto token=std::make_shared<std::atomic_bool>(true);
  auto result=updateSecurityDatabase(QCoreApplication::applicationFilePath(),t.path(),token);
  QVERIFY(!result.value("success").toBool());QVERIFY(!QFileInfo::exists(t.filePath("main.cvd")));
  token->store(false);QLockFile lock(t.filePath(".nexvary-security.lock"));QVERIFY(lock.tryLock());
  result=updateSecurityDatabase(QCoreApplication::applicationFilePath(),t.path(),token);QVERIFY(result.value("error").toString().contains("busy"));
  write(t.filePath("file.bin"),"synthetic");write(t.filePath("test.ndb"),"N:0:*:41414141\n");
  result=scanSecurityFile(t.filePath("file.bin"),QCoreApplication::applicationFilePath(),t.path(),token);QVERIFY(result.value("error").toString().contains("busy"));
 }
 void bridgeUpdateConsentAndScanExclusion(){
  QTemporaryDir t;const auto db=t.filePath("db with spaces");QVERIFY(QDir().mkpath(db));
#ifdef Q_OS_WIN
  const auto engine=t.filePath("clamscan.exe"),updater=t.filePath("freshclam.exe");
#else
  const auto engine=t.filePath("clamscan"),updater=t.filePath("freshclam");
#endif
  for(const auto &path:{engine,updater}){QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),path));QVERIFY(QFile::setPermissions(path,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));}
  SecurityBridge bridge(t.filePath("config.json"));QVERIFY(bridge.configure(engine,db));QCOMPARE(bridge.updater(),updater);
  QVERIFY(!bridge.updateDatabase(false));QVERIFY(bridge.updateDatabase(true));QVERIFY(bridge.updating());
  QVERIFY(!bridge.scan(t.filePath("customer.apk"),true));QVERIFY(!bridge.configure(engine,db));QVERIFY(!bridge.updateDatabase(true));
  QTRY_VERIFY_WITH_TIMEOUT(!bridge.busy(),10000);QVERIFY(!bridge.updating());
  QVERIFY2(bridge.updateResult().value("success").toBool(),qPrintable(bridge.error()));QVERIFY(bridge.result().isEmpty());
  QVERIFY(bridge.configure(engine,db));QVERIFY(bridge.updateResult().isEmpty());
  qputenv("MR_TEST_UPDATE_MODE","sleep");QVERIFY(bridge.updateDatabase(true));bridge.cancel();
  QTRY_VERIFY_WITH_TIMEOUT(!bridge.busy(),10000);QVERIFY(!bridge.updateResult().value("success").toBool());QVERIFY(bridge.result().isEmpty());
 }
 void realClamAvWithSyntheticSignature(){
  const auto engine=qEnvironmentVariable("MR_CLAMSCAN_PATH");if(engine.isEmpty())QSKIP("Separately installed ClamAV not configured; required in Linux CI.");
  QTemporaryDir t;const auto source=t.filePath("sample.bin"),db=t.filePath("db");QDir().mkpath(db);write(db+"/nexvary-test.ndb","Nexvary.Synthetic.Fixture:0:*:4e45585641525953594e544845544943\n");write(source,"NEXVARYSYNTHETIC\n");const auto before=read(source);const auto cancel=std::make_shared<std::atomic_bool>(false);auto r=scanSecurityFile(source,engine,db,cancel);QVERIFY2(r.value("error").toString().isEmpty(),qPrintable(r.value("error").toString()+r.value("output").toString()));QCOMPARE(r.value("exitCode").toInt(),1);QVERIFY(r.value("output").toString().contains("Nexvary.Synthetic.Fixture"));QCOMPARE(read(source),before);
  write(source,"Benign synthetic control fixture\n");r=scanSecurityFile(source,engine,db,cancel);QVERIFY2(r.value("error").toString().isEmpty(),qPrintable(r.value("error").toString()+r.value("output").toString()));QCOMPARE(r.value("exitCode").toInt(),0);QVERIFY(r.value("verdict").toString().contains("custom"));
 }
};
int main(int argc,char **argv){
 QStringList args;for(int i=1;i<argc;++i)args.append(QString::fromLocal8Bit(argv[i]));
 if(args.contains("--version")){if(qEnvironmentVariableIsSet("MR_TEST_SECURITY_VERSION_DELAY"))QThread::msleep(1000);QTextStream(stdout)<<"ClamAV synthetic test adapter\n";return 0;}
 if(args.filter("--datadir=").size()==1){
  const auto db=args.filter("--datadir=").first().mid(10);const auto configs=args.filter("--config-file=");if(configs.size()!=1)return 90;
  const auto config=read(configs.first().mid(14));
  if(!config.contains("DatabaseMirror database.clamav.net\n")||!config.contains("TestDatabases yes\n")||config.contains("OnUpdateExecute")||config.contains("PrivateMirror"))return 91;
  const auto mode=qEnvironmentVariable("MR_TEST_UPDATE_MODE");
  if(mode=="sleep")QThread::msleep(2000);
  if(mode=="failure"){QTextStream(stdout)<<"ERROR synthetic CDN failure\n";return 52;}
  if(mode=="loud"){QTextStream(stdout)<<QString(1024*1024,'x');return 0;}
  if(mode=="empty")return 0;
  write(db+"/main.cvd","SYNTHETIC NOT AN OFFICIAL DATABASE");write(db+"/daily.cvd","SYNTHETIC NOT AN OFFICIAL DATABASE");
  QTextStream(stdout)<<"Synthetic update fixture completed\n";return 0;
 }
 if(!args.isEmpty()&&args.first()=="--stdout"){
  const auto path=args.last();const auto b=read(path);
  if(b=="FAKE-SLEEP")QThread::msleep(1000);
  if(b=="FAKE-LOUD"){QTextStream(stdout)<<QString(100000,'x');return 0;}
  if(b=="FAKE-ERROR"){QTextStream(stdout)<<"ERROR synthetic failure\n";return 2;}
  if(b=="FAKE-TAMPER")write(path,"changed working copy");
  const bool alert=b.contains("NEXVARYSYNTHETIC");QTextStream(stdout)<<path<<(alert?": Synthetic.Test FOUND\n":": OK\n");return alert?1:0;
 }
 QGuiApplication app(argc,argv);SecurityTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "security_tests.moc"
