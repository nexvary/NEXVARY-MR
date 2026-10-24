#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "CaseVault.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGuiApplication>
namespace {
QVariantMap metadata(){return {{"caseNumber","SYNTHETIC-CASE-001"},{"investigatingAgency","Synthetic agency"},{"investigatorName","Synthetic operator"},{"seizedDeviceDescription","Owned lab fixture"},{"authorityReference","Synthetic scope declaration"},{"issuingAuthority","Lab owner"},{"scope","Synthetic metadata and fixture files only"},{"jurisdiction","Configurable test jurisdiction"}};}
QVariantMap device(){return {{"model","Synthetic model"},{"custodian","Custodian A"},{"serial","SYNTHETIC"}};}
}
class LawWorkspaceTests:public QObject {
 Q_OBJECT
private slots:
 void createSearchReopenAndDevice() {
  QTemporaryDir temp;QString id,dev;
  {LawCaseBridge b(temp.filePath("cases.db"),nullptr,true);QVERIFY2(b.ready(),qPrintable(b.error()));id=b.createCase(metadata());QVERIFY2(!id.isEmpty(),qPrintable(b.error()));dev=b.addDevice(device());QVERIFY(!dev.isEmpty());QCOMPARE(b.devices().size(),1);QVERIFY(b.verifyAudit());b.closeWorkspace();QVERIFY(b.activeCase().isEmpty());QVERIFY(b.searchCases("SYNTHETIC-CASE"));QCOMPARE(b.cases().size(),1);QVERIFY(b.searchCases("absent"));QCOMPARE(b.cases().size(),0);}
  LawCaseBridge reopened(temp.filePath("cases.db"),nullptr,true);QVERIFY(reopened.openCase(id));QCOMPARE(reopened.devices().first().toMap().value("id").toString(),dev);QCOMPARE(reopened.activeCase().value("revision").toInt(),2);QVERIFY(reopened.verifyAudit());
 }
 void permissionsAreNativeAndFailClosed() {
  using namespace nexvary::law;for(const auto role:{Role::Investigator,Role::Reviewer,Role::Auditor,Role::Trainer,Role::RepairTechnician,Role::Unknown,static_cast<Role>(99)})QVERIFY(!allows(role,Action::ExecuteDevice));
  QTemporaryDir temp;LawCaseBridge investigator(temp.filePath("cases.db"),nullptr,true);const auto id=investigator.createCase(metadata());QVERIFY(!id.isEmpty());
  for(const auto role:{Role::Reviewer,Role::Auditor,Role::Trainer,Role::RepairTechnician,Role::Unknown}){LawCaseBridge b(temp.filePath("cases.db"),nullptr,true,role);QVERIFY(b.createCase(metadata()).isEmpty());QVERIFY(b.addDevice(device()).isEmpty());QVERIFY(!b.updateCase(metadata()));QVERIFY(b.addEvidence("x","x","x","x",true).isEmpty());}
  LawCaseBridge trainer(temp.filePath("cases.db"),nullptr,true,Role::Trainer);QVERIFY(!trainer.openCase(id));QVERIFY(trainer.cases().isEmpty());QVERIFY(!trainer.scenarios().isEmpty());QVERIFY(trainer.evaluateTraining("usb-authorization","consent").value("correct").toBool());QVERIFY(!trainer.evaluateTraining("usb-authorization","invented").value("valid").toBool());
 }
 void staleWriterAndArchivedState() {
  QTemporaryDir temp;LawCaseBridge a(temp.filePath("cases.db"),nullptr,true),b(temp.filePath("cases.db"),nullptr,true);const auto id=a.createCase(metadata());QVERIFY(b.openCase(id));auto edited=metadata();edited.insert("notes","First writer");QVERIFY(a.updateCase(edited));QVERIFY(!b.updateCase(metadata()));QVERIFY(b.openCase(id));QVERIFY(!a.transitionCase("authorized"));QVERIFY(a.transitionCase("closed"));QVERIFY(!a.addDevice(device()).size());QVERIFY(a.verifyAudit());
 }
 void metadataAndProfileValidation() {
  QTemporaryDir temp;LawCaseBridge b(temp.filePath("cases.db"),nullptr,true);auto m=metadata();m.remove("scope");QVERIFY(b.createCase(m).isEmpty());m=metadata();m.insert("validFromUtc","2026-02-30T00:00:00Z");QVERIFY(b.createCase(m).isEmpty());m=metadata();m.insert("unknown","value");QVERIFY(b.createCase(m).isEmpty());m=metadata();m.insert("notes",QString(QChar(0x202e))+"spoof");QVERIFY(b.createCase(m).isEmpty());
#ifndef Q_OS_WIN
  LawCaseBridge production(temp.filePath("real.db"),nullptr,false);QVERIFY(!production.ready());QVERIFY(production.createCase(metadata()).isEmpty());QVERIFY(!QFile::exists(temp.filePath("real.db")));
#endif
 }
 void evidenceCustodyAndPdf() {
  QTemporaryDir temp;LawCaseBridge b(temp.filePath("cases.db"),nullptr,true);const auto id=b.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=b.addDevice(device());QVERIFY(!dev.isEmpty());QFile f(temp.filePath("fixture.txt"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("SYNTHETIC EVIDENCE\n");f.close();
  QVERIFY(b.addEvidence(dev,f.fileName(),"Fixture","Custodian A",false).isEmpty());QVERIFY(b.addEvidence("other-device",f.fileName(),"Fixture","Custodian A",true).isEmpty());
  const auto evidence=b.addEvidence(dev,f.fileName(),"Fixture","Custodian A",true);QVERIFY2(!evidence.isEmpty(),qPrintable(b.error()));QVERIFY(b.verifyEvidence(evidence));QVERIFY(!b.transferCustody(evidence,"Wrong","Custodian B","Transfer"));QVERIFY(b.transferCustody(evidence,"Custodian A","Custodian B","Sealed lab transfer"));QVERIFY(b.verifyAudit());QCOMPARE(b.activeCase().value("custody").toList().size(),2);
  QVERIFY(!b.exportCasePdf(temp.filePath("report.pdf"),true,false));QVERIFY(b.exportCasePdf(temp.filePath("report.pdf"),true,true));QFile pdf(temp.filePath("report.pdf"));QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.read(5)=="%PDF-");QVERIFY(!b.exportCasePdf(pdf.fileName(),true,true));
  LawCaseBridge reopened(temp.filePath("cases.db"),nullptr,true);QVERIFY(reopened.openCase(id));QVERIFY(reopened.verifyEvidence(evidence));QCOMPARE(reopened.activeCase().value("evidence").toList().first().toMap().value("custodian").toString(),QString("Custodian B"));
 }
 void corruptionCannotBeSilentlyOpened() {
  QTemporaryDir temp;const auto path=temp.filePath("cases.db");QString id;
  {LawCaseBridge b(path,nullptr,true);id=b.createCase(metadata());QVERIFY(!id.isEmpty());}
  const auto connection=QStringLiteral("tamper-fixture");{
   auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("SELECT payload FROM law_cases"));QVERIFY(q.next());auto payload=q.value(0).toByteArray();QVERIFY(payload.size()>100);payload[payload.size()/2]=static_cast<char>(payload[payload.size()/2]^1);q.prepare("UPDATE law_cases SET payload=?");q.addBindValue(payload);QVERIFY(q.exec());db.close();
  }QSqlDatabase::removeDatabase(connection);
  LawCaseBridge b(path,nullptr,true);QVERIFY(!b.openCase(id));QVERIFY(!b.verifyAudit());
 }
 void evidenceCorruptionAndSizeLimit() {
  QTemporaryDir temp;const auto path=temp.filePath("cases.db");QString id,evidence;{
   LawCaseBridge b(path,nullptr,true);id=b.createCase(metadata());const auto dev=b.addDevice(device());QFile file(temp.filePath("data"));QVERIFY(file.open(QIODevice::WriteOnly));file.write("SYNTHETIC");file.close();evidence=b.addEvidence(dev,file.fileName(),"Fixture","A",true);QVERIFY(!evidence.isEmpty());
   QFile large(temp.filePath("large"));QVERIFY(large.open(QIODevice::WriteOnly));QVERIFY(large.resize(16*1024*1024+1));large.close();QVERIFY(b.addEvidence(dev,large.fileName(),"Large","A",true).isEmpty());QVERIFY(b.verifyAudit());
  }
  const auto connection=QStringLiteral("tamper-evidence");{auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("UPDATE law_evidence SET payload=x'00'"));db.close();}QSqlDatabase::removeDatabase(connection);
  LawCaseBridge b(path,nullptr,true);QVERIFY(b.openCase(id));QVERIFY(!b.verifyEvidence(evidence));
 }
};
QTEST_MAIN(LawWorkspaceTests)
#include "law_workspace_tests.moc"
