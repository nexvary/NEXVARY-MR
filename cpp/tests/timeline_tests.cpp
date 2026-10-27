#include "CaseTimeline.hpp"
#include "LawCaseBridge.hpp"
#include "IdentityService.hpp"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
namespace {
QString hash(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
QVariantMap finding(const QString &value,const QString &kind="string",bool shortened=false){return {{"value",value},{"kind",kind},{"reference","/وقت"},{"valueTruncated",shortened}};}
QVariantMap run(const QString &id,const QVariantList &findings,const QString &eid="e1",bool truncated=false){
 const QByteArray json=QJsonDocument::fromVariant(QVariantMap{{"schema","nexvary-authorized-import/v1"},{"status","imported"},{"sourceSha256",QString(64,'a')},{"findings",findings},{"findingsTruncated",truncated}}).toJson(QJsonDocument::Compact);
 return {{"id",id},{"parser","authorized-import/json"},{"evidenceId",eid},{"sourceSha256",QString(64,'a')},{"reportJson",QString::fromUtf8(json)},{"reportSha256",hash(json)}};
}
QVariantMap snapshot(const QVariantList &runs){return {{"id","case1"},{"revision",9},{"synthetic",true},{"analyses",runs},{"evidence",QVariantList{QVariantMap{{"id","e1"},{"deviceId","device1"},{"name","دليل اصطناعي"},{"sha256",QString(64,'a')}}}}};}
QVariantMap metadata(){return {{"caseNumber","SYNTHETIC-TIMELINE"},{"investigatingAgency","Synthetic lab"},{"investigatorName","Synthetic operator"},{"seizedDeviceDescription","Synthetic document"},{"authorityReference","Synthetic scope"},{"issuingAuthority","Lab owner"},{"scope","Synthetic timeline only"}};}
QString add(LawCaseBridge &b,const QString &path){
 QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))return {};
 file.write("{\"first\":\"2026-10-25T12:00:00+03:00\",\"second\":\"2026-10-25T08:00:00Z\",\"unknown\":\"2026-10-25T11:00:00\"}");file.close();
 const auto d=b.addDevice({{"model","Synthetic source"},{"custodian","Synthetic custodian"}});
 return b.addEvidence(d,path,"Synthetic timestamp source","Synthetic custodian",true);
}
bool importForTimeline(LawCaseBridge &b,const QString &e){QSignalSpy done(&b,&LawCaseBridge::importCompleted);return b.importEvidence(e,"json","Synthetic timeline test",true)&&done.wait(8000)&&done.last()[0].toBool();}
bool build(LawCaseBridge &b,const QString &query={}){QSignalSpy done(&b,&LawCaseBridge::timelineCompleted);return b.buildTimeline(query)&&done.wait(8000)&&done.last()[0].toBool();}
}
class TimelineTests:public QObject{
 Q_OBJECT
private slots:
 void normalization_data(){
  QTest::addColumn<QString>("raw");QTest::addColumn<QString>("utc");QTest::addColumn<QString>("reason");
  QTest::newRow("egypt-offset")<<QString("2026-10-25T12:00:00+03:00")<<QString("2026-10-25T09:00:00.000Z")<<QString("explicit-offset-literal-only");
  QTest::newRow("previous-day")<<QString("2026-01-01T00:30:00+01:00")<<QString("2025-12-31T23:30:00.000Z")<<QString("explicit-offset-literal-only");
  QTest::newRow("negative-offset")<<QString("2026-10-25T12:00:00-05:30")<<QString("2026-10-25T17:30:00.000Z")<<QString("explicit-offset-literal-only");
  QTest::newRow("leap-valid")<<QString("2024-02-29T12:00:00.12Z")<<QString("2024-02-29T12:00:00.120Z")<<QString("explicit-offset-literal-only");
  QTest::newRow("millisecond")<<QString("2026-10-25T12:00:00.123Z")<<QString("2026-10-25T12:00:00.123Z")<<QString("explicit-offset-literal-only");
  QTest::newRow("max-offset")<<QString("2026-10-25T12:00:00+14:00")<<QString("2026-10-24T22:00:00.000Z")<<QString("explicit-offset-literal-only");
  QTest::newRow("no-zone")<<QString("2026-10-25T12:00:00")<<QString()<<QString("unsupported-format-or-missing-timezone");
  QTest::newRow("no-year-zero")<<QString("0000-01-01T12:00:00Z")<<QString()<<QString("invalid-calendar-date");
  QTest::newRow("bad-date")<<QString("2026-02-29T12:00:00Z")<<QString()<<QString("invalid-calendar-date");
  QTest::newRow("bad-hour")<<QString("2026-10-25T24:00:00Z")<<QString()<<QString("invalid-time-or-leap-second");
  QTest::newRow("leap-second")<<QString("2026-10-25T12:00:60Z")<<QString()<<QString("invalid-time-or-leap-second");
  QTest::newRow("unknown-offset")<<QString("2026-10-25T12:00:00-00:00")<<QString()<<QString("unknown-offset");
  QTest::newRow("no-rounding")<<QString("2026-10-25T12:00:00.123456Z")<<QString()<<QString("unsupported-submillisecond-precision");
  QTest::newRow("bad-offset")<<QString("2026-10-25T12:00:00+14:01")<<QString()<<QString("offset-out-of-range");
  QTest::newRow("bad-offset-minutes")<<QString("2026-10-25T12:00:00+01:60")<<QString()<<QString("offset-out-of-range");
  QTest::newRow("embedded")<<QString("2026-10-25T12:00:00Z alleged event")<<QString()<<QString("unsupported-format-or-missing-timezone");
  QTest::newRow("date-only")<<QString("2026-10-25")<<QString()<<QString("unsupported-format-or-missing-timezone");
  QTest::newRow("lowercase")<<QString("2026-10-25T12:00:00z")<<QString()<<QString("unsupported-format-or-missing-timezone");
  QTest::newRow("utc-underflow")<<QString("0001-01-01T00:00:00+14:00")<<QString()<<QString("utc-out-of-range");
 }
 void normalization(){
  QFETCH(QString,raw);QFETCH(QString,utc);QFETCH(QString,reason);
  const auto report=nexvary::law::projectTimeline(snapshot({run("r1",{finding(raw)})}),{});
  const auto rows=report.value("events").toList();QCOMPARE(rows.size(),1);
  const auto row=rows.first().toMap();QCOMPARE(row.value("originalTime").toString(),raw);QCOMPARE(row.value("utc").toString(),utc);QCOMPARE(row.value("reason").toString(),reason);QCOMPARE(row.value("confidence").toString(),QString("source-reported-unverified"));
 }
 void deterministicOrderingProvenanceAndSearch(){
  const auto input=snapshot({run("rB",{finding("2026-10-25T12:00:00+03:00"),finding("10-25 11:00:00.000 1 2 I Synthetic: fixture","logcat-threadtime-raw")}),run("rA",{finding("2026-10-25T08:00:00Z"),finding("2026-10-25T09:00:00Z")})});
  const auto first=nexvary::law::projectTimeline(input,{});QCOMPARE(first,nexvary::law::projectTimeline(input,{}));
  const auto rows=first.value("events").toList();QCOMPARE(rows.size(),4);QCOMPARE(rows[0].toMap().value("id").toString(),QString("rA:0"));QCOMPARE(rows[1].toMap().value("id").toString(),QString("rA:1"));QCOMPARE(rows[2].toMap().value("id").toString(),QString("rB:0"));QCOMPARE(rows.last().toMap().value("reason").toString(),QString("missing-year-and-timezone"));
  QCOMPARE(rows[0].toMap().value("deviceId").toString(),QString("device1"));QCOMPARE(rows[0].toMap().value("reference").toString(),QString("/وقت"));QCOMPARE(first.value("normalizedCount").toInt(),3);QCOMPARE(first.value("unresolvedCount").toInt(),1);
  QCOMPARE(nexvary::law::projectTimeline(input,"اصطناعي").value("events").toList().size(),4);QCOMPARE(nexvary::law::projectTimeline(input,"2026-10-25T08").value("events").toList().size(),1);QVERIFY(nexvary::law::projectTimeline(input,".*").value("events").toList().isEmpty());
  const auto shortValue=nexvary::law::projectTimeline(snapshot({run("r",{finding("2026-10-25T12:00:00Z","string",true)})}),{});QCOMPARE(shortValue.value("events").toList()[0].toMap().value("reason").toString(),QString("shortened-source-value"));
 }
 void integrityAndBounds(){
  auto r=run("r",{finding("2026-10-25T12:00:00Z")});r.insert("reportJson",r.value("reportJson").toString()+" ");
  QVERIFY(nexvary::law::projectTimeline(snapshot({r}),{}).contains("errorCode"));
  r=run("r",{finding("2026-10-25T12:00:00Z")},"different-case");QVERIFY(nexvary::law::projectTimeline(snapshot({r}),{}).contains("errorCode"));
  r=run("r",{finding("2026-10-25T12:00:00Z")});QVERIFY(nexvary::law::projectTimeline(snapshot({r,r}),{}).contains("errorCode"));
  QVariantList tooMany;for(int i=0;i<129;++i)tooMany.append(finding("2026-10-25T12:00:00Z"));QVERIFY(nexvary::law::projectTimeline(snapshot({run("r",tooMany)}),{}).contains("errorCode"));
  QVERIFY(nexvary::law::projectTimeline(snapshot({}),QString(257,'x')).contains("errorCode"));
  auto cancelled=std::make_shared<std::atomic_bool>(true);QCOMPARE(nexvary::law::projectTimeline(snapshot({r}),{},cancelled).value("errorCode").toString(),QString("cancelled"));
 }
 void boundedCollectionBenchmark(){
  QVariantList rows,runs;for(int i=0;i<128;++i)rows.append(finding("2026-10-25T12:00:00Z"));
  for(int i=0;i<100;++i)runs.append(run(QString("run-%1").arg(i,3,10,QChar('0')),rows,"e1",true));
  QElapsedTimer timer;timer.start();const auto result=nexvary::law::projectTimeline(snapshot(runs),{});
  qInfo("Synthetic timeline: 12800 observations; elapsed %lld ms",static_cast<long long>(timer.elapsed()));
  QCOMPARE(result.value("events").toList().size(),nexvary::law::timelineEventLimit);QCOMPARE(result.value("omittedMatchingCandidates").toInt(),12800-nexvary::law::timelineEventLimit);QCOMPARE(result.value("truncatedImportReports").toInt(),100);
 }
 void nativePersistenceCancellationAndRoles(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true);QVERIFY(!b.buildTimeline());const auto id=b.createCase(metadata());QVERIFY(!id.isEmpty());const auto e=add(b,t.filePath("source.json"));QVERIFY(!e.isEmpty());QVERIFY(importForTimeline(b,e));
  const auto revision=b.activeCase().value("revision");QVERIFY(build(b));QCOMPARE(b.timelineResult().value("normalizedCount").toInt(),2);QCOMPARE(b.timelineResult().value("unresolvedCount").toInt(),1);QCOMPARE(b.activeCase().value("revision"),revision);QVERIFY(b.verifyEvidence(e));QVERIFY(b.verifyAudit());
  const auto backup=b.exportProtectedBackup(t.filePath("backup"),true);QVERIFY(!backup.isEmpty());QVERIFY(b.timelineResult().isEmpty());
  LawCaseBridge restored(t.filePath("restored.db"),nullptr,true);QCOMPARE(restored.restoreProtectedBackup(t.filePath("backup"),backup.value("sha256").toString(),true),id);QVERIFY(build(restored));QCOMPARE(restored.timelineResult().value("events").toList().size(),3);
  LawCaseBridge auditor(path,nullptr,true,nexvary::law::Role::Auditor);QVERIFY(auditor.openCase(id));QVERIFY(!auditor.buildTimeline());QVERIFY(auditor.timelineResult().isEmpty());
  QVERIFY(b.buildTimeline());b.cancelTimeline();QTRY_VERIFY_WITH_TIMEOUT(!b.timelineBusy(),8000);QVERIFY(b.timelineResult().isEmpty());
  QVERIFY(b.buildTimeline());b.closeWorkspace();QTRY_VERIFY_WITH_TIMEOUT(!b.timelineBusy(),8000);QVERIFY(b.timelineResult().isEmpty());
  QVERIFY(b.openCase(id));LawCaseBridge other(path,nullptr,true);QVERIFY(other.openCase(id));QSignalSpy done(&b,&LawCaseBridge::timelineCompleted);QVERIFY(b.buildTimeline());auto meta=metadata();meta.insert("notes","Synthetic parallel update");QVERIFY(other.updateCase(meta));QVERIFY(done.wait(8000));QVERIFY(!done.last()[0].toBool());QVERIFY(b.timelineResult().isEmpty());
  QVERIFY(b.openCase(id));QVERIFY(build(b));QVERIFY(!b.timelineResult().isEmpty());meta.insert("notes","Synthetic update after projection");QVERIFY(other.updateCase(meta));QVERIFY(!b.buildTimeline());QVERIFY(b.timelineResult().isEmpty());
 }
 void nativeGrantRevocation(){
  QTemporaryDir t;const auto registry=t.filePath("identities");const QString password="Synthetic timeline password 2026!";
  IdentityService admin(registry,true);QVERIFY(admin.bootstrap("admin","Synthetic admin",password));QVERIFY(admin.login("admin",password));QVERIFY(!admin.createUser("owner","Synthetic owner","investigator",password).isEmpty());const auto rid=admin.createUser("reviewer","Synthetic reviewer","reviewer",password);QVERIFY(!rid.isEmpty());
  IdentityService owner(registry,true);QVERIFY(owner.login("owner",password));const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true,nexvary::law::Role::Investigator,nullptr,&owner);const auto id=b.createCase(metadata());QVERIFY(!id.isEmpty());const auto e=add(b,t.filePath("source.json"));QVERIFY(importForTimeline(b,e));QVERIFY(admin.setCaseGrant(id,rid,true));
  IdentityService reviewerIdentity(registry,true);QVERIFY(reviewerIdentity.login("reviewer",password));LawCaseBridge reviewer(path,nullptr,true,nexvary::law::Role::Reviewer,nullptr,&reviewerIdentity);QVERIFY(reviewer.openCase(id));
  QSignalSpy done(&reviewer,&LawCaseBridge::timelineCompleted);QVERIFY(reviewer.buildTimeline());QVERIFY(admin.setCaseGrant(id,rid,false));QVERIFY(done.wait(8000));QVERIFY(!done.last()[0].toBool());QVERIFY(reviewer.timelineResult().isEmpty());
  QVERIFY(b.buildTimeline());owner.logout();QTRY_VERIFY_WITH_TIMEOUT(!b.timelineBusy(),8000);QVERIFY(b.timelineResult().isEmpty());QVERIFY(b.activeCase().isEmpty());
 }
};
QTEST_GUILESS_MAIN(TimelineTests)
#include "timeline_tests.moc"
