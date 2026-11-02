#include "CaseOperationIntent.hpp"
#include "CaseVault.hpp"
#include <QJsonArray>
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "PolicyAudit.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtTest>
#include <cstdlib>
namespace {
const QString password="Synthetic transaction fixture 2026!";
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
struct Fixture {
 QTemporaryDir dir;
 IdentityService identity{dir.filePath("identity"),true};
 GovernmentOperationPolicy policy{&identity};
 LawCaseStore store{dir.filePath("cases.db"),true};
 QString id,evidence;
 QVariantMap report,intent;
 Fixture(){
  if(!identity.bootstrap("admin","Synthetic transaction admin",password)||!identity.login("admin",password)||identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty()||!identity.login("investigator",password))qFatal("Identity fixture failed");
  id=store.create({{"caseNumber","Synthetic transaction"}},identity.actor(),identity.userId());
  const auto device=store.addDevice(id,1,{{"model","synthetic"},{"custodian","Synthetic custodian"}},identity.actor());
  const QByteArray bytes("Synthetic source bytes");QFile f(dir.filePath("source"));
  if(!f.open(QIODevice::WriteOnly)||f.write(bytes)!=bytes.size())qFatal("Source fixture failed");
  f.close();
  evidence=store.addEvidence(id,2,device,f.fileName(),"Synthetic evidence","Synthetic custodian",identity.actor());
  report={{"sourceSha256",sha(bytes)},{"fixture",true}};
  intent=store.prepareAnalysis(id,3,evidence,"classic-pcap",report,identity.userId(),"investigator");
  if(intent.isEmpty())qFatal("Preparation fixture failed");
 }
};
void rewriteCase(Fixture &f,const std::function<void(QVariantMap&)> &change){
 const QString connection="synthetic-rewrite";
 {
  auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(f.dir.filePath("cases.db"));
  if(!db.open())qFatal("Cannot open synthetic rewrite");
  QSqlQuery q(db);q.prepare("SELECT payload FROM law_cases WHERE id=?");q.addBindValue(f.id);
  if(!q.exec()||!q.next())qFatal("Cannot read synthetic rewrite");
  QString error;const auto bytes=mr::recovery::openCaseBytes(q.value(0).toByteArray(),true,error);
  auto r=QJsonDocument::fromJson(bytes).object().toVariantMap();change(r);
  auto events=r.value("audit").toList();auto last=events.takeLast().toMap();last.remove("hash");
  auto state=r;state.remove("audit");
  last.insert("stateSha256",sha(QJsonDocument(QJsonObject::fromVariantMap(state)).toJson(QJsonDocument::Compact)));
  last.insert("hash",sha(QJsonDocument(QJsonObject::fromVariantMap(last)).toJson(QJsonDocument::Compact)));
  events.append(last);r.insert("audit",events);
  const auto sealed=mr::recovery::sealCase(QJsonDocument(QJsonObject::fromVariantMap(r)).toJson(QJsonDocument::Compact),true,error);
  q.prepare("UPDATE law_cases SET payload=? WHERE id=?");q.addBindValue(sealed);q.addBindValue(f.id);
  if(!q.exec())qFatal("Cannot write synthetic rewrite");
  db.close();
 }
 QSqlDatabase::removeDatabase(connection);
}
void commitAnalysis(Fixture &f){
 if(!f.policy.recordCaseOperation(f.intent)||f.store.recordAnalysis(f.id,3,f.evidence,"classic-pcap",f.report,f.identity.actor(),f.intent).isEmpty())qFatal("Cannot commit synthetic analysis");
 auto completed=f.intent;completed.insert("recoveryState","completed");
 if(!f.policy.recordCaseOperation(completed))qFatal("Cannot finish synthetic analysis");
}
int recoverOrphanFixture(const QString &root,const QString &stage){
 IdentityService identity(root+"/identity",true);GovernmentOperationPolicy policy(&identity);
 if(stage=="before")std::_Exit(86);
 if(stage=="during")policy.setSyntheticAuditWriteCheckpoint([]{std::_Exit(86);});
 LawCaseBridge bridge(root+"/cases.db",nullptr,true);bridge.setOperationPolicy(&policy);
 if(bridge.ready())return 1;
 std::_Exit(86);
}
int crashFixture(const QString &root,const QString &stage){
 IdentityService identity(root+"/identity",true);
 if(!identity.bootstrap("admin","Synthetic crash admin",password)||!identity.login("admin",password)||identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty()||!identity.login("investigator",password))return 1;
 GovernmentOperationPolicy policy(&identity);LawCaseStore store(root+"/cases.db",true);
 const auto id=store.create({{"caseNumber","Synthetic crash"}},identity.actor(),identity.userId());
 const auto device=store.addDevice(id,1,{{"model","synthetic"},{"custodian","Custodian"}},identity.actor());
 const QByteArray bytes("Synthetic interruption source");QFile f(root+"/source");if(!f.open(QIODevice::WriteOnly)||f.write(bytes)!=bytes.size())return 2;
 f.close();
 const auto evidence=store.addEvidence(id,2,device,f.fileName(),"Synthetic evidence","Custodian",identity.actor());
 const QVariantMap report{{"sourceSha256",sha(bytes)},{"fixture",true}};
 auto intent=store.prepareAnalysis(id,3,evidence,"classic-pcap",report,identity.userId(),"investigator");
 if(intent.isEmpty())return 3;
 if(stage=="before-prepare")std::_Exit(86);
 if(stage=="during-prepare")policy.setSyntheticAuditWriteCheckpoint([]{std::_Exit(86);});
 if(!policy.recordCaseOperation(intent))return 4;
 if(stage=="after-prepare")std::_Exit(86);
 if(stage=="during-case")store.setSyntheticWriteCheckpoint([]{std::_Exit(86);});
 if(store.recordAnalysis(id,3,evidence,"classic-pcap",report,identity.actor(),intent).isEmpty())return 5;
 if(stage=="after-case")std::_Exit(86);
 intent.insert("recoveryState",store.reconcileAnalysis(intent));
 if(stage=="during-terminal")policy.setSyntheticAuditWriteCheckpoint([]{std::_Exit(86);});
 if(!policy.recordCaseOperation(intent))return 6;
 std::_Exit(86);
}
}
class CaseTransactionTests:public QObject {
 Q_OBJECT
private slots:
 void replacedSnapshotMustReadBackBeforeAcknowledgement(){
  QTemporaryDir dir;const auto path=dir.filePath("synthetic-journal");PolicyAudit journal(path,true);QVERIFY(journal.healthy());
  QFile input(path);QVERIFY(input.open(QIODevice::ReadOnly));const auto original=input.readAll();input.close();
  journal.setSyntheticPostReplaceCheckpoint([path,original]{
   QFile replacement(path);if(!replacement.open(QIODevice::WriteOnly|QIODevice::Truncate)||replacement.write(original)!=original.size()||!replacement.flush())qFatal("Synthetic old-snapshot injection failed");
  });
  QVERIFY(!journal.append("policy-rejection",{{"reason","Synthetic readback race"}},"denied"));
  QVERIFY(!journal.healthy());QVERIFY(journal.error().contains("(readback)"));QCOMPARE(journal.events().size(),0);
  journal.setSyntheticPostReplaceCheckpoint({});QVERIFY(!journal.append("policy-rejection",{{"reason","Refused writer stays closed"}},"denied"));
  PolicyAudit restarted(path,true);QVERIFY(restarted.healthy());QCOMPARE(restarted.events().size(),0);
 }
 void validSignedJournalRollbackMustBlockOrphanResult(){
  Fixture f;
  QFile journal(f.identity.policyJournalPath());QVERIFY(journal.open(QIODevice::ReadOnly));
  const auto older=journal.readAll();journal.close();
  QVERIFY(f.policy.recordCaseOperation(f.intent));
  QVERIFY(!f.store.recordAnalysis(f.id,3,f.evidence,"classic-pcap",f.report,f.identity.actor(),f.intent).isEmpty());
  auto completed=f.intent;completed.insert("recoveryState","completed");QVERIFY(f.policy.recordCaseOperation(completed));
  QVERIFY(journal.open(QIODevice::WriteOnly|QIODevice::Truncate));QCOMPARE(journal.write(older),older.size());journal.close();
  IdentityService restartedIdentity(f.dir.filePath("identity"),true);
  GovernmentOperationPolicy restartedPolicy(&restartedIdentity);
  QVERIFY(restartedPolicy.caseOperationStates().isEmpty());QVERIFY(restartedPolicy.caseOperationHistoryHealthy());
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&restartedPolicy);
  QVERIFY2(!bridge.ready(),"A correctly signed old journal must not validate an orphan committed analysis");
  QCOMPARE(f.store.get(f.id).value("analyses").toList().size(),1);
 }
 void laterLegitimateRevisionsRemainVerified(){
  Fixture f;commitAnalysis(f);
  QVERIFY(f.store.update(f.id,4,{{"caseNumber","Legitimate later metadata"}},f.identity.actor()));
  QCOMPARE(f.store.reconcileAnalysis(f.intent),QString("completed"));
  QString snapshot;QVERIFY(f.store.verifyOperationLinks(f.policy.caseOperationStates(),snapshot));
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);QVERIFY2(bridge.ready(),qPrintable(bridge.error()));
  const auto anchor=f.policy.auditStatus();bridge.setOperationPolicy(&f.policy);QCOMPARE(f.policy.auditStatus(),anchor);
 }
 void retainedResultTampering_data(){
  QTest::addColumn<QString>("field");
  for(const auto &name:{"missing","duplicate","operationId","id","evidenceId","parser","actor","sourceSha256","reportJson","requestId","requesterRole","expectedRevision"})QTest::newRow(name)<<QString(name);
 }
 void retainedResultTampering(){
  QFETCH(QString,field);Fixture f;commitAnalysis(f);
  rewriteCase(f,[&](QVariantMap &r){
   auto analyses=r.value("analyses").toList();auto a=analyses.first().toMap();
   if(field=="missing")analyses.clear();
   else if(field=="duplicate")analyses.append(a);
   else {
    if(field=="requestId"||field=="requesterRole"||field=="expectedRevision"){
     auto binding=a.value("operationBinding").toMap();binding.insert(field,field=="expectedRevision"?QVariant(2):QVariant("changed"));a.insert("operationBinding",binding);
    }else if(field=="reportJson"){
     const auto bytes=QByteArray("{\"sourceSha256\":\"changed\"}");a.insert("reportJson",QString::fromUtf8(bytes));a.insert("reportSha256",sha(bytes));
    }else a.insert(field,"changed");
    analyses[0]=a;
   }
   r.insert("analyses",analyses);
  });
  QVERIFY(f.store.verifyAudit(f.id)); // The attacker recomputed the ordinary case hashes.
  QString snapshot;QVERIFY(!f.store.verifyOperationLinks(f.policy.caseOperationStates(),snapshot));
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);QVERIFY(!bridge.ready());
  QVERIFY(f.policy.caseReconciliationBlocked());const auto anchor=f.policy.auditStatus();
  bridge.setOperationPolicy(&f.policy);QCOMPARE(f.policy.auditStatus(),anchor);
  IdentityService secondIdentity(f.dir.filePath("identity"),true);GovernmentOperationPolicy secondPolicy(&secondIdentity);
  LawCaseBridge second(f.dir.filePath("cases.db"),nullptr,true);second.setOperationPolicy(&secondPolicy);QVERIFY(!second.ready());QCOMPARE(secondPolicy.auditStatus(),anchor);
 }
 void legacyUnlinkedIsNotUpgraded(){
  Fixture f;QVERIFY(!f.store.recordAnalysis(f.id,3,f.evidence,"classic-pcap",f.report,f.identity.actor()).isEmpty());
  QString snapshot;QVERIFY(f.store.verifyOperationLinks({},snapshot));QVERIFY(f.policy.caseOperationStates().isEmpty());
  QVERIFY(!f.store.get(f.id).value("analyses").toList().first().toMap().contains("operationId"));
 }
 void runtimeGateRechecksCommittedResults(){
  Fixture f;commitAnalysis(f);LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);QVERIFY(bridge.ready());
  rewriteCase(f,[](QVariantMap &r){r.insert("analyses",QVariantList());});
  QVERIFY(!bridge.ready());QVERIFY(!bridge.searchCases());QVERIFY(f.policy.caseReconciliationBlocked());
 }
 void completedSourceBytesMustStillVerify(){
  Fixture f;commitAnalysis(f);
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","synthetic-source-replacement");db.setDatabaseName(f.dir.filePath("cases.db"));QVERIFY(db.open());
   QString error;const auto bytes=mr::recovery::sealCase("Changed protected source",true,error);
   {QSqlQuery q(db);q.prepare("UPDATE law_evidence SET payload=? WHERE id=?");q.addBindValue(bytes);q.addBindValue(f.evidence);QVERIFY(q.exec());}db.close();
  }
  QSqlDatabase::removeDatabase("synthetic-source-replacement");
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);QVERIFY(!bridge.ready());QVERIFY(f.policy.caseReconciliationBlocked());
 }
 void recoveryDecisionInterruption_data(){
  QTest::addColumn<QString>("stage");for(const auto &stage:{"before","during","after"})QTest::newRow(stage)<<QString(stage);
 }
 void recoveryDecisionInterruption(){
  QFETCH(QString,stage);Fixture f;
  QFile journal(f.identity.policyJournalPath());QVERIFY(journal.open(QIODevice::ReadOnly));const auto older=journal.readAll();journal.close();
  commitAnalysis(f);QVERIFY(journal.open(QIODevice::WriteOnly|QIODevice::Truncate));QCOMPARE(journal.write(older),older.size());journal.close();
  QProcess child;child.start(QCoreApplication::applicationFilePath(),{"--recover-orphan",f.dir.path(),stage});QVERIFY(child.waitForFinished(20000));QCOMPARE(child.exitCode(),86);
  IdentityService identity(f.dir.filePath("identity"),true);GovernmentOperationPolicy policy(&identity);
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&policy);QVERIFY(!bridge.ready());QVERIFY(policy.caseReconciliationBlocked());
  const auto anchor=policy.auditStatus();bridge.setOperationPolicy(&policy);QCOMPARE(policy.auditStatus(),anchor);
  QCOMPARE(f.store.get(f.id).value("analyses").toList().size(),1);
 }
 void authenticatedBridgeUsesSignedIntent(){
  QTemporaryDir dir;IdentityService identity(dir.filePath("identity"),true);
  QVERIFY(identity.bootstrap("admin","Synthetic admin",password));QVERIFY(identity.login("admin",password));QVERIFY(!identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty());QVERIFY(identity.login("investigator",password));
  GovernmentOperationPolicy policy(&identity);LawCaseBridge law(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);law.setOperationPolicy(&policy);
  const QVariantMap metadata{{"caseNumber","Synthetic-001"},{"investigatingAgency","Synthetic agency"},{"investigatorName","Synthetic investigator"},{"seizedDeviceDescription","Fixture"},{"authorityReference","Synthetic authority"},{"issuingAuthority","Lab owner"},{"scope","Synthetic fixtures"},{"jurisdiction","Test"}};
  const auto id=law.createCase(metadata);QVERIFY2(!id.isEmpty(),qPrintable(law.error()));
  const auto device=law.addDevice({{"model","Synthetic"},{"custodian","A"}});QVERIFY(!device.isEmpty());
  QFile input(dir.filePath("capture.pcap"));QVERIFY(input.open(QIODevice::WriteOnly));input.write(QByteArray::fromHex("d4c3b2a1020004000000000000000000ffff00000100000002000000050000000e0000000e0000000000000000000000000000000806"));input.close();
  const auto evidence=law.addEvidence(device,input.fileName(),"Synthetic PCAP","A",true);QVERIFY2(!evidence.isEmpty(),qPrintable(law.error()));
  const auto result=law.analyzeEvidence(evidence,"classic-pcap");QVERIFY2(!result.isEmpty(),qPrintable(law.error()));
  const auto states=policy.caseOperationStates();QCOMPARE(states.size(),1);const auto intent=states.first().toMap();
  QCOMPARE(intent.value("recoveryState").toString(),QString("completed"));QCOMPARE(intent.value("caseId").toString(),id);QCOMPARE(result.value("operationId"),intent.value("operationId"));QCOMPARE(law.activeCase().value("revision").toInt(),4);
  identity.logout();QVERIFY(law.analyzeEvidence(evidence,"classic-pcap").isEmpty());QCOMPARE(policy.caseOperationStates().size(),1);
 }
 void verifiedMembershipRevisionAndDigest(){
  Fixture f;QVERIFY(mr::law::caseIntent(f.intent));
  QVERIFY(f.store.prepareAnalysis(f.id,2,f.evidence,"classic-pcap",f.report,f.identity.userId(),"investigator").isEmpty());
  QVERIFY(f.store.prepareAnalysis(f.id,3,QUuid::createUuid().toString(),"classic-pcap",f.report,f.identity.userId(),"investigator").isEmpty());
  const auto other=f.store.create({{"caseNumber","Other synthetic case"}},f.identity.actor(),f.identity.userId());
  QVERIFY(f.store.prepareAnalysis(other,1,f.evidence,"classic-pcap",f.report,f.identity.userId(),"investigator").isEmpty());
  auto wrong=f.report;wrong.insert("sourceSha256",QString(64,'0'));
  QVERIFY(f.store.prepareAnalysis(f.id,3,f.evidence,"classic-pcap",wrong,f.identity.userId(),"investigator").isEmpty());
  QCOMPARE(f.store.reconcileAnalysis(f.intent),QString("failed"));
  QVERIFY(f.policy.recordCaseOperation(f.intent));
  QVERIFY(!f.policy.recordCaseOperation(f.intent)); // no duplicate prepare
  auto forged=f.intent;forged.insert("evidenceId",QUuid::createUuid().toString());
  QVERIFY(f.store.recordAnalysis(f.id,3,f.evidence,"classic-pcap",f.report,f.identity.actor(),forged).isEmpty());
  QVERIFY(!f.store.recordAnalysis(f.id,3,f.evidence,"classic-pcap",f.report,f.identity.actor(),f.intent).isEmpty());
  QCOMPARE(f.store.reconcileAnalysis(f.intent),QString("completed"));
  QVERIFY(f.store.recordAnalysis(f.id,3,f.evidence,"classic-pcap",f.report,f.identity.actor(),f.intent).isEmpty());
  QCOMPARE(f.store.get(f.id).value("analyses").toList().size(),1);
 }
 void terminalBindingAndIdempotency(){
  Fixture f;QVERIFY(f.policy.recordCaseOperation(f.intent));
  auto outcome=f.intent;outcome.insert("recoveryState","failed");
  auto forged=outcome;forged.insert("reportSha256",QString(64,'0'));
  QVERIFY(!f.policy.recordCaseOperation(forged));
  QVERIFY(f.policy.recordCaseOperation(outcome));
  const auto anchor=f.policy.auditStatus();QVERIFY(f.policy.recordCaseOperation(outcome));QCOMPARE(f.policy.auditStatus(),anchor);
  outcome.insert("recoveryState","completed");QVERIFY(!f.policy.recordCaseOperation(outcome));
  QString why;QFile exportFile(f.dir.filePath("audit.json"));
  QVERIFY(f.identity.login("admin",password));
  QVERIFY(f.policy.exportAudit(exportFile.fileName(),true));QVERIFY(exportFile.open(QIODevice::ReadOnly));
  auto retained=f.policy.auditStatus();retained.remove("healthy");retained.remove("verification");
  QVERIFY2(PolicyAudit::verifyExport(exportFile.readAll(),retained.value("publicKey").toString(),retained,why),qPrintable(why));
 }
 void changedCaseIsUnknown(){
  Fixture f;QVERIFY(f.policy.recordCaseOperation(f.intent));
  QVERIFY(f.store.update(f.id,3,{{"caseNumber","Changed"}},f.identity.actor()));
  QCOMPARE(f.store.reconcileAnalysis(f.intent),QString("unknown"));
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);
  QVERIFY(!bridge.ready());QCOMPARE(f.policy.caseOperationStates().first().toMap().value("recoveryState").toString(),QString("unknown"));
 }
 void missingEvidenceIsNeverSuccess(){
  Fixture f;QVERIFY(f.policy.recordCaseOperation(f.intent));
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","synthetic-corruption");db.setDatabaseName(f.dir.filePath("cases.db"));QVERIFY(db.open());
   {QSqlQuery q(db);QVERIFY(q.exec("UPDATE law_evidence SET payload=X'00'"));}
   db.close();
  }
  QSqlDatabase::removeDatabase("synthetic-corruption");
  QCOMPARE(f.store.reconcileAnalysis(f.intent),QString("unknown"));
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);QVERIFY(!bridge.ready());
  QCOMPARE(f.policy.caseOperationStates().first().toMap().value("recoveryState").toString(),QString("unknown"));
 }
 void tamperedJournalStopsRecovery(){
  Fixture f;QVERIFY(f.policy.recordCaseOperation(f.intent));
  QFile journal(f.identity.policyJournalPath());QVERIFY(journal.open(QIODevice::Append));journal.write("X");journal.close();
  LawCaseBridge bridge(f.dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&f.policy);QVERIFY(!bridge.ready());
  QCOMPARE(f.store.get(f.id).value("revision").toInt(),3);
 }
 void interruptedWrites_data(){
  QTest::addColumn<QString>("stage");QTest::addColumn<QString>("outcome");QTest::addColumn<int>("analyses");
  QTest::newRow("before-intent-write")<<QString("before-prepare")<<QString()<<0;
  QTest::newRow("inside-intent-atomic-replacement")<<QString("during-prepare")<<QString()<<0;
  QTest::newRow("after-signed-intent")<<QString("after-prepare")<<QString("failed")<<0;
  QTest::newRow("inside-case-transaction")<<QString("during-case")<<QString("failed")<<0;
  QTest::newRow("after-case-before-terminal")<<QString("after-case")<<QString("completed")<<1;
  QTest::newRow("inside-terminal-atomic-replacement")<<QString("during-terminal")<<QString("completed")<<1;
  QTest::newRow("after-terminal")<<QString("after-terminal")<<QString("completed")<<1;
 }
 void interruptedWrites(){
  QFETCH(QString,stage);QFETCH(QString,outcome);QFETCH(int,analyses);
  QTemporaryDir dir;QProcess child;child.start(QCoreApplication::applicationFilePath(),{"--crash-fixture",dir.path(),stage});
  QVERIFY(child.waitForFinished(20000));QCOMPARE(child.exitCode(),86);
  IdentityService identity(dir.filePath("identity"),true);GovernmentOperationPolicy policy(&identity);
  LawCaseBridge bridge(dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&policy);QVERIFY2(bridge.ready(),qPrintable(bridge.error()));
  const auto states=policy.caseOperationStates();
  if(outcome.isEmpty())QVERIFY(states.isEmpty());else {QCOMPARE(states.size(),1);QCOMPARE(states.first().toMap().value("recoveryState").toString(),outcome);}
  const auto anchor=policy.auditStatus();bridge.setOperationPolicy(&policy);QCOMPARE(policy.auditStatus(),anchor);
  IdentityService secondIdentity(dir.filePath("identity"),true);GovernmentOperationPolicy secondPolicy(&secondIdentity);
  LawCaseBridge secondBridge(dir.filePath("cases.db"),nullptr,true);secondBridge.setOperationPolicy(&secondPolicy);QVERIFY(secondBridge.ready());QCOMPARE(secondPolicy.auditStatus(),anchor);
  LawCaseStore store(dir.filePath("cases.db"),true);const auto cases=store.list();QCOMPARE(cases.size(),1);
  const auto record=cases.first().toMap();QCOMPARE(record.value("analyses").toList().size(),analyses);QCOMPARE(record.value("revision").toInt(),3+analyses);QVERIFY(store.verifyAudit(record.value("id").toString()));
 }
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);const auto args=app.arguments();if(args.size()==4 && args[1]=="--recover-orphan")return recoverOrphanFixture(args[2],args[3]);if(args.size()==4 && args[1]=="--crash-fixture")return crashFixture(args[2],args[3]);CaseTransactionTests test;return QTest::qExec(&test,argc,argv);}
#include "case_transaction_tests.moc"
