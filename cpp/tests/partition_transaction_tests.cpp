#include "CaseOperationIntent.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "RecoveryBridge.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QThread>
#include <QtTest>
#include <cstdio>
#include <cstdlib>
namespace {
const QString password="Synthetic partition transaction 2026!";
QString sha(const QByteArray &data){return QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex());}
struct Fixture {
 QTemporaryDir temporary;
 QString root;
 IdentityService identity;
 GovernmentOperationPolicy policy;
 LawCaseBridge law;
 QString id,evidence,request;
 Fixture(const QString &path={},int lifetime=600000):root(path.isEmpty()?temporary.path():path),identity(root+"/identity",true),policy(&identity,nullptr,lifetime),law(root+"/cases.db",nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity){
  if(!identity.bootstrap("admin","Synthetic admin",password)||!identity.login("admin",password)||identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty()||identity.createUser("reviewer","Synthetic reviewer","reviewer",password).isEmpty()||!identity.login("investigator",password))qFatal("Identity fixture failed");
  law.setOperationPolicy(&policy);
  id=law.createCase({{"caseNumber","Synthetic partition"},{"investigatingAgency","Synthetic agency"},{"investigatorName","Synthetic investigator"},{"seizedDeviceDescription","Fixture image"},{"authorityReference","Synthetic authority"},{"issuingAuthority","Fixture owner"},{"scope","Synthetic images only"},{"jurisdiction","Test"}});
  const auto device=law.addDevice({{"model","Synthetic raw image"},{"custodian","Synthetic custodian"}});
  QFile source(root+"/source.img");QByteArray data(1024*1024,'\0');data.replace(510,2,QByteArray::fromHex("55aa"));
  // One independent MBR Linux partition, sectors 1..1023.
  data.replace(450,1,QByteArray::fromHex("83"));data[454]=1;data.replace(458,2,QByteArray::fromHex("ff03"));
  if(!source.open(QIODevice::WriteOnly)||source.write(data)!=data.size())qFatal("Source fixture failed");
  source.close();
  evidence=law.addEvidence(device,source.fileName(),"Synthetic MBR fixture","Synthetic custodian",true);
  if(id.isEmpty()||evidence.isEmpty())qFatal("Case fixture failed");
 }
 bool approve(const QString &caseRef){
  request=policy.requestException("external-analysis","Synthetic registered-evidence test",caseRef,60,true);
  return !request.isEmpty()&&identity.login("reviewer",password)&&policy.reviewException(request,"Synthetic independent review")&&policy.decideException(request,true,"Synthetic independent approval")&&identity.login("investigator",password)&&policy.activateException(request)&&law.openCase(id);
 }
 QVariantMap prepare(LawCaseStore &store){
  const auto operation=policy.beginCasePartitions(id);
  return store.preparePartitions(id,3,evidence,operation,sha("synthetic executable"));
 }
 QVariantMap report(const QVariantMap &intent){return {{"sourceSha256",intent.value("sourceSha256")},{"toolSha256",intent.value("toolSha256")},{"output","Synthetic partitions"}};}
};
int crashFixture(const QString &root,const QString &stage){
 Fixture f(root);if(!f.approve(f.id))return 1;LawCaseStore store(root+"/cases.db",true);auto intent=f.prepare(store);
 if(intent.isEmpty())return 2;
 if(stage=="before-prepare")std::_Exit(86);
 if(stage=="during-prepare")f.policy.setSyntheticAuditWriteCheckpoint([]{std::_Exit(86);});
 if(!f.policy.recordCaseOperation(intent))return 3;
 if(stage=="after-prepare")std::_Exit(86);
 intent=store.partitionCandidate(intent,f.report(intent));
 if(stage=="during-candidate")f.policy.setSyntheticAuditWriteCheckpoint([]{std::_Exit(86);});
 if(!f.policy.recordCaseOperation(intent))return 4;
 if(stage=="after-candidate")std::_Exit(86);
 if(stage=="during-case")store.setSyntheticWriteCheckpoint([]{std::_Exit(86);});
 if(store.recordAnalysis(f.id,3,f.evidence,"partition-analysis",f.report(intent),f.identity.actor(),intent).isEmpty())return 5;
 if(stage=="after-case")std::_Exit(86);
 intent.insert("recoveryState",store.reconcileAnalysis(intent));
 if(stage=="during-terminal")f.policy.setSyntheticAuditWriteCheckpoint([]{std::_Exit(86);});
 if(!f.policy.recordCaseOperation(intent))return 6;
 std::_Exit(86);
}
}
class PartitionTransactionTests:public QObject {
 Q_OBJECT
private slots:
 void cleanup(){qunsetenv("MR_SYNTHETIC_PARTITION_TOOL");qunsetenv("MR_SYNTHETIC_PARTITION_MARKER");}
 void verifiedCaseAndOriginalApproval(){
  Fixture f;QVERIFY(f.approve("00000000-0000-4000-8000-000000000079"));
  QVERIFY(!f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));QVERIFY(f.policy.caseOperationStates().isEmpty());
  f.policy.revoke("external-analysis");QVERIFY(f.approve(f.id));
  QVERIFY(!f.law.analyzePartitions(QUuid::createUuid().toString(QUuid::WithoutBraces),QCoreApplication::applicationFilePath(),true));
  QVERIFY(f.policy.caseOperationStates().isEmpty());
  QVERIFY(!f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),false));
  const auto marker=f.root+"/positive-dispatch-marker";qputenv("MR_SYNTHETIC_PARTITION_MARKER",marker.toUtf8());
  QVERIFY(f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));
  QTRY_VERIFY_WITH_TIMEOUT(!f.law.partitionsBusy(),20000);
  QVERIFY(QFile::exists(marker));
  QVERIFY2(!f.law.partitionsResult().isEmpty(),qPrintable(f.law.error()));
  const auto states=f.policy.caseOperationStates();QCOMPARE(states.size(),1);const auto intent=states.first().toMap();
  QCOMPARE(intent.value("recoveryState").toString(),QString("completed"));QCOMPARE(intent.value("requestId").toString(),f.request);QCOMPARE(intent.value("caseId").toString(),f.id);
  QCOMPARE(f.law.activeCase().value("revision").toInt(),4);QCOMPARE(f.law.activeCase().value("analyses").toList().size(),1);
  QCOMPARE(f.law.partitionsResult().value("operationId"),intent.value("operationId"));
  const auto receipt=f.policy.operationReceipts().first().toMap();QCOMPARE(receipt.value("status").toString(),QString("completed"));QCOMPARE(receipt.value("outputSha256"),intent.value("reportSha256"));
  const auto anchor=f.policy.auditStatus();f.law.setOperationPolicy(&f.policy);QCOMPARE(f.policy.auditStatus(),anchor);
 }
 void candidateBindingAndNoReplay(){
  Fixture f;QVERIFY(f.approve(f.id));LawCaseStore store(f.root+"/cases.db",true);auto intent=f.prepare(store);QVERIFY(!intent.isEmpty());QVERIFY(f.policy.recordCaseOperation(intent));
  QVERIFY(store.analysisSnapshotCurrent(intent));
  QCOMPARE(store.reconcileAnalysis(intent),QString("unknown"));
  auto forged=intent;forged.insert("recoveryState","completed");QVERIFY(!f.policy.recordCaseOperation(forged));
  QVERIFY(store.recordAnalysis(f.id,3,f.evidence,"partition-analysis",f.report(intent),f.identity.actor(),intent).isEmpty());
  auto report=f.report(intent);report.insert("toolSha256",QString(64,'0'));QVERIFY(store.partitionCandidate(intent,report).isEmpty());
  auto candidate=store.partitionCandidate(intent,f.report(intent));QVERIFY(f.policy.recordCaseOperation(candidate));
  QCOMPARE(store.reconcileAnalysis(candidate),QString("failed"));
  forged=candidate;forged.insert("evidenceId",QUuid::createUuid().toString(QUuid::WithoutBraces));QVERIFY(!f.policy.recordCaseOperation(forged));
  QVERIFY(!store.recordAnalysis(f.id,3,f.evidence,"partition-analysis",f.report(intent),f.identity.actor(),candidate).isEmpty());
  QCOMPARE(store.reconcileAnalysis(candidate),QString("completed"));
  QVERIFY(store.recordAnalysis(f.id,3,f.evidence,"partition-analysis",f.report(intent),f.identity.actor(),candidate).isEmpty());
  candidate.insert("recoveryState","completed");QVERIFY(f.policy.recordCaseOperation(candidate));const auto anchor=f.policy.auditStatus();QVERIFY(f.policy.recordCaseOperation(candidate));QCOMPARE(f.policy.auditStatus(),anchor);
 }
 void authorityEnds_data(){QTest::addColumn<QString>("reason");for(const auto &name:{"cancel","logout","revoke","case-close","revision","expiry"})QTest::newRow(name)<<QString::fromLatin1(name);}
 void authorityEnds(){
  QFETCH(QString,reason);Fixture f({},reason=="expiry"?1500:600000);QVERIFY(f.approve(f.id));qputenv("MR_SYNTHETIC_PARTITION_TOOL","wait");
  QVERIFY(f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));
  if(reason=="cancel")f.law.cancelPartitions();
  else if(reason=="logout")f.identity.logout();
  else if(reason=="revoke")f.policy.revoke("external-analysis");
  else if(reason=="case-close")f.law.closeWorkspace();
  else if(reason=="revision"){LawCaseStore store(f.root+"/cases.db",true);QVERIFY(store.update(f.id,3,{{"caseNumber","Synthetic concurrent edit"}},f.identity.actor()));}
  else {QTest::qWait(2000);}
  QTRY_VERIFY_WITH_TIMEOUT(!f.law.partitionsBusy(),20000);
  QVERIFY(f.law.partitionsResult().isEmpty());LawCaseStore store(f.root+"/cases.db",true);QVERIFY(store.get(f.id).value("analyses").toList().isEmpty());
  const auto states=f.policy.caseOperationStates();QCOMPARE(states.size(),1);QVERIFY(states.first().toMap().value("recoveryState")!="completed");
 }
 void changedCaseAfterPreparePreventsDispatch(){
  Fixture f;QVERIFY(f.approve(f.id));bool changed=false;
  const auto marker=f.root+"/dispatch-marker";qputenv("MR_SYNTHETIC_PARTITION_MARKER",marker.toUtf8());
  connect(&f.policy,&GovernmentOperationPolicy::auditChanged,&f.law,[&]{
   if(changed)return;
   const auto states=f.policy.caseOperationStates();
   if(!states.isEmpty() && states.first().toMap().value("recoveryState")=="prepared"){
    changed=true;LawCaseStore store(f.root+"/cases.db",true);if(!store.update(f.id,3,{{"caseNumber","Concurrent synthetic edit"}},f.identity.actor()))qFatal("Concurrent fixture failed");
   }
  });
  QVERIFY(!f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));QVERIFY(changed);QVERIFY(!QFile::exists(marker));QVERIFY(!f.law.partitionsBusy());QVERIFY(f.law.partitionsResult().isEmpty());
  LawCaseStore store(f.root+"/cases.db",true);QCOMPARE(store.get(f.id).value("revision").toInt(),4);QVERIFY(store.get(f.id).value("analyses").toList().isEmpty());
 }
 void revocationAfterCandidatePreventsCaseWrite(){
  Fixture f;QVERIFY(f.approve(f.id));bool revoked=false;
  connect(&f.policy,&GovernmentOperationPolicy::auditChanged,&f.law,[&]{
   if(revoked)return;
   const auto states=f.policy.caseOperationStates();
   if(!states.isEmpty() && states.first().toMap().value("recoveryState")=="candidate"){revoked=true;f.policy.revoke("external-analysis");}
  });
  QVERIFY(f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));QTRY_VERIFY_WITH_TIMEOUT(!f.law.partitionsBusy(),20000);
  QVERIFY(revoked);QVERIFY(f.law.partitionsResult().isEmpty());QVERIFY(f.law.activeCase().value("analyses").toList().isEmpty());QCOMPARE(f.policy.operationReceipts().first().toMap().value("status").toString(),QString("revoked"));
 }
 void logoutAfterReceiptWithholdsVisibleSuccess(){
  Fixture f;QVERIFY(f.approve(f.id));bool loggedOut=false;QSignalSpy completed(&f.law,&LawCaseBridge::partitionsCompleted);
  connect(&f.policy,&GovernmentOperationPolicy::auditChanged,&f.law,[&]{
   if(loggedOut)return;
   const auto receipts=f.policy.operationReceipts();
   if(!receipts.isEmpty() && receipts.first().toMap().value("status")=="completed"){loggedOut=true;f.identity.logout();}
  });
  QVERIFY(f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));QTRY_VERIFY_WITH_TIMEOUT(!f.law.partitionsBusy(),20000);
  QVERIFY(loggedOut);QCOMPARE(completed.size(),1);QCOMPARE(completed.first().first().toBool(),false);QVERIFY(f.law.partitionsResult().isEmpty());
  LawCaseStore store(f.root+"/cases.db",true);QCOMPARE(store.get(f.id).value("analyses").toList().size(),1);
 }
 void newApprovalCannotAuthorizeOldOperation(){
  Fixture f;QVERIFY(f.approve(f.id));LawCaseStore store(f.root+"/cases.db",true);auto intent=f.prepare(store);QVERIFY(f.policy.recordCaseOperation(intent));
  const auto old=intent.value("operationId").toString();f.policy.revoke("external-analysis");
  intent.insert("recoveryState","failed");QVERIFY(f.policy.recordCaseOperation(intent));
  QVERIFY(f.approve(f.id));QVERIFY(!f.policy.caseOperationAuthorized(old));
  const auto operation=f.policy.beginCasePartitions(f.id);QVERIFY(!operation.isEmpty());QVERIFY(f.policy.caseOperationAuthorized(operation.value("operationId").toString()));
  auto rebound=intent;rebound.insert("recoveryState","candidate");rebound.insert("reportSha256",QString(64,'a'));QVERIFY(!f.policy.recordCaseOperation(rebound));
 }
 void rejectedWorker_data(){QTest::addColumn<QByteArray>("mode");for(const auto &mode:{"error","oversize","wrong-version"})QTest::newRow(mode)<<QByteArray(mode);}
 void rejectedWorker(){QFETCH(QByteArray,mode);Fixture f;QVERIFY(f.approve(f.id));qputenv("MR_SYNTHETIC_PARTITION_TOOL",mode);QVERIFY(f.law.analyzePartitions(f.evidence,QCoreApplication::applicationFilePath(),true));QTRY_VERIFY_WITH_TIMEOUT(!f.law.partitionsBusy(),20000);QVERIFY(f.law.partitionsResult().isEmpty());QVERIFY(f.law.activeCase().value("analyses").toList().isEmpty());}
 void productionRecoveryLabelCannotDispatch(){QTemporaryDir dir;IdentityService identity(dir.filePath("identity"),true);GovernmentOperationPolicy policy(&identity);RecoveryBridge legacy(dir.filePath("legacy"),false);legacy.setOperationPolicy(&policy);QVERIFY(!legacy.analyzePartitions("ignored.img",QCoreApplication::applicationFilePath(),true));QVERIFY(legacy.error().contains("registered evidence"));QVERIFY(policy.operationReceipts().isEmpty());}
 void realMmls(){
  const auto tool=qEnvironmentVariable("MR_TSK_MMLS_PATH");if(tool.isEmpty())QSKIP("Separately installed mmls not configured.");
  Fixture f;QVERIFY(f.approve(f.id));QVERIFY(f.law.analyzePartitions(f.evidence,tool,true));QTRY_VERIFY_WITH_TIMEOUT(!f.law.partitionsBusy(),20000);QVERIFY2(!f.law.partitionsResult().isEmpty(),qPrintable(f.law.error()));
  const auto report=QJsonDocument::fromJson(f.law.partitionsResult().value("reportJson").toString().toUtf8()).object();QVERIFY(report.value("output").toString().contains("Linux"));
 }
 void interruptedWrites_data(){
  QTest::addColumn<QString>("stage");QTest::addColumn<QString>("outcome");QTest::addColumn<int>("analyses");
  QTest::newRow("before-prepare")<<QString("before-prepare")<<QString()<<0;
  QTest::newRow("during-prepare")<<QString("during-prepare")<<QString()<<0;
  QTest::newRow("after-prepare")<<QString("after-prepare")<<QString("unknown")<<0;
  QTest::newRow("during-candidate")<<QString("during-candidate")<<QString("unknown")<<0;
  QTest::newRow("after-candidate")<<QString("after-candidate")<<QString("failed")<<0;
  QTest::newRow("during-case")<<QString("during-case")<<QString("failed")<<0;
  QTest::newRow("after-case")<<QString("after-case")<<QString("completed")<<1;
  QTest::newRow("during-terminal")<<QString("during-terminal")<<QString("completed")<<1;
  QTest::newRow("after-terminal")<<QString("after-terminal")<<QString("completed")<<1;
 }
 void interruptedWrites(){
  QFETCH(QString,stage);QFETCH(QString,outcome);QFETCH(int,analyses);QTemporaryDir dir;QProcess child;child.start(QCoreApplication::applicationFilePath(),{"--case-crash",dir.path(),stage});QVERIFY(child.waitForFinished(20000));QCOMPARE(child.exitCode(),86);
  IdentityService identity(dir.filePath("identity"),true);GovernmentOperationPolicy policy(&identity);LawCaseBridge law(dir.filePath("cases.db"),nullptr,true);law.setOperationPolicy(&policy);QCOMPARE(law.ready(),outcome!="unknown");
  const auto states=policy.caseOperationStates();if(outcome.isEmpty())QVERIFY(states.isEmpty());else {QCOMPARE(states.size(),1);QCOMPARE(states.first().toMap().value("recoveryState").toString(),outcome);}
  const auto anchor=policy.auditStatus();law.setOperationPolicy(&policy);QCOMPARE(policy.auditStatus(),anchor);
  IdentityService again(dir.filePath("identity"),true);GovernmentOperationPolicy restarted(&again);LawCaseBridge second(dir.filePath("cases.db"),nullptr,true);second.setOperationPolicy(&restarted);QCOMPARE(restarted.auditStatus(),anchor);
  LawCaseStore store(dir.filePath("cases.db"),true);const auto record=store.list().first().toMap();QCOMPARE(record.value("analyses").toList().size(),analyses);QCOMPARE(record.value("revision").toInt(),3+analyses);QVERIFY(store.verifyAudit(record.value("id").toString()));
 }
};
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);const auto args=app.arguments();
 if((args.size()==2 && args[1]=="-V") || (args.size()==4 && args[1]=="-i")){
  const auto marker=qEnvironmentVariable("MR_SYNTHETIC_PARTITION_MARKER");
  if(!marker.isEmpty()){QFile file(marker);if(!file.open(QIODevice::WriteOnly)||file.write("dispatched")!=10)return 9;}
 }
 if(args.size()==2 && args[1]=="-V"){
  const auto mode=qgetenv("MR_SYNTHETIC_PARTITION_TOOL");if(mode=="wrong-version")std::puts("Unsupported synthetic tool");else std::puts("The Sleuth Kit ver synthetic-fixture");return 0;
 }
 if(args.size()==4 && args[1]=="-i"){
  const auto mode=qgetenv("MR_SYNTHETIC_PARTITION_TOOL");if(mode=="wait")QThread::msleep(4000);if(mode=="error")return 3;
  if(mode=="oversize"){const QByteArray bytes(9000,'x');std::fwrite(bytes.constData(),1,size_t(bytes.size()),stdout);}else std::puts("Synthetic partition listing");return 0;
 }
 if(args.size()==4 && args[1]=="--case-crash")return crashFixture(args[2],args[3]);
 PartitionTransactionTests test;return QTest::qExec(&test,argc,argv);
}
#include "partition_transaction_tests.moc"
