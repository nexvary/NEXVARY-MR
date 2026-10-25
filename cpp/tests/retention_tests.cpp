#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "LawRetention.hpp"
#include "IdentityService.hpp"
#include "CaseVault.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QUuid>
namespace {
QVariantMap meta(){return {{"caseNumber","SYNTHETIC-RETENTION"},{"investigatingAgency","Synthetic lab"},{"investigatorName","Synthetic investigator"},{"seizedDeviceDescription","Synthetic fixture"},{"authorityReference","Synthetic authority"},{"issuingAuthority","Lab"},{"scope","Synthetic files only"}};}
QVariantMap policy(){return {{"label","Synthetic review policy"},{"days",1},{"archiveAfterDays",0},{"authorityReference","SYNTHETIC-POLICY-1"}};}
QByteArray json(const QVariantMap &v){return QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact);}
QString hash(const QByteArray &v){return QString::fromLatin1(QCryptographicHash::hash(v,QCryptographicHash::Sha256).toHex());}
// Synthetic database fixture only: set historical registration and reseal its audit head.
bool age(const QString &path,const QString &id){
 const auto name=QUuid::createUuid().toString();bool ok=false;
 {auto db=QSqlDatabase::addDatabase("QSQLITE",name);db.setDatabaseName(path);if(db.open()){
  QSqlQuery q(db);q.prepare("SELECT payload FROM law_cases WHERE id=?");q.addBindValue(id);
  if(q.exec()&&q.next()){QString error;auto r=QJsonDocument::fromJson(mr::recovery::openCaseBytes(q.value(0).toByteArray(),true,error)).toVariant().toMap();auto rows=r.value("evidence").toList();auto e=rows.first().toMap();e.insert("registeredAtUtc","2000-01-01T00:00:00.000Z");rows[0]=e;r.insert("evidence",rows);auto audit=r.take("audit").toList();auto last=audit.last().toMap();last.insert("stateSha256",hash(json(r)));last.remove("hash");last.insert("hash",hash(json(last)));audit.last()=last;r.insert("audit",audit);q.finish();q.prepare("UPDATE law_cases SET payload=? WHERE id=?");q.addBindValue(mr::recovery::sealCase(json(r),true,error));q.addBindValue(id);ok=q.exec();}
  db.close();
 }}QSqlDatabase::removeDatabase(name);return ok;
}
QString add(LawCaseBridge &b,const QString &path){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly))return {};f.write("SYNTHETIC RETENTION BYTES");f.close();const auto d=b.addDevice({{"model","Synthetic"},{"custodian","A"}});return b.addEvidence(d,path,"Synthetic source","A",true);}
}
class RetentionTests:public QObject {
 Q_OBJECT
private slots:
 void legacyMigrationAndProtectedRoundTrip(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge owner(path,nullptr,true);const auto id=owner.createCase(meta()),e=add(owner,t.filePath("source"));QVERIFY(!e.isEmpty());
  const auto original=owner.activeCase();QVERIFY(!original.contains("retention"));QVERIFY(!owner.retentionAssessment(e).value("disposalEligible").toBool());
  LawCaseBridge reviewer(path,nullptr,true,nexvary::law::Role::Reviewer);QVERIFY(reviewer.openCase(id));QVERIFY(reviewer.retentionOperation("policy",policy()));
  const auto pid=reviewer.activeCase().value("retention").toMap().value("policies").toList().first().toMap().value("id");
  QVERIFY(reviewer.retentionOperation("assign",{{"evidenceId",e},{"policyId",pid}}));QVERIFY(reviewer.retentionOperation("archive",{{"evidenceId",e}}));
  const auto archive=reviewer.activeCase().value("retention").toMap().value("archives").toList().first().toMap();QVERIFY(reviewer.verifyArchive(archive.value("id").toString()).value("verified").toBool());
  QVERIFY(reviewer.retentionOperation("hold",{{"evidenceId",e},{"kind","legal-hold"},{"reason","Synthetic legal hold"},{"authorityReference","Synthetic authority"}}));
  const auto backup=reviewer.exportProtectedBackup(t.filePath("backup"),true);QVERIFY2(!backup.isEmpty(),qPrintable(reviewer.error()));
  LawCaseBridge restored(t.filePath("recovered.db"),nullptr,true);QCOMPARE(restored.restoreProtectedBackup(t.filePath("backup"),backup.value("sha256").toString(),true),id);
  QCOMPARE(restored.activeCase().value("retention"),reviewer.activeCase().value("retention"));QVERIFY(restored.retentionAssessment(e).value("held").toBool());QVERIFY(restored.verifyArchive(archive.value("id").toString()).value("verified").toBool());QVERIFY(restored.verifyAudit());
  LawCaseBridge reopened(path,nullptr,true);QVERIFY(reopened.openCase(id));QVERIFY(reopened.verifyEvidence(e));QCOMPARE(reopened.activeCase().value("evidence"),original.value("evidence"));QVERIFY(reopened.exportCasePdf(t.filePath("retention.pdf"),true,true));
 }
 void malformedExtensionsFailClosed(){
  QVariantMap r{{"evidence",QVariantList()}};
  QVariantMap g{{"schema",1},{"policies",QVariantList()},{"assignments",QVariantList()},{"holds",QVariantList()},{"archives",QVariantList()},{"disposals",QVariantList()},{"events",QVariantList()}};
  r.insert("retention",g);QVERIFY(nexvary::law::validRetention(r));
  for(const auto &v:QVariantList{true,"1",2,1.5}){auto bad=g;bad.insert("schema",v);r.insert("retention",bad);QVERIFY(!nexvary::law::validRetention(r));}
  auto bad=g;bad.insert("unknown",true);r.insert("retention",bad);QVERIFY(!nexvary::law::validRetention(r));
  bad=g;bad.insert("holds",QString("wrong type"));r.insert("retention",bad);QVERIFY(!nexvary::law::validRetention(r));
  QVariantList excessive;for(int i=0;i<201;++i)excessive.append(QVariantMap());bad=g;bad.insert("events",excessive);r.insert("retention",bad);QVERIFY(!nexvary::law::validRetention(r));
 }
 void exactExpiryAndPreservationBoundaries(){
  const auto e=QUuid::createUuid().toString(QUuid::WithoutBraces),pid=QUuid::createUuid().toString(QUuid::WithoutBraces);
  QVariantMap r{{"status","closed"},{"evidence",QVariantList{QVariantMap{{"id",e},{"registeredAtUtc","2024-02-28T12:00:00.000Z"}}}}};
  QVariantMap g{{"schema",1},{"policies",QVariantList{QVariantMap{{"id",pid},{"label","Synthetic"},{"authorityReference","SYNTHETIC"},{"days",1},{"archiveAfterDays",0},{"atUtc","2024-02-28T12:00:00Z"}}}},{"assignments",QVariantList{QVariantMap{{"evidenceId",e},{"policyId",pid},{"atUtc","2024-02-28T12:00:00Z"}}}},{"holds",QVariantList()},{"archives",QVariantList()},{"disposals",QVariantList()},{"events",QVariantList()}};
  r.insert("retention",g);QVERIFY(nexvary::law::validRetention(r));
  const auto time=QDateTime::fromString("2024-02-29T12:00:00Z",Qt::ISODate);QVERIFY(!nexvary::law::assessRetention(r,e,time.addMSecs(-1)).value("expired").toBool());QVERIFY(nexvary::law::assessRetention(r,e,time).value("disposalEligible").toBool());
  for(const auto &kind:{"legal-hold","preservation-exception"}){g.insert("holds",QVariantList{QVariantMap{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"evidenceId",e},{"kind",kind},{"reason","Synthetic hold"},{"atUtc","2024-02-28T12:00:00Z"}}});r.insert("retention",g);QVERIFY(!nexvary::law::assessRetention(r,e,time).value("disposalEligible").toBool());}
  g.insert("holds",QVariantList());r.insert("retention",g);r.insert("status","draft");QVERIFY(!nexvary::law::assessRetention(r,e,time).value("disposalEligible").toBool());QVERIFY(!nexvary::law::assessRetention(r,e,{}).value("disposalEligible").toBool());QVERIFY(!nexvary::law::assessRetention(r,"other",time).value("policyAssigned").toBool());
 }
 void validationStaleWritesAndPermissions(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge owner(path,nullptr,true);const auto id=owner.createCase(meta()),e=add(owner,t.filePath("source"));
  QVERIFY(!owner.retentionOperation("policy",policy()));LawCaseBridge a(path,nullptr,true,nexvary::law::Role::Reviewer),b(path,nullptr,true,nexvary::law::Role::Reviewer);QVERIFY(a.openCase(id));QVERIFY(b.openCase(id));
  for(const auto &v:QVariantList{0,-1,36501,1.5,true,"1"}){auto f=policy();f.insert("days",v);QVERIFY(!a.retentionOperation("policy",f));}
  auto f=policy();f.insert("unknown","bad");QVERIFY(!a.retentionOperation("policy",f));f=policy();f.insert("label",QString(3000,'x'));QVERIFY(!a.retentionOperation("policy",f));QVERIFY(!a.retentionOperation("invented",{}));
  QVERIFY(a.retentionOperation("policy",policy()));QVERIFY(!b.retentionOperation("policy",policy()));QVERIFY(b.openCase(id));
  const auto pid=a.activeCase().value("retention").toMap().value("policies").toList().first().toMap().value("id");QVERIFY(!a.retentionOperation("assign",{{"evidenceId","different-case"},{"policyId",pid}}));QVERIFY(a.retentionOperation("assign",{{"evidenceId",e},{"policyId",pid}}));QVERIFY(!a.retentionOperation("request-disposal",{{"evidenceId",e},{"reason","Synthetic"},{"authorityReference","Synthetic"}}));
  for(const auto role:{nexvary::law::Role::Auditor,nexvary::law::Role::Trainer,nexvary::law::Role::RepairTechnician}){LawCaseBridge denied(path,nullptr,true,role);denied.openCase(id);QVERIFY(!denied.retentionOperation("hold",{{"evidenceId",e},{"kind","legal-hold"},{"reason","Synthetic"},{"authorityReference","Synthetic"}}));}
  QVERIFY(a.verifyAudit());QVERIFY(a.verifyEvidence(e));
 }
 void legalHoldReleaseAndClosedArchive(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge owner(path,nullptr,true);const auto id=owner.createCase(meta()),e=add(owner,t.filePath("source"));QVERIFY(owner.transitionCase("closed"));
  QVERIFY(owner.retentionOperation("hold",{{"evidenceId",e},{"kind","preservation-exception"},{"reason","Synthetic hold"},{"authorityReference","Synthetic"}}));
  const auto hid=owner.activeCase().value("retention").toMap().value("holds").toList().first().toMap().value("id");QVERIFY(!owner.retentionOperation("release",{{"holdId",hid},{"reason","Synthetic release"},{"authorityReference","Synthetic"}}));
  LawCaseBridge reviewer(path,nullptr,true,nexvary::law::Role::Reviewer);QVERIFY(reviewer.openCase(id));QVERIFY(reviewer.retentionOperation("release",{{"holdId",hid},{"reason","Synthetic release"},{"authorityReference","Synthetic"}}));QVERIFY(!reviewer.retentionOperation("release",{{"holdId",hid},{"reason","Again"},{"authorityReference","Synthetic"}}));QVERIFY(!reviewer.retentionAssessment(e).value("held").toBool());QVERIFY(reviewer.transitionCase("archived"));QVERIFY(reviewer.retentionOperation("policy",policy()));QVERIFY(reviewer.verifyAudit());
 }
 void independentReviewRechecksHoldAndRevokedGrant(){
  QTemporaryDir t;const auto path=t.filePath("cases.db"),registry=t.filePath("users");const QString password="Synthetic retention password 2026!";
  IdentityService admin(registry,true);QVERIFY(admin.bootstrap("admin","Synthetic admin",password));QVERIFY(admin.login("admin",password));QVERIFY(!admin.createUser("owner","Synthetic owner","investigator",password).isEmpty());const auto rid=admin.createUser("reviewer","Synthetic reviewer","reviewer",password);QVERIFY(!rid.isEmpty());
  IdentityService oi(registry,true),ri(registry,true);QVERIFY(oi.login("owner",password));QVERIFY(ri.login("reviewer",password));
  LawCaseBridge owner(path,nullptr,true,nexvary::law::Role::Investigator,nullptr,&oi),reviewer(path,nullptr,true,nexvary::law::Role::Reviewer,nullptr,&ri);
  const auto id=owner.createCase(meta()),e=add(owner,t.filePath("source"));QVERIFY(age(path,id));QVERIFY(owner.openCase(id));QVERIFY(admin.setCaseGrant(id,rid,true));QVERIFY(reviewer.openCase(id));
  QVERIFY(reviewer.retentionOperation("policy",policy()));const auto pid=reviewer.activeCase().value("retention").toMap().value("policies").toList().first().toMap().value("id");QVERIFY(reviewer.retentionOperation("assign",{{"evidenceId",e},{"policyId",pid}}));QVERIFY(owner.openCase(id));QVERIFY(owner.transitionCase("closed"));QVERIFY(owner.retentionOperation("request-disposal",{{"evidenceId",e},{"reason","Synthetic expiry"},{"authorityReference","SYNTHETIC"}}));const auto request=owner.activeCase().value("retention").toMap().value("disposals").toList().first().toMap().value("id");
  QVariantMap review{{"requestId",request},{"decision","approved"},{"reason","Synthetic independent review"},{"authorityReference","SYNTHETIC"}};QVERIFY(!owner.retentionOperation("review-disposal",review));
  QVERIFY(owner.retentionOperation("hold",{{"evidenceId",e},{"kind","legal-hold"},{"reason","Synthetic litigation"},{"authorityReference","SYNTHETIC"}}));const auto hold=owner.activeCase().value("retention").toMap().value("holds").toList().first().toMap().value("id");QVERIFY(reviewer.openCase(id));QVERIFY(!reviewer.retentionOperation("review-disposal",review));
  QVERIFY(reviewer.retentionOperation("release",{{"holdId",hold},{"reason","Synthetic clearance"},{"authorityReference","SYNTHETIC"}}));QVERIFY(reviewer.retentionOperation("review-disposal",review));QVERIFY(!reviewer.retentionOperation("review-disposal",review));QVERIFY(reviewer.verifyEvidence(e));QVERIFY(reviewer.verifyAudit());
  QVERIFY(admin.setCaseGrant(id,rid,false));QVERIFY(!reviewer.retentionOperation("policy",policy()));QVERIFY(reviewer.activeCase().isEmpty());
 }
 void corruptedArchiveAndAtomicRollback(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge owner(path,nullptr,true);const auto id=owner.createCase(meta()),e=add(owner,t.filePath("source"));LawCaseBridge reviewer(path,nullptr,true,nexvary::law::Role::Reviewer);QVERIFY(reviewer.openCase(id));QVERIFY(reviewer.retentionOperation("policy",policy()));const auto pid=reviewer.activeCase().value("retention").toMap().value("policies").toList().first().toMap().value("id");QVERIFY(reviewer.retentionOperation("assign",{{"evidenceId",e},{"policyId",pid}}));QVERIFY(reviewer.retentionOperation("archive",{{"evidenceId",e}}));const auto aid=reviewer.activeCase().value("retention").toMap().value("archives").toList().first().toMap().value("id").toString();
  const auto before=reviewer.activeCase();const auto name=QUuid::createUuid().toString();{auto db=QSqlDatabase::addDatabase("QSQLITE",name);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("CREATE TRIGGER reject_retention BEFORE UPDATE ON law_cases BEGIN SELECT RAISE(ABORT,'synthetic write rejection'); END"));QVERIFY(!reviewer.retentionOperation("policy",policy()));QVERIFY(q.exec("DROP TRIGGER reject_retention"));QVERIFY(q.exec("UPDATE law_evidence SET payload=X'00'"));db.close();}QSqlDatabase::removeDatabase(name);
  QVERIFY(reviewer.openCase(id));QCOMPARE(reviewer.activeCase(),before);QVERIFY(reviewer.verifyArchive(aid).isEmpty());QVERIFY(!reviewer.retentionOperation("archive",{{"evidenceId",e}}));QVERIFY(reviewer.verifyAudit());
 }
};
QTEST_MAIN(RetentionTests)
#include "retention_tests.moc"
