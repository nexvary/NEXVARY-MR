#include "RecoveryBridge.hpp"
#include "CaseVault.hpp"
#include "ForensicRecord.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
namespace {
QByteArray read(const QString &path){QFile file(path);return file.open(QIODevice::ReadOnly)?file.readAll():QByteArray();}
bool write(const QString &path,const QByteArray &bytes){QFile file(path);return file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size();}
bool create(RecoveryBridge &b){return b.createCase("<script>alert('fixture')</script>","SYNTHETIC-CONSENT","Lab","No device","15","backup","unknown",true);}
}
class ForensicTests final:public QObject {
 Q_OBJECT
private slots:
 void auditDetectsEditsAndReordering(){
  QJsonObject doc{{"operator","Synthetic operator"},{"audit",QJsonArray()}};
  mr::forensic::appendAudit(doc,"create","first");mr::forensic::appendAudit(doc,"read","second");
  QString error;QVERIFY(mr::forensic::validateHistory(doc,error));
  auto changed=doc;auto audit=changed.value("audit").toArray();auto entry=audit[0].toObject();entry.insert("outcome","edited");audit[0]=entry;changed.insert("audit",audit);
  QVERIFY(!mr::forensic::validateHistory(changed,error));
  changed=doc;audit=changed.value("audit").toArray();auto first=audit[0];audit[0]=audit[1];audit[1]=first;changed.insert("audit",audit);
  QVERIFY(!mr::forensic::validateHistory(changed,error));
  changed=doc;changed.remove("historyVersion");QVERIFY(!mr::forensic::validateHistory(changed,error));
 }
 void oldCasesUpgradeWithoutInventingIdentity(){
  QTemporaryDir temp;RecoveryBridge bridge(temp.filePath("cases"),true);QVERIFY(create(bridge));
  const auto path=bridge.casePath();QString error;
  auto doc=QJsonDocument::fromJson(mr::recovery::openCaseBytes(read(path),true,error)).object();QVERIFY(error.isEmpty());
  doc.remove("historyVersion");doc.insert("audit",QJsonArray{QJsonObject{{"atUtc","2026-10-01T00:00:00.000Z"},{"operation","legacy-event"},{"outcome","fixture"}}});
  QVERIFY(bridge.closeCase());QVERIFY(write(path,mr::recovery::sealCase(QJsonDocument(doc).toJson(),true,error)));
  QVERIFY(bridge.openCase(path));QTRY_VERIFY(!bridge.busy());QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));
  QVERIFY(bridge.caseOperator().isEmpty());QVERIFY(bridge.audit().first().toMap().value("legacyUnverified").toBool());
  QVERIFY(bridge.audit().first().toMap().value("operator").toString().isEmpty());
  QVERIFY(bridge.verifyEvidence());QTRY_VERIFY(!bridge.busy());QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));
 }
 void caseAcquisitionCustodyTimelineAndReports(){
  QTemporaryDir temp;QVERIFY(temp.isValid());RecoveryBridge bridge(temp.filePath("cases"),true);QVERIFY(create(bridge));
  QVERIFY(!bridge.recordOperator("Lab technician","Signed form LAB-1",false));
  QVERIFY(!bridge.recordOperator(" ","LAB-1",true));
  QVERIFY(bridge.recordOperator("Lab technician","Signed form LAB-1",true));
  const auto id=bridge.caseId();QVERIFY(!id.isEmpty());
  const auto source=temp.filePath("evidence.txt");const QByteArray original("Synthetic fixture <img src=x onerror=alert(1)>\n");QVERIFY(write(source,original));
  QVERIFY(bridge.importVisibleFile(source,true));QTRY_VERIFY(!bridge.busy());QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));
  QCOMPARE(read(source),original);QCOMPARE(bridge.evidence().size(),1);
  const auto evidenceId=bridge.evidence()[0].toMap().value("id").toString();QVERIFY(!evidenceId.isEmpty());
  QCOMPARE(bridge.previewEvidence(0),QString::fromUtf8(original));QCOMPARE(bridge.previewEvidence(-1),QString());
  QCOMPARE(bridge.evidenceCustodian(0),QString("Lab technician"));
  QVERIFY(!bridge.transferCustody(0,"Evidence room","Review",false));
  QVERIFY(!bridge.transferCustody(0,"Lab technician","Review",true));
  QVERIFY(bridge.transferCustody(0,"Evidence room","Authorized review",true));
  QCOMPARE(bridge.evidenceCustodian(0),QString("Evidence room"));
  QVERIFY(bridge.verifyEvidence());QTRY_VERIFY(!bridge.busy());QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));
  auto timeline=bridge.timeline();QVERIFY(timeline.size()>=6);
  for(qsizetype i=1;i<timeline.size();++i)QVERIFY(timeline[i-1].toMap().value("atUtc").toString()<=timeline[i].toMap().value("atUtc").toString());
  const auto html=temp.filePath("report.html"),pdf=temp.filePath("report.pdf"),events=temp.filePath("timeline.json");
  QVERIFY(!bridge.exportFormattedReport(html,"html",false,false));QVERIFY(!QFile::exists(html));
  QVERIFY(!bridge.exportFormattedReport(source,"html",false,true));QCOMPARE(read(source),original);
  QVERIFY(bridge.exportFormattedReport(html,"html",false,true));
  const auto report=read(html);QVERIFY(report.contains("&lt;script&gt;"));QVERIFY(!report.contains("<script>"));QVERIFY(!report.contains("onerror=alert"));
  QVERIFY(report.contains(evidenceId.toUtf8()));QVERIFY(report.contains("custody-transfer"));QVERIFY(report.contains("report-export"));QVERIFY(!report.contains("payload"));
  QVERIFY(!bridge.exportFormattedReport(html,"html",false,true));QCOMPARE(read(html),report);
  QVERIFY2(bridge.exportFormattedReport(pdf,"pdf",true,true),qPrintable(bridge.error()));QVERIFY(read(pdf).startsWith("%PDF-"));QVERIFY(read(pdf).size()>1000);
  QVERIFY(bridge.exportTimeline(events,true));const auto eventDoc=QJsonDocument::fromJson(read(events)).object();QCOMPARE(eventDoc.value("caseId").toString(),id);QVERIFY(eventDoc.value("events").toArray().size()>=timeline.size());
  const auto saved=bridge.casePath();QVERIFY(bridge.closeCase());QVERIFY(bridge.openCase(saved));QTRY_VERIFY(!bridge.busy());QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));
  QCOMPARE(bridge.caseId(),id);QCOMPARE(bridge.caseOperator(),QString("Lab technician"));QCOMPARE(bridge.evidence()[0].toMap().value("id").toString(),evidenceId);QCOMPARE(bridge.evidenceCustodian(0),QString("Evidence room"));
 }
 void reopeningNeverAttributesLegacyEvidenceToAnotherCaseOperator(){
  QTemporaryDir temp;RecoveryBridge bridge(temp.filePath("cases"),true);QVERIFY(create(bridge));
  const auto source=temp.filePath("source.txt");QVERIFY(write(source,"Synthetic unknown collector"));
  QVERIFY(bridge.importVisibleFile(source,true));QTRY_VERIFY(!bridge.busy());const auto original=bridge.casePath();
  QVERIFY(bridge.recordOperator("Later operator","LAB-ONLY",true));
  QVERIFY(bridge.importVisibleFile(source,true));QTRY_VERIFY(!bridge.busy());
  QVERIFY(bridge.evidence()[0].toMap().value("collectedBy").toString().isEmpty());QCOMPARE(bridge.evidence()[1].toMap().value("collectedBy").toString(),QString("Later operator"));
  QVERIFY(create(bridge));QVERIFY(bridge.recordOperator("Another case operator","LAB-2",true));
  QVERIFY(bridge.openCase(original));QTRY_VERIFY(!bridge.busy());QVERIFY(bridge.error().isEmpty());
  QVERIFY(bridge.evidence()[0].toMap().value("collectedBy").toString().isEmpty());QCOMPARE(bridge.evidence()[1].toMap().value("collectedBy").toString(),QString("Later operator"));
 }
 void modifiedSealedAuditIsRefusedWithoutReplacingOpenCase(){
  QTemporaryDir temp;RecoveryBridge bridge(temp.filePath("cases"),true);QVERIFY(create(bridge));const auto original=bridge.casePath();QString error;
  auto doc=QJsonDocument::fromJson(mr::recovery::openCaseBytes(read(original),true,error)).object();auto audit=doc.value("audit").toArray();auto event=audit[0].toObject();event.insert("operator","forged");audit[0]=event;doc.insert("audit",audit);
  const auto path=temp.filePath("tampered.nmrcase");QVERIFY(write(path,mr::recovery::sealCase(QJsonDocument(doc).toJson(),true,error)));const auto before=read(path);
  QVERIFY(bridge.openCase(path));QTRY_VERIFY(!bridge.busy());QVERIFY(!bridge.error().isEmpty());QCOMPARE(bridge.casePath(),original);QCOMPARE(read(path),before);
 }
};
QTEST_MAIN(ForensicTests)
#include "forensic_tests.moc"
