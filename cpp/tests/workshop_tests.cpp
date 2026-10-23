#include "WorkshopBridge.hpp"
#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QSqlQuery>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
class WorkshopTests:public QObject {
 Q_OBJECT
private slots:
 void lifecycle() {
  QTemporaryDir temp; QVERIFY(temp.isValid()); const auto path=temp.filePath("repairs.db");
  { WorkshopBridge w(path);QVERIFY2(w.ready(),qPrintable(w.error()));QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
    QCOMPARE(w.createOrder(" ","","Samsung","A52","","Bootloop","",""),-1);
    const int id=w.createOrder(QStringLiteral("محمد"),"0101","Samsung","A52","safe-serial","Bootloop","Ali","<script>literal</script>");QVERIFY(id>0);QCOMPARE(w.orders().size(),1);
    QVERIFY(w.updateOrder(id,"Diagnosing","Ali",1250,"preserve data")); QVERIFY(!w.updateOrder(id,"invented","",0,"")); QVERIFY(!w.updateOrder(id,"Ready","",-1,""));
    QVERIFY(w.addPart("USB-1","Connector",2,350));QVERIFY(!w.addPart("USB-1","Duplicate",1,1));
    const int part=w.inventory().first().toMap().value("id").toInt();
    QVERIFY(!w.usePart(id,part,3));QCOMPARE(w.inventory().first().toMap().value("quantity").toInt(),2);
    QVERIFY(!w.usePart(999,part,1));QCOMPARE(w.inventory().first().toMap().value("quantity").toInt(),2);
    QVERIFY(w.usePart(id,part,2));QCOMPARE(w.invoice(id).value("total_cents").toInt(),1950);QCOMPARE(w.inventory().first().toMap().value("quantity").toInt(),0);
    QVERIFY(!w.exportOrders(path));QVERIFY(!w.exportInvoice(id,path));QVERIFY(!w.exportOrders(path+"-wal"));QVERIFY(!w.exportInvoice(id,QUrl::fromLocalFile(path).toString()));
    QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));QCOMPARE(w.orders().size(),1);
    QVERIFY(w.exportOrders(temp.filePath("orders.csv")));QVERIFY(w.exportInvoice(id,temp.filePath("invoice.pdf")));QFile pdf(temp.filePath("invoice.pdf"));QVERIFY(pdf.open(QIODevice::ReadOnly));const auto pdfBytes=pdf.readAll();QVERIFY(pdfBytes.startsWith("%PDF"));QVERIFY(pdfBytes.trimmed().endsWith("%%EOF"));pdf.close();
    QVERIFY(w.backup(temp.filePath("backup.db")));QVERIFY(!w.backup(temp.filePath("backup.db")));QVERIFY(!w.restore(temp.filePath("backup.db"),false));
    QVERIFY(w.createOrder("Other","","Apple","iPhone","","USB","","")>id);QCOMPARE(w.orders().size(),2);
    QVERIFY2(w.restore(temp.filePath("backup.db"),true),qPrintable(w.error()));QCOMPARE(w.orders().size(),1);QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
    QFile bad(temp.filePath("bad.db"));QVERIFY(bad.open(QIODevice::WriteOnly));bad.write("not a database");bad.close();QVERIFY(!w.restore(bad.fileName(),true));QCOMPARE(w.orders().size(),1);
  }
  {WorkshopBridge reopened(path);QVERIFY(reopened.ready());QCOMPARE(reopened.orders().size(),1);QCOMPARE(reopened.events().size(),4);}
 }
 void inputAndExport() {
  QTemporaryDir temp;WorkshopBridge w(temp.filePath("db"));QVERIFY(w.ready());
  const QString attack="=cmd|' /C calc'!A0";const int id=w.createOrder(attack,"","X'; DROP TABLE orders;--","M","","USB","",QStringLiteral("line 1\nline 2"));QVERIFY(id>0);QCOMPARE(w.orders().size(),1);QVERIFY(w.exportOrders(temp.filePath("safe.csv")));QFile f(temp.filePath("safe.csv"));QVERIFY(f.open(QIODevice::ReadOnly));QVERIFY(f.readAll().contains("'=cmd"));QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
 }
 void restockKeepsHistoricalPrice(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("stock.db"));QVERIFY(w.ready());
  const int order=w.createOrder("A","","B","M","","S","","");QVERIFY(order>0);QVERIFY(w.addPart("P","Connector",1,100));
  const int part=w.inventory().first().toMap().value("id").toInt();QVERIFY(w.usePart(order,part,1));QCOMPARE(w.invoice(order).value("total_cents").toInt(),100);
  QVERIFY(w.restockPart(part,3,250));QCOMPARE(w.inventory().first().toMap().value("quantity").toInt(),3);QCOMPARE(w.invoice(order).value("total_cents").toInt(),100);
  QVERIFY(!w.restockPart(part,1000000,250));QVERIFY(!w.restockPart(999,1,250));QCOMPARE(w.inventory().first().toMap().value("quantity").toInt(),3);QVERIFY(w.usePart(order,part,1));QCOMPARE(w.invoice(order).value("total_cents").toInt(),350);
 }
 void optionalNullStrings(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("optional.db"));QVERIFY(w.ready());
  const int id=w.createOrder("Customer",QString(),"Brand","Model",QString(),"Symptom",QString(),QString());QVERIFY2(id>0,qPrintable(w.error()));
  QVERIFY(w.updateOrder(id,"Diagnosing",QString(),0,QString()));QCOMPARE(w.orders().first().toMap().value("phone").toString(),QStringLiteral(""));
 }
 void diagnosticAudit(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("audit.db"));QVERIFY(w.ready());
  QVERIFY(!w.recordDiagnostic("arbitrary command",true));QVERIFY(w.recordDiagnostic("adb-scan",true));QCOMPARE(w.events().size(),1);QVERIFY(w.events().first().toMap().value("action").toString().contains("adb-scan"));
 }
 void diagnosisPersistenceAndReport(){
  QTemporaryDir t;const auto path=t.filePath("diagnosis.db");int order=-1;
  const QString finding=QStringLiteral("USB data pin damaged; <script> is literal, not markup.\nManual test only.");
  {WorkshopBridge w(path);QVERIFY(w.ready());order=w.createOrder("Customer","","Brand","Model","","USB not detected","Tech","");QVERIFY(order>0);
   const int other=w.createOrder("Other","","Brand","Other","","Bootloop","Tech","");QVERIFY(other>0);
   QVERIFY(!w.saveDiagnosis(999,"USB","Guidance","Finding","","Tech"));
   QVERIFY(!w.saveDiagnosis(order,"USB","Guidance"," ","","Tech"));
   QVERIFY(!w.saveDiagnosis(order,"USB","Guidance","Finding","",""));
   QVERIFY(!w.saveDiagnosis(order,"USB",QString(8001,'x'),"Finding","","Tech"));
   QVERIFY(!w.saveDiagnosis(order,"USB","Guidance","Finding",QString(4001,'x'),"Tech"));
   QVERIFY(!w.saveDiagnosis(order,"USB","Guidance",QString("Hidden")+QChar::Null+"text","","Tech"));
   QVERIFY(w.diagnoses(order).isEmpty());
   QVERIFY(w.saveDiagnosis(order,"USB not detected","Rule guidance; not confirmed diagnosis",finding,"Known-good cable tested manually","Tech"));
   QCOMPARE(w.diagnoses(order).size(),1);QVERIFY(w.diagnoses(other).isEmpty());
   QCOMPARE(w.invoice(order).value("diagnoses").toList().size(),1);
   QVERIFY(w.events().first().toMap().value("action").toString().contains("Diagnostic findings saved"));
   QVERIFY(!w.exportDiagnosisReport(order,path));QVERIFY(!w.exportDiagnosisReport(order,path+"-wal"));QVERIFY(!w.exportDiagnosisReport(other,t.filePath("empty.json")));
   QVERIFY(!w.exportDiagnosisReport(order,"https://example.invalid/report.json"));
   const auto report=t.filePath("diagnosis-report.json");QVERIFY(w.exportDiagnosisReport(order,QUrl::fromLocalFile(report).toString()));
   QFile file(report);QVERIFY(file.open(QIODevice::ReadOnly));const auto document=QJsonDocument::fromJson(file.readAll());QVERIFY(document.isObject());
   const auto object=document.object();QCOMPARE(object.value("reportType").toString(),QStringLiteral("nexvary-mr-diagnostic-findings"));
   const auto saved=object.value("order").toObject().value("diagnoses").toArray().first().toObject();QCOMPARE(saved.value("findings").toString(),finding);QCOMPARE(saved.value("evidence").toString(),QStringLiteral("Known-good cable tested manually"));
   QVERIFY(!object.contains("logcat"));QVERIFY(w.exportInvoice(order,t.filePath("diagnosis-invoice.pdf")));
   QVERIFY(w.backup(t.filePath("saved.db")));QVERIFY(w.saveDiagnosis(order,"USB","Guidance","Second finding","","Tech"));QCOMPARE(w.diagnoses(order).size(),2);
   QVERIFY(w.restore(t.filePath("saved.db"),true));QCOMPARE(w.diagnoses(order).size(),1);QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
  }
  WorkshopBridge reopened(path);QVERIFY(reopened.ready());QCOMPARE(reopened.diagnoses(order).first().toMap().value("findings").toString(),finding);
 }
 void diagnosisEntryLimit(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("bounded.db"));QVERIFY(w.ready());const int order=w.createOrder("C","","B","M","","S","T","");QVERIFY(order>0);
  for(int i=0;i<100;++i)QVERIFY(w.saveDiagnosis(order,"S","Guidance",QStringLiteral("Finding %1").arg(i),QString(),"T"));
  QVERIFY(!w.saveDiagnosis(order,"S","Guidance","Excess entry","","T"));QCOMPARE(w.diagnoses(order).size(),100);QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
 }
 void migratesAndRestoresLegacyDiagnosisSchema(){
  QTemporaryDir t;const auto legacy=t.filePath("legacy-v1.db");int order=-1;
  {WorkshopBridge w(legacy);QVERIFY(w.ready());order=w.createOrder("Preserved","","Brand","Model","","USB","Tech","Old notes");QVERIFY(order>0);}
  const QString connection="diagnosis-legacy";
  {auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(legacy);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("DROP TABLE order_labels"));QVERIFY(q.exec("DROP TABLE tracking_links"));QVERIFY(q.exec("DROP TABLE diagnoses"));QVERIFY(q.exec("PRAGMA user_version=1"));db.close();}QSqlDatabase::removeDatabase(connection);
  const auto backup=t.filePath("legacy-backup.db");QVERIFY(QFile::copy(legacy,backup));
  {WorkshopBridge migrated(legacy);QVERIFY2(migrated.ready(),qPrintable(migrated.error()));QCOMPARE(migrated.orders().size(),1);QCOMPARE(migrated.orders().first().toMap().value("notes").toString(),QStringLiteral("Old notes"));QVERIFY(migrated.saveDiagnosis(order,"USB","Guidance","New finding","","Tech"));QCOMPARE(migrated.checkIntegrity(),QStringLiteral("ok"));}
  WorkshopBridge active(t.filePath("active.db"));QVERIFY(active.ready());QVERIFY(active.restore(backup,true));QCOMPARE(active.orders().size(),1);QVERIFY(active.diagnoses(order).isEmpty());QVERIFY(active.saveDiagnosis(order,"USB","Guidance","After legacy restore","","Tech"));QCOMPARE(active.checkIntegrity(),QStringLiteral("ok"));
 }
 void newerSchema() {
  QTemporaryDir t;const QString path=t.filePath("new.db");const QString c="future-test";
  {auto db=QSqlDatabase::addDatabase("QSQLITE",c);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("PRAGMA user_version=99"));db.close();}QSqlDatabase::removeDatabase(c);
  WorkshopBridge w(path);QVERIFY(!w.ready());QVERIFY(!w.error().isEmpty());
 }
 void rejectsExecutableBackupSchema() {
  QTemporaryDir t;WorkshopBridge w(t.filePath("active.db"));QVERIFY(w.ready());
  const int id=w.createOrder("Preserve","","Example","Synthetic","","USB","","");QVERIFY(id>0);
  const auto backup=t.filePath("malicious.db");QVERIFY(w.backup(backup));
  const QString c="hostile-backup";
  {auto db=QSqlDatabase::addDatabase("QSQLITE",c);db.setDatabaseName(backup);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("CREATE TRIGGER hostile AFTER INSERT ON customers BEGIN DELETE FROM orders; END"));db.close();}QSqlDatabase::removeDatabase(c);
  QVERIFY(!w.restore(backup,true));QCOMPARE(w.orders().size(),1);QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
  QVERIFY(!w.exportOrders("https://example.invalid/file.csv"));
  const QString nullText=QString("note")+QChar::Null+QString("hidden");
  QCOMPARE(w.createOrder("Customer",nullText,"Example","Synthetic","","USB","",""),-1);
  QVERIFY(!w.updateOrder(id,"Ready","Tech",0,nullText));
 }
 void rejectsRelaxedBackupSchema() {
  QTemporaryDir t;WorkshopBridge w(t.filePath("active.db"));QVERIFY(w.ready());const auto path=t.filePath("relaxed.db");QVERIFY(w.backup(path));
  const QString c="relaxed-backup";
  {auto db=QSqlDatabase::addDatabase("QSQLITE",c);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("ALTER TABLE parts RENAME TO old_parts"));QVERIFY(q.exec("CREATE TABLE parts(id INTEGER PRIMARY KEY,sku TEXT,name TEXT,quantity INTEGER,unit_cents INTEGER)"));QVERIFY(q.exec("DROP TABLE old_parts"));db.close();}QSqlDatabase::removeDatabase(c);
  QVERIFY(!w.restore(path,true));QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
 }
 void invoiceIntegerBoundary() {
  QTemporaryDir t;const auto path=t.filePath("large-totals.db");WorkshopBridge w(path);QVERIFY(w.ready());
  const int order=w.createOrder("Arithmetic stress test","","Synthetic","Training","","USB","","");QVERIFY(order>0);
  QVERIFY(w.addPart("BOUNDARY","Synthetic high-value test part",1,100000000));const auto part=w.inventory().first().toMap().value("id").toInt();
  const QString c="large-invoice-test";
  {auto db=QSqlDatabase::addDatabase("QSQLITE",c);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
   q.prepare("WITH RECURSIVE n(x) AS (VALUES(1) UNION ALL SELECT x+1 FROM n WHERE x<92233) INSERT INTO order_parts(order_id,part_id,quantity,unit_cents) SELECT ?,?,1000000,100000000 FROM n");q.addBindValue(order);q.addBindValue(part);QVERIFY(q.exec());
   q.prepare("INSERT INTO order_parts(order_id,part_id,quantity,unit_cents) VALUES(?,?,720368,100000000)");q.addBindValue(order);q.addBindValue(part);QVERIFY(q.exec());db.close();}QSqlDatabase::removeDatabase(c);
  QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
  const qint64 before=w.invoice(order).value("total_cents").toLongLong();QCOMPARE(before,Q_INT64_C(9223372036800000000));QCOMPARE(w.invoice(order).value("total_display").toString(),QStringLiteral("92233720368000000.00"));
  QVERIFY(!w.usePart(order,part,1));QCOMPARE(w.inventory().first().toMap().value("quantity").toInt(),1);
  QVERIFY(!w.updateOrder(order,"Ready","",100000000,""));QCOMPARE(w.invoice(order).value("total_cents").toLongLong(),before);
 }
};
QTEST_MAIN(WorkshopTests)
#include "workshop_tests.moc"

