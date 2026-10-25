#include "ImportAdapters.hpp"
#include "EvidenceImportRunner.hpp"
#include "LawCaseBridge.hpp"
#include "IdentityService.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdio>
namespace {
QVariantMap meta(){return {{"caseNumber","SYNTHETIC-IMPORT"},{"investigatingAgency","Synthetic agency"},{"investigatorName","Synthetic operator"},{"seizedDeviceDescription","Synthetic file"},{"authorityReference","Synthetic scope"},{"issuingAuthority","Lab owner"},{"scope","Synthetic import fixtures only"}};}
QString hash(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
bool write(const QString &p,const QByteArray &b){QFile f(p);return f.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&f.write(b)==b.size();}
QByteArray read(const QString &p){QFile f(p);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
QString evidence(LawCaseBridge &b,const QString &path,const QByteArray &bytes){const auto d=b.addDevice({{"model","Synthetic source"},{"custodian","Synthetic custodian"}});if(!write(path,bytes))return {};return b.addEvidence(d,path,"Synthetic authorized source","Synthetic custodian",true);}
bool imported(LawCaseBridge &b,const QString &e,const QString &adapter,bool expected=true){QSignalSpy done(&b,&LawCaseBridge::importCompleted);if(!b.importEvidence(e,adapter,"Synthetic reviewed import",true))return false;if(done.isEmpty()&&!done.wait(8000))return false;return done.last()[0].toBool()==expected;}
}
class ImportTests:public QObject {
 Q_OBJECT
private slots:
 void supportedFormats_data(){
  QTest::addColumn<QString>("adapter");QTest::addColumn<QByteArray>("bytes");
  QTest::newRow("text-arabic")<<QString("utf8-document")<<QString("دليل اصطناعي\nSynthetic document").toUtf8();
  QTest::newRow("json")<<QString("json")<<QByteArray("{\"z\":null,\"a\":[true,3,\"Synthetic\"]}");
  QTest::newRow("xml")<<QString("xml")<<QByteArray("<case id=\"SYNTHETIC\"><item>Observed</item></case>");
  QTest::newRow("csv")<<QString("csv")<<QByteArray("id,note\r\n1,\"Synthetic, line\nwith \"\"quotes\"\"\"\r\n");
  QTest::newRow("android")<<QString("android-log")<<QByteArray("--------- beginning of main\n10-25 10:00:00.000  1  2 I Synthetic: fixture\n");
 }
 void supportedFormats(){
  QFETCH(QString,adapter);QFETCH(QByteArray,bytes);QString error;
  const auto first=mr::forensic::parseAuthorizedImport(bytes,adapter,{},error);QVERIFY2(!first.isEmpty(),qPrintable(error));
  QCOMPARE(first,mr::forensic::parseAuthorizedImport(bytes,adapter,{},error));QCOMPARE(first.value("sourceSha256").toString(),hash(bytes));QVERIFY(!first.value("findings").toList().isEmpty());
  EvidenceImportRunner runner;QSignalSpy done(&runner,&EvidenceImportRunner::completed);QVERIFY(runner.start(bytes,hash(bytes),adapter));QVERIFY(done.wait(8000));QVERIFY2(done[0][1].toString().isEmpty(),qPrintable(done[0][1].toString()));QCOMPARE(done[0][0].toMap(),first);QVERIFY(!runner.busy());
 }
 void malformed_data(){
  QTest::addColumn<QString>("adapter");QTest::addColumn<QByteArray>("bytes");
  QTest::newRow("json-syntax")<<QString("json")<<QByteArray("{broken");
  QTest::newRow("duplicate-key")<<QString("json")<<QByteArray("{\"x\":1,\"\\u0078\":2}");
  QTest::newRow("json-depth")<<QString("json")<<(QByteArray(33,'[')+QByteArray(33,']'));
  QTest::newRow("xml-dtd")<<QString("xml")<<QByteArray("<!DOCTYPE a [<!ENTITY x SYSTEM 'file:///never-read'>]><a>&x;</a>");
  QTest::newRow("xml-depth")<<QString("xml")<<(QByteArray("<a>").repeated(33)+QByteArray("</a>").repeated(33));
  QTest::newRow("xml-broken")<<QString("xml")<<QByteArray("<a>");
  QTest::newRow("xml-other-encoding")<<QString("xml")<<QByteArray("<?xml version='1.0' encoding='ISO-8859-1'?><a>text</a>");
  QTest::newRow("csv-open-quote")<<QString("csv")<<QByteArray("x,\"unfinished");
  QTest::newRow("csv-trailing")<<QString("csv")<<QByteArray("\"x\"garbage");
  QTest::newRow("csv-columns")<<QString("csv")<<QByteArray(129,',');
  QTest::newRow("binary-document")<<QString("utf8-document")<<QByteArray("%PDF-1.7\0binary",15);
  QTest::newRow("invalid-utf8")<<QString("json")<<QByteArray::fromHex("7b2278223a22c080227d");
  QTest::newRow("log-unknown-format")<<QString("android-log")<<QByteArray("unreviewed log format");
  QTest::newRow("encrypted-sqlite")<<QString("sqlite")<<QByteArray("encrypted bytes");
 }
 void malformed(){QFETCH(QString,adapter);QFETCH(QByteArray,bytes);QString error;QVERIFY(mr::forensic::parseAuthorizedImport(bytes,adapter,{},error).isEmpty());QVERIFY(!error.isEmpty());}
 void limitsAndSafePresentation(){
  QString error;QVERIFY(mr::forensic::parseAuthorizedImport(QByteArray(mr::forensic::importInputLimit+1,'x'),"utf8-document",{},error).isEmpty());
  QByteArray rows;for(int i=0;i<150;++i)rows+="SYNTHETIC\n";
  const auto report=mr::forensic::parseAuthorizedImport(rows,"utf8-document",{},error);QCOMPARE(report.value("findings").toList().size(),128);QVERIFY(report.value("findingsTruncated").toBool());
  const auto bidi=mr::forensic::parseAuthorizedImport(QString("Synthetic "+QString(QChar(0x202e))+"hidden").toUtf8(),"utf8-document",{},error);QVERIFY(bidi.value("findings").toList()[0].toMap().value("value").toString().contains("\\u202e"));
  EvidenceImportRunner r;QVERIFY(!r.start("synthetic",QString(64,'0'),"json"));QVERIFY(!r.start("synthetic",hash("synthetic"),"invented"));
 }
 void standaloneSqlite(){
  QTemporaryDir t;const auto path=t.filePath("source.sqlite");
  {auto db=QSqlDatabase::addDatabase("QSQLITE","import-fixture");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("CREATE TABLE facts(note TEXT, payload BLOB)"));QVERIFY(q.exec("INSERT INTO facts VALUES('SYNTHETIC',x'010203')"));QVERIFY(q.exec("CREATE VIEW never_execute AS SELECT load_extension('never')"));db.close();}QSqlDatabase::removeDatabase("import-fixture");
  const auto bytes=read(path);EvidenceImportRunner r;QSignalSpy done(&r,&EvidenceImportRunner::completed);QVERIFY(r.start(bytes,hash(bytes),"sqlite"));QVERIFY(done.wait(8000));QVERIFY(done[0][1].toString().isEmpty());const auto report=done[0][0].toMap();QVERIFY(QJsonDocument::fromVariant(report).toJson().contains("SYNTHETIC"));QVERIFY(QJsonDocument::fromVariant(report).toJson().contains("sqlite-skipped"));QCOMPARE(read(path),bytes);
  auto wal=bytes;wal[18]=2;wal[19]=2;QString error;QVERIFY(mr::forensic::parseAuthorizedImport(wal,"sqlite",path,error).isEmpty());
 }
 void isolatedFailures_data(){QTest::addColumn<QByteArray>("mode");for(const auto &m:{"hang","flood","failure","forged","changed-copy"})QTest::newRow(m)<<QByteArray(m);}
 void isolatedFailures(){
  QFETCH(QByteArray,mode);QTemporaryDir t;const auto marker=t.filePath("child-working-path");qputenv("MR_SYNTHETIC_IMPORT_CHILD",mode);qputenv("MR_SYNTHETIC_IMPORT_CHILD_MARKER",marker.toUtf8());EvidenceImportRunner r(nullptr,QCoreApplication::applicationFilePath());QSignalSpy done(&r,&EvidenceImportRunner::completed);QVERIFY(r.start("{}",hash("{}"),"json"));QVERIFY(done.wait(8000));QVERIFY(done[0][0].toMap().isEmpty());QVERIFY(!done[0][1].toString().isEmpty());QVERIFY(!r.busy());const auto path=QString::fromUtf8(read(marker));QVERIFY(!path.isEmpty());QVERIFY(!QFileInfo::exists(path));QVERIFY(!QDir(QFileInfo(path).absolutePath()).exists());if(mode=="changed-copy")QCOMPARE(done[0][1].toString(),QString("working-copy-integrity"));qunsetenv("MR_SYNTHETIC_IMPORT_CHILD");qunsetenv("MR_SYNTHETIC_IMPORT_CHILD_MARKER");
 }
 void caseProvenanceBackupAndLegacyRecord(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true);const auto id=b.createCase(meta());QVERIFY(!id.isEmpty());const auto e=evidence(b,t.filePath("original.json"),"{\"synthetic\":42}");QVERIFY(!e.isEmpty());const auto original=read(t.filePath("original.json"));
  {LawCaseBridge legacyBeforeImport(path,nullptr,true);QVERIFY(legacyBeforeImport.openCase(id));QVERIFY(!legacyBeforeImport.activeCase().contains("analyses"));QCOMPARE(legacyBeforeImport.activeCase(),b.activeCase());}
  QVERIFY(!b.importEvidence(e,"json","Synthetic",false));QVERIFY(!b.importEvidence("other-case","json","Synthetic",true));QVERIFY(imported(b,e,"json"));
  const auto first=b.importResult();QCOMPARE(b.audit().last().toMap().value("event").toString(),QString("evidence-imported"));QVERIFY(imported(b,e,"json"));QCOMPARE(first.value("reportSha256"),b.importResult().value("reportSha256"));QCOMPARE(read(t.filePath("original.json")),original);QVERIFY(b.verifyEvidence(e));QVERIFY(b.verifyAudit());
  const auto backup=b.exportProtectedBackup(t.filePath("backup"),true);QVERIFY(!backup.isEmpty());LawCaseBridge restored(t.filePath("restored.db"),nullptr,true);QCOMPARE(restored.restoreProtectedBackup(t.filePath("backup"),backup.value("sha256").toString(),true),id);QCOMPARE(restored.activeCase().value("analyses"),b.activeCase().value("analyses"));QVERIFY(restored.verifyAudit());
  LawCaseBridge legacy(path,nullptr,true);QVERIFY(legacy.openCase(id));QCOMPARE(legacy.activeCase().value("schema").toInt(),1);QVERIFY(legacy.verifyEvidence(e));
 }
 void oversizedAndSuspended(){
  QTemporaryDir t;LawCaseBridge b(t.filePath("cases.db"),nullptr,true);QVERIFY(!b.createCase(meta()).isEmpty());const auto e=evidence(b,t.filePath("large.txt"),QByteArray(mr::forensic::importInputLimit+1,'x'));QVERIFY(!e.isEmpty());const auto revision=b.activeCase().value("revision");QVERIFY(!b.importEvidence(e,"utf8-document","Synthetic",true));QCOMPARE(b.activeCase().value("revision"),revision);QVERIFY(b.verifyEvidence(e));QVERIFY(b.transitionCase("suspended"));QVERIFY(!b.importEvidence(e,"utf8-document","Synthetic",true));QVERIFY(!b.importBusy());QVERIFY(b.verifyAudit());
 }
 void rejectionAuditPermissionsAndCancellation(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true);const auto id=b.createCase(meta());const auto e=evidence(b,t.filePath("bad.json"),"not json");QVERIFY(imported(b,e,"json",false));QCOMPARE(b.audit().last().toMap().value("event").toString(),QString("evidence-import-rejected"));QVERIFY(b.verifyEvidence(e));QVERIFY(b.verifyAudit());
  LawCaseBridge auditor(path,nullptr,true,nexvary::law::Role::Auditor);QVERIFY(auditor.openCase(id));QVERIFY(!auditor.importEvidence(e,"json","Synthetic",true));
  QSignalSpy done(&b,&LawCaseBridge::importCompleted);QVERIFY(b.importEvidence(e,"json","Synthetic",true));b.cancelImport();QVERIFY(done.wait(8000));QCOMPARE(done.last()[0].toBool(),false);QVERIFY(b.verifyAudit());
  QVERIFY(b.importEvidence(e,"json","Synthetic",true));b.closeWorkspace();QTRY_VERIFY_WITH_TIMEOUT(!b.importBusy(),8000);QVERIFY(b.activeCase().isEmpty());QVERIFY(b.importResult().isEmpty());
 }
 void staleRevisionAndRevocation(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true),other(path,nullptr,true);const auto id=b.createCase(meta());const auto e=evidence(b,t.filePath("good.json"),"{}");QVERIFY(other.openCase(id));QSignalSpy done(&b,&LawCaseBridge::importCompleted);QVERIFY(b.importEvidence(e,"json","Synthetic",true));auto edited=meta();edited.insert("notes","Synthetic parallel change");QVERIFY(other.updateCase(edited));QVERIFY(done.wait(8000));QVERIFY(!done.last()[0].toBool());QVERIFY(b.openCase(id));QVERIFY(b.activeCase().value("analyses").toList().isEmpty());QVERIFY(b.verifyAudit());
  const auto registry=t.filePath("users");const QString password="Synthetic import password 2026!";IdentityService admin(registry,true);QVERIFY(admin.bootstrap("admin","Synthetic administrator",password));QVERIFY(admin.login("admin",password));const auto uid=admin.createUser("owner","Synthetic owner","investigator",password);QVERIFY(!uid.isEmpty());IdentityService owner(registry,true);QVERIFY(owner.login("owner",password));LawCaseBridge secured(t.filePath("secure.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&owner);const auto cid=secured.createCase(meta()),se=evidence(secured,t.filePath("secure.json"),"{}");QVERIFY(!cid.isEmpty());QSignalSpy completed(&secured,&LawCaseBridge::importCompleted);QVERIFY(secured.importEvidence(se,"json","Synthetic",true));owner.logout();QTRY_VERIFY_WITH_TIMEOUT(!secured.importBusy(),8000);QVERIFY(secured.activeCase().isEmpty());QVERIFY(secured.importResult().isEmpty());QVERIFY(owner.login("owner",password));QVERIFY(secured.openCase(cid));QVERIFY(secured.activeCase().value("analyses").toList().isEmpty());
  const auto rid=admin.createUser("reviewer","Synthetic reviewer","reviewer",password);QVERIFY(!rid.isEmpty());QVERIFY(admin.setCaseGrant(cid,rid,true));IdentityService reviewerIdentity(registry,true);QVERIFY(reviewerIdentity.login("reviewer",password));LawCaseBridge reviewer(t.filePath("secure.db"),nullptr,true,nexvary::law::Role::Reviewer,nullptr,&reviewerIdentity);QVERIFY(reviewer.openCase(cid));QSignalSpy reviewed(&reviewer,&LawCaseBridge::importCompleted);QVERIFY(reviewer.importEvidence(se,"json","Synthetic",true));QVERIFY(admin.setCaseGrant(cid,rid,false));QVERIFY(reviewed.wait(8000));QVERIFY(!reviewed.last()[0].toBool());QVERIFY(reviewer.importResult().isEmpty());QVERIFY(!reviewer.openCase(cid));QVERIFY(secured.openCase(cid));QVERIFY(secured.activeCase().value("analyses").toList().isEmpty());QVERIFY(secured.verifyAudit());
 }
};
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 if(app.arguments().contains("--input")){
  const auto path=app.arguments().value(app.arguments().indexOf("--input")+1);const auto marker=QString::fromUtf8(qgetenv("MR_SYNTHETIC_IMPORT_CHILD_MARKER"));if(!marker.isEmpty())write(marker,path.toUtf8());
  const auto mode=qgetenv("MR_SYNTHETIC_IMPORT_CHILD");if(mode=="hang")QThread::msleep(30000);
  if(mode=="changed-copy"){QFile f(path);f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);if(f.open(QIODevice::WriteOnly|QIODevice::Truncate))f.write("[]");f.close();std::fputs("{}",stdout);return 0;}
  if(mode=="flood"){const QByteArray bytes(128*1024,'x');std::fwrite(bytes.constData(),1,static_cast<size_t>(bytes.size()),stdout);return 0;}
  if(mode=="forged"){std::fputs("{\"schema\":\"nexvary-authorized-import/v1\",\"status\":\"imported\"}",stdout);return 0;}
  return 7;
 }
 ImportTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "import_tests.moc"
