#include "TskImage.hpp"
#include "RecoveryBridge.hpp"
#include "ToolBridge.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdio>
#include <thread>
namespace {
QByteArray read(const QString &path){QFile file(path);return file.open(QIODevice::ReadOnly)?file.readAll():QByteArray();}
bool write(const QString &path,const QByteArray &bytes){QFile file(path);return file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size();}
QByteArray image(){QByteArray bytes(4*1024*1024,'\0');bytes[510]=char(0x55);bytes[511]=char(0xaa);bytes[450]=char(0x83);bytes[455]=char(0x08);bytes[459]=char(0x08);return bytes;}
int fakeTool(const QStringList &arguments){
 if(arguments==QStringList{"-V"}){std::fputs("The Sleuth Kit ver 4.12.1\n",stdout);return 0;}
 if(arguments.size()!=3||arguments[0]!="-i"||arguments[1]!="raw")return 99;
 const auto path=arguments[2];const auto mode=qEnvironmentVariable("MR_TSK_TEST_MODE");
 if(mode=="hang")QThread::msleep(5000);
 if(mode=="failure")return 5;
 if(mode=="flood"){const QByteArray flood(600*1024,'X');std::fwrite(flood.constData(),1,static_cast<size_t>(flood.size()),stdout);return 0;}
 if(mode=="modify") {QFile::setPermissions(path,QFileDevice::ReadOwner|QFileDevice::WriteOwner);write(path,"Modified working copy");}
 if(mode=="original")write(qEnvironmentVariable("MR_TSK_SYNTHETIC_SOURCE"),"Modified synthetic original");
 const auto bytes=read(path);if(bytes.isEmpty())return 7;
 std::fputs("DOS Partition Table\nUnits are in 512-byte sectors\n000: Meta 0000000000 0000000000 0000000001 Primary Table (#0)\n001: 000:000 0000002048 0000004095 0000002048 Linux (0x83)\n",stdout);return 0;
}
}
class TskTests final:public QObject {
 Q_OBJECT
 QTemporaryDir temp;
 QString executable,source;
private slots:
 void initTestCase(){
  QVERIFY(temp.isValid());
#ifdef Q_OS_WIN
  executable=temp.filePath("fixture-mmls.exe");
#else
  executable=temp.filePath("fixture-mmls");
#endif
  QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),executable));QVERIFY(QFile::setPermissions(executable,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));source=temp.filePath("synthetic.img");
 }
 void init(){qunsetenv("MR_TSK_TEST_MODE");qunsetenv("MR_TSK_SYNTHETIC_SOURCE");QVERIFY(write(source,image()));}
 void cleanup(){qunsetenv("MR_TSK_TEST_MODE");qunsetenv("MR_TSK_SYNTHETIC_SOURCE");}
 void privateCopyHashAndProvenance(){
  const auto original=read(source);qint64 last=0,total=0;auto token=std::make_shared<std::atomic_bool>(false);
  const auto result=mr::forensic::analyzePartitions(source,executable,token,[&](qint64 d,qint64 t){QVERIFY(d>=last);last=d;total=t;});
  QVERIFY2(result.error.isEmpty(),qPrintable(result.error));QVERIFY(result.sourceUnchanged);QVERIFY(!result.cancelled);QCOMPARE(read(source),original);
  QCOMPARE(result.sourceSha256,QCryptographicHash::hash(original,QCryptographicHash::Sha256));QCOMPARE(result.toolSha256,QCryptographicHash::hash(read(executable),QCryptographicHash::Sha256));QCOMPARE(last,total);QVERIFY(result.output.contains("Linux (0x83)"));
 }
 void failureCancellationBoundsAndChanges(){
  auto token=std::make_shared<std::atomic_bool>(false);mr::forensic::TskLimits limits;limits.processTimeoutMs=150;
  for(const auto &mode:QStringList{"failure","flood","hang","modify","original"}){
   QVERIFY(write(source,image()));qputenv("MR_TSK_TEST_MODE",mode.toUtf8());qputenv("MR_TSK_SYNTHETIC_SOURCE",source.toUtf8());
   const auto result=mr::forensic::analyzePartitions(source,executable,token,{},limits);QVERIFY2(!result.error.isEmpty(),qPrintable(mode));QVERIFY(!result.sourceUnchanged);
   if(mode!="original")QCOMPARE(read(source),image());
  }
  qunsetenv("MR_TSK_TEST_MODE");token->store(true);const auto preCancelled=mr::forensic::analyzePartitions(source,executable,token);QVERIFY(preCancelled.cancelled);QVERIFY(!preCancelled.sourceUnchanged);
  QVERIFY(write(source,image()));token->store(false);qputenv("MR_TSK_TEST_MODE","hang");
  std::jthread cancellation([token]{QThread::msleep(100);token->store(true);});
  const auto cancelled=mr::forensic::analyzePartitions(source,executable,token);QVERIFY(cancelled.cancelled);QVERIFY(!cancelled.sourceUnchanged);
 }
 void toolSelectionDoesNotExecuteAndPersists(){
  ToolBridge bridge;const auto config=temp.filePath("forensic-tools.json");QVERIFY(bridge.loadToolConfiguration(config));
  QCOMPARE(bridge.tools().size(),3);QCOMPARE(bridge.forensicTools().size(),10);qputenv("MR_TSK_TEST_MODE","hang");
  QVERIFY(bridge.configureTool("mmls",executable));QVERIFY(!bridge.busy());QVERIFY(bridge.forensicTools()[0].toMap().value("configured").toBool());
  ToolBridge reopened;QVERIFY(reopened.loadToolConfiguration(config));QCOMPARE(reopened.forensicTools()[0].toMap().value("configuredPath").toString(),executable);QVERIFY(!reopened.busy());
  QVERIFY(!bridge.configureTool("photorec",executable));QVERIFY(bridge.clearTool("mmls"));QVERIFY(!bridge.forensicTools()[0].toMap().value("configured").toBool());
 }
 void inputPolicy(){
  auto token=std::make_shared<std::atomic_bool>(false);
  for(const auto &path:QStringList{"https://example.invalid/evidence.img","//host/evidence.img",temp.path(),temp.filePath("absent.img")})QVERIFY(!mr::forensic::analyzePartitions(path,executable,token).error.isEmpty());
  QVERIFY(!mr::forensic::analyzePartitions(source,"mmls",token).error.isEmpty());
  const auto tiny=temp.filePath("tiny.img");QVERIFY(write(tiny,"small"));QVERIFY(!mr::forensic::analyzePartitions(tiny,executable,token).error.isEmpty());
 }
 void caseAdapterCommitsOnlyVerifiedResults(){
  RecoveryBridge bridge(temp.filePath("cases"),true);QVERIFY(bridge.createCase("Synthetic TSK","LAB-ONLY","Lab","No phone","","backup","unknown",true));
  QVERIFY(!bridge.analyzePartitions(source,executable,true));QVERIFY(bridge.recordOperator("Synthetic operator","Fixture authorization",true));QVERIFY(!bridge.analyzePartitions(source,executable,false));
  QVERIFY(bridge.analyzePartitions(source,executable,true));QTRY_VERIFY(!bridge.busy());QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));QCOMPARE(bridge.evidence().size(),1);QCOMPARE(bridge.evidence()[0].toMap().value("kind").toString(),QString("external-analysis"));
  const auto report=QJsonDocument::fromJson(bridge.previewEvidence(0).toUtf8()).object();QVERIFY(report.value("originalUnchanged").toBool());QVERIFY(report.value("output").toString().contains("Linux (0x83)"));
  qputenv("MR_TSK_TEST_MODE","failure");QVERIFY(bridge.analyzePartitions(source,executable,true));QTRY_VERIFY(!bridge.busy());QVERIFY(!bridge.error().isEmpty());QCOMPARE(bridge.evidence().size(),1);
 }
 void realInstalledMmlsIntegration(){
  const auto real=qEnvironmentVariable("MR_TSK_MMLS_PATH");if(real.isEmpty())QSKIP("Set MR_TSK_MMLS_PATH to exercise a real separately installed TSK binary.");
  const auto original=read(source);auto token=std::make_shared<std::atomic_bool>(false);const auto result=mr::forensic::analyzePartitions(source,real,token);
  QVERIFY2(result.error.isEmpty(),qPrintable(result.error));QVERIFY(result.sourceUnchanged);QCOMPARE(read(source),original);QVERIFY(result.output.contains("DOS Partition Table"));QVERIFY(result.output.contains("Linux (0x83)"));
  qInfo("Real installed mmls integration succeeded: %s",qPrintable(result.version));
 }
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);const auto args=app.arguments().mid(1);if(args==QStringList{"-V"}||args.value(0)=="-i")return fakeTool(args);TskTests test;return QTest::qExec(&test,argc,argv);}
#include "tsk_tests.moc"
