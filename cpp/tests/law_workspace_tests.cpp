#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "DeviceAssessment.hpp"
#include "ExifFixture.hpp"
#include "CaseVault.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QUuid>
namespace {
QVariantMap metadata(){return {{"caseNumber","SYNTHETIC-CASE-001"},{"investigatingAgency","Synthetic agency"},{"investigatorName","Synthetic operator"},{"seizedDeviceDescription","Owned lab fixture"},{"authorityReference","Synthetic scope declaration"},{"issuingAuthority","Lab owner"},{"scope","Synthetic metadata and fixture files only"},{"jurisdiction","Configurable test jurisdiction"}};}
QVariantMap device(){return {{"model","Synthetic model"},{"custodian","Custodian A"},{"serial","SYNTHETIC"}};}
bool writeFixture(const QString &path,const QByteArray &bytes){QFile f(path);return f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(bytes)==bytes.size();}
QByteArray readFixture(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
QString fileHash(const QString &path){return QString::fromLatin1(QCryptographicHash::hash(readFixture(path),QCryptographicHash::Sha256).toHex());}

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
 void caseBoundAnalysisIsReproducibleAndProtected() {
  QTemporaryDir t;LawCaseBridge b(t.filePath("cases.db"),nullptr,true);const auto id=b.createCase(metadata()),dev=b.addDevice(device());QVERIFY(!id.isEmpty());
  QFile f(t.filePath("capture.pcap"));QVERIFY(f.open(QIODevice::WriteOnly));const auto pcap=QByteArray::fromHex("d4c3b2a1020004000000000000000000ffff00000100000002000000050000000e0000000e0000000000000000000000000000000806");f.write(pcap);f.close();
  const auto e=b.addEvidence(dev,f.fileName(),"Synthetic PCAP","A",true);QVERIFY(!e.isEmpty());const auto first=b.analyzeEvidence(e,"classic-pcap");QVERIFY2(!first.isEmpty(),qPrintable(b.error()));
  const auto second=b.analyzeEvidence(e,"classic-pcap");QVERIFY(!second.isEmpty());QCOMPARE(first.value("reportSha256"),second.value("reportSha256"));QCOMPARE(first.value("reportJson"),second.value("reportJson"));
  const auto report=QJsonDocument::fromJson(first.value("reportJson").toString().toUtf8()).object();QCOMPARE(report.value("packetCount").toString(),QString("1"));QVERIFY(!report.value("decryptionPerformed").toBool());QVERIFY(b.verifyEvidence(e));QVERIFY(b.verifyAudit());
  QVERIFY(b.analyzeEvidence(e,"invented").isEmpty());QVERIFY(b.analyzeEvidence("different-case-evidence","classic-pcap").isEmpty());
  QFile jpeg(t.filePath("photo.jpg"));QVERIFY(jpeg.open(QIODevice::WriteOnly));jpeg.write(syntheticExifJpeg());jpeg.close();const auto image=b.addEvidence(dev,jpeg.fileName(),"Synthetic JPEG","A",true);QVERIFY(!image.isEmpty());QVERIFY(!b.analyzeEvidence(image,"jpeg-exif").isEmpty());QVERIFY(b.analyzeEvidence(image,"classic-pcap").isEmpty());
  QCOMPARE(b.activeCase().value("analyses").toList().size(),3);QVERIFY(b.exportCasePdf(t.filePath("analysis-review.pdf"),true,true));LawCaseBridge auditor(t.filePath("cases.db"),nullptr,true,nexvary::law::Role::Auditor);QVERIFY(auditor.openCase(id));QVERIFY(auditor.analyzeEvidence(e,"classic-pcap").isEmpty());
 }
 void assessmentDistinguishesReportsFromValidatedCapabilities() {
  QVariantMap report{{"propertiesCurrent",true},{"fields",QVariantList{QVariantMap{{"id","model"},{"state","reported"},{"value","Synthetic model"},{"source","adb:shell getprop"},{"observedAtUtc","2026-10-25T00:00:00Z"},{"physicalDeviceTested",true}}}}};
  auto rows=nexvary::law::assessmentFields(report);QCOMPARE(rows.size(),13);
  const auto model=rows.at(1).toMap();QCOMPARE(model.value("evidenceClass").toString(),QString("reported"));QVERIFY(model.value("fact").toString().contains("Synthetic model"));
  for(const auto &v:rows){QVERIFY(!v.toMap().value("executionApproved").toBool());QVERIFY(!v.toMap().value("physicalDeviceTested").toBool());}
  report.insert("propertiesCurrent",false);rows=nexvary::law::assessmentFields(report);for(const auto &v:rows)QVERIFY(!v.toMap().value("fact").toString().contains("Synthetic model"));
 }
 void workingCopyIntegrityAndAudit() {
  QTemporaryDir temp;const auto database=temp.filePath("cases.db");LawCaseBridge b(database,nullptr,true);
  const auto id=b.createCase(metadata()),dev=b.addDevice(device());QVERIFY(!id.isEmpty());
  QFile source(temp.filePath("source.bin"));QVERIFY(source.open(QIODevice::WriteOnly));const QByteArray bytes("SYNTHETIC evidence\0payload",26);QCOMPARE(source.write(bytes),bytes.size());source.close();
  const auto evidence=b.addEvidence(dev,source.fileName(),"Fixture","A",true);QVERIFY(!evidence.isEmpty());
  const auto revision=b.activeCase().value("revision").toInt();const auto output=temp.filePath("package");
  QVERIFY(b.exportWorkingCopy(evidence,output,"Analysis",false).isEmpty());QVERIFY(!QFileInfo::exists(output));
  const auto result=b.exportWorkingCopy(evidence,output,"Read-only analysis",true);QVERIFY2(!result.isEmpty(),qPrintable(b.error()));
  QFile copy(QDir(output).filePath("evidence.bin")),manifest(QDir(output).filePath("manifest.json"));QVERIFY(copy.open(QIODevice::ReadOnly));QCOMPARE(copy.readAll(),bytes);QVERIFY(manifest.open(QIODevice::ReadOnly));
  const auto data=manifest.readAll();QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex()),result.value("manifestSha256").toString());
  const auto m=QJsonDocument::fromJson(data).object();QCOMPARE(m.value("caseId").toString(),id);QCOMPARE(m.value("caseRevision").toInt(),revision);QVERIFY(m.value("synthetic").toBool());
  QCOMPARE(b.activeCase().value("revision").toInt(),revision+1);QCOMPARE(b.activeCase().value("workingCopies").toList().size(),1);QCOMPARE(b.audit().last().toMap().value("event").toString(),QString("evidence-working-copy-exported"));QVERIFY(b.verifyAudit());
  // Editing the exported copy cannot alter the protected original.
  copy.close();QVERIFY(copy.open(QIODevice::WriteOnly|QIODevice::Truncate));copy.write("changed");copy.close();QVERIFY(b.verifyEvidence(evidence));QVERIFY(source.open(QIODevice::ReadOnly));QCOMPARE(source.readAll(),bytes);
  QVERIFY(b.exportWorkingCopy(evidence,output,"Retry",true).isEmpty());QCOMPARE(b.activeCase().value("revision").toInt(),revision+1);
  const auto fixture=qEnvironmentVariable("MR_EVIDENCE_FIXTURE_DIR");if(!fixture.isEmpty()){
   const auto exported=b.exportWorkingCopy(evidence,fixture,"Independent offline verifier fixture",true);QVERIFY2(!exported.isEmpty(),qPrintable(b.error()));
   QFile expected(fixture+".sha256");QVERIFY(expected.open(QIODevice::WriteOnly|QIODevice::NewOnly));expected.write(exported.value("manifestSha256").toString().toLatin1());
  }
 }
 void workingCopyPermissionsStaleRevisionAndClosedCase() {
  using namespace nexvary::law;QTemporaryDir temp;const auto path=temp.filePath("cases.db");LawCaseBridge a(path,nullptr,true),stale(path,nullptr,true);const auto id=a.createCase(metadata()),dev=a.addDevice(device());
  QFile f(temp.filePath("source"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("SYNTHETIC");f.close();const auto e=a.addEvidence(dev,f.fileName(),"Fixture","A",true);QVERIFY(!e.isEmpty());QVERIFY(stale.openCase(id));QVERIFY(!a.addDevice(device()).isEmpty());
  QVERIFY(stale.exportWorkingCopy(e,temp.filePath("stale"),"Analysis",true).isEmpty());QVERIFY(!QFileInfo::exists(temp.filePath("stale")));
  LawCaseBridge auditor(path,nullptr,true,Role::Auditor);QVERIFY(auditor.openCase(id));QVERIFY(auditor.exportWorkingCopy(e,temp.filePath("denied"),"Analysis",true).isEmpty());QVERIFY(!QFileInfo::exists(temp.filePath("denied")));
  QVERIFY(a.transitionCase("closed"));LawCaseBridge reviewer(path,nullptr,true,Role::Reviewer);QVERIFY(reviewer.openCase(id));QVERIFY(reviewer.transitionCase("archived"));
  QVERIFY(!reviewer.exportWorkingCopy(e,temp.filePath("archived-copy"),"Review closed case",true).isEmpty());QCOMPARE(reviewer.activeCase().value("status").toString(),QString("archived"));QVERIFY(reviewer.verifyAudit());QVERIFY(a.openCase(id));QVERIFY(a.addDevice(device()).isEmpty());
  QVERIFY(a.exportWorkingCopy("unrelated",temp.filePath("missing"),"Analysis",true).isEmpty());QVERIFY(!QFileInfo::exists(temp.filePath("missing")));
 }
 void protectedBackupRoundTripAndOverwriteDenial() {
  QTemporaryDir t;LawCaseBridge source(t.filePath("source.db"),nullptr,true);const auto id=source.createCase(metadata()),dev=source.addDevice(device());QVERIFY(!id.isEmpty());
  const auto file=t.filePath("fixture.bin");QVERIFY(writeFixture(file,"SYNTHETIC protected backup fixture"));const auto evidence=source.addEvidence(dev,file,"Fixture","A",true);QVERIFY(!evidence.isEmpty());
  const auto before=source.activeCase();const auto backup=t.filePath("case.nmrbackup");QVERIFY(source.exportProtectedBackup(backup,false).isEmpty());QVERIFY(!QFileInfo::exists(backup));
  const auto result=source.exportProtectedBackup(backup,true);QVERIFY2(!result.isEmpty(),qPrintable(source.error()));QCOMPARE(result.value("sha256").toString(),fileHash(backup));QCOMPARE(result.value("sourceRevision"),before.value("revision"));QVERIFY(source.verifyAudit());
  const auto protectedFile=readFixture(backup);if(mr::recovery::vaultAvailable())QVERIFY(!protectedFile.contains("SYNTHETIC-CASE-001"));
  const auto after=source.activeCase();QCOMPARE(after.value("revision").toInt(),before.value("revision").toInt()+1);QVERIFY(source.exportProtectedBackup(backup,true).isEmpty());QCOMPARE(readFixture(backup),protectedFile);
  LawCaseBridge restored(t.filePath("restored.db"),nullptr,true);QVERIFY(restored.restoreProtectedBackup(backup,result.value("sha256").toString(),false).isEmpty());QVERIFY(restored.restoreProtectedBackup(backup,QString(64,'0'),true).isEmpty());QVERIFY(restored.cases().isEmpty());
  QCOMPARE(restored.restoreProtectedBackup(backup,result.value("sha256").toString(),true),id);QCOMPARE(restored.activeCase().value("metadata"),before.value("metadata"));QCOMPARE(restored.activeCase().value("evidence"),before.value("evidence"));QVERIFY(restored.verifyEvidence(evidence));QVERIFY(restored.verifyAudit());QCOMPARE(restored.audit().last().toMap().value("event").toString(),QString("case-backup-restored"));
  QCOMPARE(restored.activeCase().value("restorations").toList().first().toMap().value("sourceRevision"),before.value("revision"));QVERIFY(source.exportCasePdf(t.filePath("backup-report.pdf"),true,true));QVERIFY(restored.exportCasePdf(t.filePath("restored-report.pdf"),true,true));QVERIFY(restored.restoreProtectedBackup(backup,result.value("sha256").toString(),true).isEmpty());QCOMPARE(restored.cases().size(),1);QVERIFY(source.verifyEvidence(evidence));QCOMPARE(source.activeCase().value("revision"),after.value("revision"));
 }
 void protectedBackupRejectsCorruptionMissingBlobsAndLimits() {
  QTemporaryDir t;LawCaseBridge source(t.filePath("source.db"),nullptr,true);source.createCase(metadata());const auto dev=source.addDevice(device());const auto file=t.filePath("fixture");QVERIFY(writeFixture(file,"SYNTHETIC fixture"));
  QVERIFY(!source.addEvidence(dev,file,"First","A",true).isEmpty());QVERIFY(!source.addEvidence(dev,file,"Second","A",true).isEmpty());const auto backup=t.filePath("original");const auto receipt=source.exportProtectedBackup(backup,true);QVERIFY(!receipt.isEmpty());QString error;
  const auto original=QJsonDocument::fromJson(mr::recovery::openCaseBytes(readFixture(backup),true,error)).object().toVariantMap();QVERIFY(!original.isEmpty());LawCaseBridge target(t.filePath("target.db"),nullptr,true);
  for(int kind=0;kind<10;++kind){auto envelope=original;auto blobs=envelope.value("evidence").toList();if(kind==0)blobs.removeLast();if(kind==1)blobs[1]=blobs[0];if(kind==2){auto blob=blobs[0].toMap();blob.insert("payloadBase64","INVALID-");blobs[0]=blob;}if(kind==3){auto blob=blobs[0].toMap();blob.insert("payloadBase64",QString::fromLatin1(mr::recovery::sealCase("CHANGED",true,error).toBase64()));blobs[0]=blob;}if(kind==4)envelope.insert("schemaVersion",2);if(kind==5)envelope.insert("synthetic",false);
   if(kind>=6&&kind<=8){auto record=QJsonDocument::fromJson(mr::recovery::openCaseBytes(QByteArray::fromBase64(envelope.value("casePayloadBase64").toString().toLatin1()),true,error)).object().toVariantMap();if(kind==6)record.insert("schema",true);if(kind==7)record.insert("deviceCount",QString::number(record.value("deviceCount").toInt()));if(kind==8){auto rows=record.value("evidence").toList();auto item=rows[0].toMap();item.insert("size",QString::number(item.value("size").toInt()));rows[0]=item;record.insert("evidence",rows);}auto events=record.value("audit").toList();auto event=events.last().toMap();auto state=record;state.remove("audit");event.insert("stateSha256",QString::fromLatin1(QCryptographicHash::hash(QJsonDocument::fromVariant(state).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex()));event.remove("hash");event.insert("hash",QString::fromLatin1(QCryptographicHash::hash(QJsonDocument::fromVariant(event).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex()));events.last()=event;record.insert("audit",events);envelope.insert("casePayloadBase64",QString::fromLatin1(mr::recovery::sealCase(QJsonDocument::fromVariant(record).toJson(QJsonDocument::Compact),true,error).toBase64()));}
   if(kind==9){envelope.insert("createdAtUtc",QStringLiteral("2026-10-25T00:00:00"));}
   envelope.insert("evidence",blobs);
   const auto path=t.filePath("malformed-"+QString::number(kind));QVERIFY(writeFixture(path,mr::recovery::sealCase(QJsonDocument::fromVariant(envelope).toJson(QJsonDocument::Compact),true,error)));QVERIFY2(target.restoreProtectedBackup(path,fileHash(path),true).isEmpty(),qPrintable(QString::number(kind)));QVERIFY(target.cases().isEmpty());
  }
  auto corrupted=readFixture(backup);corrupted[corrupted.size()/2]=static_cast<char>(corrupted.at(corrupted.size()/2)^1);const auto bad=t.filePath("damaged");QVERIFY(writeFixture(bad,corrupted));QVERIFY(target.restoreProtectedBackup(bad,fileHash(bad),true).isEmpty());
  QFile huge(t.filePath("oversized"));QVERIFY(huge.open(QIODevice::WriteOnly));QVERIFY(huge.resize(64*1024*1024+1));huge.close();QVERIFY(target.restoreProtectedBackup(huge.fileName(),QString(64,'0'),true).isEmpty());QVERIFY(target.cases().isEmpty());
#ifndef Q_OS_WIN
  const auto link=t.filePath("backup-link");QVERIFY(QFile::link(backup,link));QVERIFY(target.restoreProtectedBackup(link,receipt.value("sha256").toString(),true).isEmpty());
#endif
 }
 void protectedRestoreRollsBackConflictingEvidenceAndBackupWriteFailure() {
  QTemporaryDir t;const auto sourcePath=t.filePath("source.db"),targetPath=t.filePath("target.db");LawCaseBridge source(sourcePath,nullptr,true),target(targetPath,nullptr,true);source.createCase(metadata());const auto dev=source.addDevice(device());const auto file=t.filePath("fixture");QVERIFY(writeFixture(file,"SYNTHETIC"));QVERIFY(!source.addEvidence(dev,file,"First","A",true).isEmpty());const auto second=source.addEvidence(dev,file,"Second","A",true);QVERIFY(!second.isEmpty());const auto backup=t.filePath("backup");const auto receipt=source.exportProtectedBackup(backup,true);QVERIFY(!receipt.isEmpty());
  const auto connection=QStringLiteral("backup-conflict-fixture");{auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(targetPath);QVERIFY(db.open());{QSqlQuery q(db);q.prepare("INSERT INTO law_evidence(id,case_id,payload) VALUES(?,?,?)");q.addBindValue(second);q.addBindValue("unrelated-existing-row");q.addBindValue(QByteArray("preserve"));QVERIFY(q.exec());QVERIFY(target.restoreProtectedBackup(backup,receipt.value("sha256").toString(),true).isEmpty());QVERIFY(q.exec("SELECT COUNT(*) FROM law_cases"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);QVERIFY(q.exec("SELECT COUNT(*) FROM law_evidence"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),1);}db.close();}QSqlDatabase::removeDatabase(connection);
  const auto trigger=QStringLiteral("backup-failing-update");{auto db=QSqlDatabase::addDatabase("QSQLITE",trigger);db.setDatabaseName(sourcePath);QVERIFY(db.open());{QSqlQuery q(db);QVERIFY(q.exec("CREATE TRIGGER reject_backup_update BEFORE UPDATE ON law_cases BEGIN SELECT RAISE(ABORT,'synthetic failure'); END"));}db.close();}QSqlDatabase::removeDatabase(trigger);
  const auto revision=source.activeCase().value("revision");const auto rejected=t.filePath("rejected-backup");QVERIFY(source.exportProtectedBackup(rejected,true).isEmpty());QVERIFY(!QFileInfo::exists(rejected));QCOMPARE(source.activeCase().value("revision"),revision);QVERIFY(source.verifyAudit());
 }
 void protectedBackupStatusRolesStaleWriterAndAggregateLimit() {
  using namespace nexvary::law;QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge a(path,nullptr,true),stale(path,nullptr,true);const auto id=a.createCase(metadata());QVERIFY(stale.openCase(id));QVERIFY(!a.addDevice(device()).isEmpty());QVERIFY(stale.exportProtectedBackup(t.filePath("stale"),true).isEmpty());QVERIFY(!QFileInfo::exists(t.filePath("stale")));
  LawCaseBridge auditor(path,nullptr,true,Role::Auditor);QVERIFY(auditor.openCase(id));QVERIFY(auditor.exportProtectedBackup(t.filePath("denied"),true).isEmpty());QVERIFY(a.transitionCase("suspended"));QVERIFY(!a.exportProtectedBackup(t.filePath("suspended"),true).isEmpty());QVERIFY(a.addDevice(device()).isEmpty());QVERIFY(a.transitionCase("draft"));QVERIFY(a.transitionCase("closed"));
  LawCaseBridge reviewer(path,nullptr,true,Role::Reviewer);QVERIFY(reviewer.openCase(id));QVERIFY(reviewer.transitionCase("archived"));const auto backup=t.filePath("archived");const auto receipt=reviewer.exportProtectedBackup(backup,true);QVERIFY(!receipt.isEmpty());QVERIFY(reviewer.restoreProtectedBackup(backup,receipt.value("sha256").toString(),true).isEmpty());LawCaseBridge restored(t.filePath("archived-restored.db"),nullptr,true);QCOMPARE(restored.restoreProtectedBackup(backup,receipt.value("sha256").toString(),true),id);QCOMPARE(restored.activeCase().value("status").toString(),QString("archived"));QVERIFY(restored.addDevice(device()).isEmpty());QVERIFY(restored.verifyAudit());
  LawCaseBridge large(t.filePath("large.db"),nullptr,true);large.createCase(metadata());const auto dev=large.addDevice(device());const auto file=t.filePath("large-evidence");QVERIFY(writeFixture(file,QByteArray(11*1024*1024,'x')));for(int i=0;i<3;++i)QVERIFY(!large.addEvidence(dev,file,"Bounded fixture","A",true).isEmpty());const auto revision=large.activeCase().value("revision");const auto destination=t.filePath("over-limit-backup");QVERIFY(large.exportProtectedBackup(destination,true).isEmpty());QVERIFY(!QFileInfo::exists(destination));QCOMPARE(large.activeCase().value("revision"),revision);QVERIFY(large.verifyAudit());
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
