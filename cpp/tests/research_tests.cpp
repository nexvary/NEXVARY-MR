#include "ResearchBridge.hpp"
#include "ToolBridge.hpp"
#include "FirmwareBridge.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
namespace {
QVariantMap result(){return {{"experimentId","EXP-002"},{"question","Synthetic envelope?"},{"hypothesis","Bounded parser"},{"model","Synthetic fixture"},{"osBuild","No OS"},{"firmware","Synthetic vbmeta"},{"environment","Isolated test"},{"equipment","Workstation"},{"scope","Synthetic data only"},{"methodology","Compare known fixture"},{"expected","Known envelope"},{"actual","Known envelope observed"},{"reproducibility","Successful"},{"researcher","Synthetic test runner"}};}
QVariantMap matrix(){return {{"manufacturer","Synthetic"},{"model","Fixture"},{"chipset","Not a phone"},{"osBuild","Synthetic"},{"firmware","Synthetic"},{"securityPatch","Not applicable"},{"connectionMode","Offline"},{"capability","Parser envelope"},{"unsupported","Device writes"},{"sourceId","aosp.avb"},{"lastTestUtc","not tested"},{"result","not tested"},{"confidence","Research only"},{"status","Research Hypothesis"}};}
}
class ResearchTests:public QObject {
 Q_OBJECT
private slots:
 void catalogHasSourcesAndSearch() {
  QTemporaryDir t;ResearchBridge b(t.filePath("ledger.json"));QVERIFY2(b.ready(),qPrintable(b.error()));QVERIFY(b.search("atlas","FBE").size()>0);QVERIFY(b.search("tools","Cellebrite").size()>=2);QCOMPARE(b.search("tools","impossible").size(),0);QCOMPARE(b.catalog().value("tools").toList().size(),10);QVERIFY(!b.engineContext().value("executionApproved").toBool());QVERIFY(b.results().isEmpty());QVERIFY(b.compatibility().isEmpty());
 }
 void untrustedCatalogCannotPromoteClaims() {
  QTemporaryDir t;ResearchBridge b(t.filePath("ledger.json"));auto c=b.catalog();auto rows=c.value("atlas").toList();auto r=rows.first().toMap();r.insert("status","Lab Verified");rows[0]=r;c.insert("atlas",rows);QVERIFY(!ResearchBridge::validateCatalog(c).isEmpty());c=b.catalog();rows=c.value("tools").toList();r=rows.first().toMap();r.insert("sourceIds",QStringList{"missing"});rows[0]=r;c.insert("tools",rows);QVERIFY(!ResearchBridge::validateCatalog(c).isEmpty());
 }
 void resultPersistenceAndPromotionDenial() {
  QTemporaryDir t;QFile f(t.filePath("evidence"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("PUBLIC SYNTHETIC RESULT");f.close();QString id;{
   ResearchBridge b(t.filePath("ledger.json"));QVERIFY(b.recordResult(result(),f.fileName(),false).isEmpty());auto r=result();r.insert("commercialSupported","true");QVERIFY(b.recordResult(r,f.fileName(),true).isEmpty());id=b.recordResult(result(),f.fileName(),true);QVERIFY2(!id.isEmpty(),qPrintable(b.error()));QVERIFY(!b.validation(id).value("commercialSupported").toBool());QCOMPARE(b.validation(id).value("review").toString(),QString("Pending independent review"));
   auto m=matrix();m.insert("status","Lab Verified");QVERIFY(!b.addCompatibility(m));QVERIFY(b.addCompatibility(matrix()));QVERIFY(b.exportReport(t.filePath("report.json"),true));QVERIFY(!b.exportReport(t.filePath("report.json"),true));
  }
  ResearchBridge b(t.filePath("ledger.json"));QVERIFY(b.ready());QCOMPARE(b.results().size(),1);QCOMPARE(b.compatibility().size(),1);QVERIFY(!b.validation(id).value("commercialSupported").toBool());
 }
 void invalidResultsAndCorruptionFailClosed() {
  QTemporaryDir t;QFile f(t.filePath("evidence"));QVERIFY(f.open(QIODevice::WriteOnly));f.write("SYNTHETIC");f.close();ResearchBridge b(t.filePath("ledger.json"));auto r=result();r.remove("model");QVERIFY(b.recordResult(r,f.fileName(),true).isEmpty());r=result();r.insert("experimentId","unknown");QVERIFY(b.recordResult(r,f.fileName(),true).isEmpty());r=result();r.insert("reproducibility","Lab Verified");QVERIFY(b.recordResult(r,f.fileName(),true).isEmpty());QVERIFY(!b.recordResult(result(),f.fileName(),true).isEmpty());
  QFile ledger(t.filePath("ledger.json"));QVERIFY(ledger.open(QIODevice::WriteOnly|QIODevice::Truncate));ledger.write("{corrupt");ledger.close();ResearchBridge reopened(ledger.fileName());QVERIFY(!reopened.ready());QVERIFY(reopened.recordResult(result(),f.fileName(),true).isEmpty());
 }
 void concurrentLedgerPreservesBothWriters() {
  QTemporaryDir t;ResearchBridge a(t.filePath("ledger.json")),b(t.filePath("ledger.json"));QVERIFY(a.addCompatibility(matrix()));QVERIFY(!b.addCompatibility(matrix()));QCOMPARE(b.compatibility().size(),1);QVERIFY(b.addCompatibility(matrix()));ResearchBridge c(t.filePath("ledger.json"));QCOMPARE(c.compatibility().size(),2);
 }
 void recomputedChecksumCannotPromoteResearch() {
  QTemporaryDir t;const auto path=t.filePath("ledger.json");ResearchBridge b(path);QVERIFY(b.addCompatibility(matrix()));QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));auto data=QJsonDocument::fromJson(file.readAll()).object().toVariantMap();file.close();data.remove("sha256");auto rows=data.value("compatibility").toList();auto row=rows.first().toMap();row.insert("status","Lab Verified");row.insert("commercialSupported",true);rows[0]=row;data.insert("compatibility",rows);const auto bytes=QJsonDocument(QJsonObject::fromVariantMap(data)).toJson(QJsonDocument::Compact);data.insert("sha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()));QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write(QJsonDocument(QJsonObject::fromVariantMap(data)).toJson(QJsonDocument::Compact));file.close();ResearchBridge reopened(path);QVERIFY(!reopened.ready());QVERIFY(!reopened.addCompatibility(matrix()));
 }
 void sharedContextDoesNotCertifyDevice() {
  QTemporaryDir t;ToolBridge tools;FirmwareBridge firmware;ResearchBridge b(t.filePath("ledger.json"),&tools,&firmware);const auto context=b.engineContext();QVERIFY(context.contains("usbDiagnosis"));QVERIFY(context.contains("firmwareAvb"));QVERIFY(context.contains("deviceIntelligence"));QCOMPARE(context.value("modelAcceptance").toString(),QString("not-established"));QVERIFY(!context.value("executionApproved").toBool());
 }
};
QTEST_MAIN(ResearchTests)
#include "research_tests.moc"
