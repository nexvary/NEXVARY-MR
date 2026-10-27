#include "CaseArtifacts.hpp"
#include "LawCaseBridge.hpp"
#include "IdentityService.hpp"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>

namespace {
QString hash(const QByteArray &bytes) { return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex()); }
QVariantMap reference() { QFile f(MR_ARTIFACT_REFERENCE_PATH); if (!f.open(QIODevice::ReadOnly)) return {}; return QJsonDocument::fromJson(f.readAll()).toVariant().toMap(); }
QVariantMap snapshot() { return reference().value("snapshot").toMap(); }
QVariantMap report(const QVariantMap &run) { return QJsonDocument::fromJson(run.value("reportJson").toString().toUtf8()).toVariant().toMap(); }
void encode(QVariantMap &run, const QVariantMap &value) { const auto bytes = QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact); run.insert("reportJson", QString::fromUtf8(bytes)); run.insert("reportSha256", hash(bytes)); }
QSet<QByteArray> truthEdges(const QVariantMap &result) {
 QMap<QString,QVariantMap> nodes;for(const auto &v:result.value("nodes").toList())nodes.insert(v.toMap().value("id").toString(),v.toMap());
 QSet<QByteArray> out;
 for(const auto &v:result.value("edges").toList()) {const auto e=v.toMap(),from=nodes.value(e.value("from").toString()),to=nodes.value(e.value("to").toString());
  out.insert(QJsonDocument::fromVariant(QVariantList{from.value("type"),from.value("recordId"),e.value("type"),to.value("type"),to.value("recordId")}).toJson(QJsonDocument::Compact));}
 return out;
}
QVariantMap metadata(){return {{"caseNumber","SYNTHETIC-INDEX"},{"investigatingAgency","Synthetic lab"},{"investigatorName","Synthetic operator"},{"seizedDeviceDescription","Synthetic document"},{"authorityReference","Synthetic scope"},{"issuingAuthority","Lab owner"},{"scope","Synthetic findings only"}};}
QString add(LawCaseBridge &b,const QString &path){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly))return {};f.write(QString("{\"name\":\"أحمد\",\"note\":\"SYNTHETIC\",\"time\":\"2026-10-26T08:00:00Z\"}").toUtf8());f.close();const auto d=b.addDevice({{"model","Synthetic source"},{"custodian","Synthetic custodian"}});return b.addEvidence(d,path,"Synthetic artifact source","Synthetic custodian",true);}
bool imported(LawCaseBridge &b,const QString &e){QSignalSpy done(&b,&LawCaseBridge::importCompleted);return b.importEvidence(e,"json","Synthetic artifact test",true)&&done.wait(8000)&&done.last()[0].toBool();}
bool build(LawCaseBridge &b,const QString &query={}){QSignalSpy done(&b,&LawCaseBridge::artifactsCompleted);return b.buildArtifacts(query)&&done.wait(8000)&&done.last()[0].toBool();}
}
class ArtifactTests:public QObject {
 Q_OBJECT
private slots:
 void independentTruthAndDeterminism(){
  const auto input=snapshot(),expected=reference().value("expected").toMap();QVERIFY(!input.isEmpty());
  const auto result=nexvary::law::projectArtifacts(input,{});QVERIFY2(!result.contains("errorCode"),qPrintable(result.value("errorCode").toString()));
  QCOMPARE(result,nexvary::law::projectArtifacts(input,{}));QCOMPARE(result.value("rows").toList().size(),6);QCOMPARE(result.value("nodes").toList().size(),19);
  QSet<QByteArray> wanted;for(const auto &v:expected.value("edges").toList())wanted.insert(QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact));
  const auto got=truthEdges(result);QCOMPARE(got.size(),20);QCOMPARE(got,wanted);
  qInfo("Structural truth corpus: 20 true edges, 0 false links, 0 missed links; excludes people/events");
  const auto groups=result.value("digestGroups").toList();QCOMPARE(groups.size(),2);const auto groupTruth=expected.value("digestGroups").toMap();
  QMap<QString,QVariantMap> nodes;for(const auto &v:result.value("nodes").toList())nodes.insert(v.toMap().value("id").toString(),v.toMap());
  for(const auto &g:groups){const auto group=g.toMap();QCOMPARE(group.value("basis").toString(),QString("equal-recorded-digest-only"));QSet<QString> actual,want;
   for(const auto &v:group.value("members").toList()){const auto n=nodes.value(v.toMap().value("nodeId").toString());actual.insert(n.value("type").toString()+"/"+n.value("recordId").toString());}
   for(const auto &v:groupTruth.value(group.value("sha256").toString()).toList()){want.insert(v.toString());}
   QCOMPARE(actual,want);}
  for(const auto &v:result.value("rows").toList()){const auto row=v.toMap();const auto path=row.value("provenancePath").toList();QCOMPARE(path.size(),5);QCOMPARE(path[0].toMap().value("recordId"),input.value("id"));QCOMPARE(path[3].toMap().value("recordId"),row.value("analysisId"));QCOMPARE(path[4].toMap().value("id"),row.value("id"));}
  auto reordered=input;auto runs=input.value("analyses").toList();std::reverse(runs.begin(),runs.end());reordered.insert("analyses",runs);QCOMPARE(result,nexvary::law::projectArtifacts(reordered,{}));
 }
 void literalArabicSearchAndScope(){
  const auto input=snapshot();QCOMPARE(nexvary::law::projectArtifacts(input,"أحمد").value("rows").toList().size(),3);QCOMPARE(nexvary::law::projectArtifacts(input,"SAME-NAME").value("rows").toList().size(),6);
  for(const auto &q:{QString(".*"),QString("احمد"),QString("caseId"),QString("unrelated")})QVERIFY(nexvary::law::projectArtifacts(input,q).value("rows").toList().isEmpty());
  const auto all=nexvary::law::projectArtifacts(input,{});const auto one=all.value("rows").toList().first().toMap();QVERIFY(!nexvary::law::projectArtifacts(input,one.value("reportSha256").toString()).value("rows").toList().isEmpty());
  const auto filtered=nexvary::law::projectArtifacts(input,"أحمد");QCOMPARE(filtered.value("matchingCount").toInt(),3);QCOMPARE(filtered.value("digestGroups").toList().size(),1);
 }
 void corruption_data(){QTest::addColumn<QString>("mode");for(const auto &m:{"hash","wrong-evidence","wrong-source","size","duplicate-run","device","duplicate-device","duplicate-evidence","provenance-case","provenance-device","provenance-evidence","schema","adapter","version","findings","count","truncation","kind","value","member-hash","duplicate-key","malformed-json","report-limit"})QTest::newRow(m)<<QString(m);}
 void corruption(){
  QFETCH(QString,mode);auto input=snapshot();auto runs=input.value("analyses").toList();auto run=runs[0].toMap();auto r=report(run);
  if(mode=="hash")run.insert("reportJson",run.value("reportJson").toString()+" ");
  else if(mode=="wrong-evidence")run.insert("evidenceId","foreign");
  else if(mode=="wrong-source")run.insert("sourceSha256",QString(64,'0'));
  else if(mode=="size")run.insert("sourceSize",999);
  else if(mode=="duplicate-run")runs.append(run);
  else if(mode=="device")input.insert("devices",QVariantList());
  else if(mode=="duplicate-device"){auto list=input.value("devices").toList();list.append(list.first());input.insert("devices",list);}
  else if(mode=="duplicate-evidence"){auto list=input.value("evidence").toList();list.append(list.first());input.insert("evidence",list);}
  else if(mode.startsWith("provenance-")){auto p=r.value("provenance").toMap();p.insert(mode=="provenance-case"?"caseId":mode=="provenance-device"?"deviceId":"evidenceId","foreign");r.insert("provenance",p);encode(run,r);}
  else if(mode=="schema"){r.insert("schema","future");encode(run,r);}
  else if(mode=="adapter"){r.insert("adapter","json");encode(run,r);}
  else if(mode=="version"){r.insert("parserVersion",1.25);encode(run,r);}
  else if(mode=="findings"){r.insert("findings","not an array");encode(run,r);}
  else if(mode=="count"){r.insert("observedCount",1);encode(run,r);}
  else if(mode=="truncation"){r.insert("findingsTruncated",true);encode(run,r);}
  else if(mode=="kind"||mode=="value"||mode=="member-hash"){auto list=r.value("findings").toList();auto f=list[mode=="member-hash"?1:0].toMap();f.insert(mode=="kind"?"kind":"value",mode=="value"?QVariant(123):mode=="member-hash"?QVariant("not-a-digest"):QVariant(QString()));list[mode=="member-hash"?1:0]=f;r.insert("findings",list);encode(run,r);}
  else if(mode=="duplicate-key"||mode=="malformed-json"||mode=="report-limit"){const QByteArray bytes=mode=="duplicate-key"?QByteArray("{\"x\":1,\"\\u0078\":2}"):mode=="report-limit"?QByteArray(64*1024+1,' '):QByteArray("{broken");run.insert("reportJson",QString::fromUtf8(bytes));run.insert("reportSha256",hash(bytes));}
  runs[0]=run;input.insert("analyses",runs);QVERIFY2(nexvary::law::projectArtifacts(input,{}).contains("errorCode"),qPrintable(mode));
 }
 void boundsFullScanAndBenchmark(){
  auto input=snapshot();auto run=input.value("analyses").toList()[0].toMap(),r=report(run);QVariantList findings;const auto f=r.value("findings").toList()[0];for(int i=0;i<128;++i)findings.append(f);r.insert("findings",findings);r.insert("observedCount",129);r.insert("findingsTruncated",true);encode(run,r);QVariantList runs;
  for(int i=0;i<100;++i){run.insert("id",QString("r-%1").arg(i));runs.append(run);}input.insert("analyses",runs);
  QElapsedTimer timer;timer.start();const auto result=nexvary::law::projectArtifacts(input,{});qInfo("Synthetic artifact index: 12800 retained observations; %lld ms",static_cast<long long>(timer.elapsed()));
  QVERIFY2(!result.contains("errorCode"),qPrintable(result.value("errorCode").toString()));QCOMPARE(result.value("observedCount").toInt(),12800);QCOMPARE(result.value("rows").toList().size(),2048);QCOMPARE(result.value("omittedMatchingFindings").toInt(),10752);QCOMPARE(result.value("truncatedImportReports").toInt(),100);QVERIFY(result.value("nodes").toList().size()<=8192);
  run=runs.last().toMap();run.insert("reportSha256",QString(64,'0'));runs.last()=run;input.insert("analyses",runs);QVERIFY(nexvary::law::projectArtifacts(input,{}).contains("errorCode"));
  QVERIFY(nexvary::law::projectArtifacts(snapshot(),QString(257,'x')).contains("errorCode"));auto cancelled=std::make_shared<std::atomic_bool>(true);QCOMPARE(nexvary::law::projectArtifacts(snapshot(),{},cancelled).value("errorCode").toString(),QString("cancelled"));
 }
 void adbDigestAndUnsupportedScopes(){
  auto input=snapshot();auto sources=input.value("evidence").toList();auto source=sources[0].toMap();source.insert("size",4096);sources[0]=source;input.insert("evidence",sources);
  auto runs=input.value("analyses").toList();auto run=runs[0].toMap(),r=report(run);const auto digest=r.value("findings").toList()[1].toMap().value("value").toString();
  const QVariantMap f{{"reference","apps/com.synthetic/f/blob"},{"kind","adb-backup-member"},{"value",digest},{"valueTruncated",false},{"memberSha256",digest},{"entryType","file"},{"memberSize",64},{"sourceByteOffset",1024},{"rawMtimeEpochSeconds","1761393600"}};
  run.insert("parser","authorized-import/adb-backup-v5");run.insert("sourceSize",4096);r.insert("adapter","adb-backup-v5");r.insert("sourceSize",4096);r.insert("findings",QVariantList{f});r.insert("observedCount",1);encode(run,r);runs[0]=run;
  auto rejected=run;rejected.insert("id","rejected-run");auto rejectedReport=r;rejectedReport.insert("status","rejected");rejectedReport.insert("findings",QVariantList());encode(rejected,rejectedReport);runs.append(rejected);runs.append(QVariantMap{{"id","classic"},{"parser","classic-pcap"}});input.insert("analyses",runs);
  const auto result=nexvary::law::projectArtifacts(input,{});QVERIFY2(!result.contains("errorCode"),qPrintable(result.value("errorCode").toString()));QCOMPARE(result.value("rejectedImportReports").toInt(),1);QCOMPARE(result.value("unsupportedAnalysisReports").toInt(),1);QCOMPARE(result.value("rows").toList().size(),5);
  QVERIFY(nexvary::law::projectArtifacts(input,"1761393600").value("rows").toList().isEmpty());
  auto changed=f;changed.insert("memberSha256",QString(64,'0'));r.insert("findings",QVariantList{changed});encode(run,r);runs[0]=run;input.insert("analyses",runs);QCOMPARE(nexvary::law::projectArtifacts(input,{}).value("errorCode").toString(),QString("member-provenance-schema"));
  const QVariantMap empty{{"id","empty-case"},{"revision",1},{"synthetic",true}};const auto noRows=nexvary::law::projectArtifacts(empty,{});QVERIFY(!noRows.contains("errorCode"));QCOMPARE(noRows.value("nodes").toList().size(),1);QVERIFY(noRows.value("edges").toList().isEmpty());
 }
 void caseIsolation(){
  const auto first=nexvary::law::projectArtifacts(snapshot(),{});auto other=snapshot();other.insert("id","other-case");auto runs=other.value("analyses").toList();
  QVERIFY(nexvary::law::projectArtifacts(other,{}).contains("errorCode"));
  for(auto &v:runs){auto run=v.toMap(),r=report(run),p=r.value("provenance").toMap();p.insert("caseId","other-case");r.insert("provenance",p);encode(run,r);v=run;}other.insert("analyses",runs);
  const auto second=nexvary::law::projectArtifacts(other,{});QVERIFY(!second.contains("errorCode"));QSet<QString> ids;for(const auto &v:first.value("nodes").toList())ids.insert(v.toMap().value("id").toString());for(const auto &v:second.value("nodes").toList())QVERIFY(!ids.contains(v.toMap().value("id").toString()));
 }
 void nativePersistenceCancellationAndRoles(){
  QTemporaryDir t;const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true);QVERIFY(!b.buildArtifacts());const auto id=b.createCase(metadata());QVERIFY(!id.isEmpty());const auto e=add(b,t.filePath("source.json"));QVERIFY(!e.isEmpty());QVERIFY(imported(b,e));const auto revision=b.activeCase().value("revision");QVERIFY(build(b));QCOMPARE(b.artifactsResult().value("rows").toList().size(),3);QCOMPARE(b.activeCase().value("revision"),revision);const auto rows=b.artifactsResult().value("rows");QVERIFY(build(b));QCOMPARE(b.artifactsResult().value("rows"),rows);QVERIFY(b.verifyEvidence(e));QVERIFY(b.verifyAudit());
  const auto backup=b.exportProtectedBackup(t.filePath("backup"),true);QVERIFY(!backup.isEmpty());QVERIFY(b.artifactsResult().isEmpty());LawCaseBridge restored(t.filePath("restored.db"),nullptr,true);QCOMPARE(restored.restoreProtectedBackup(t.filePath("backup"),backup.value("sha256").toString(),true),id);QVERIFY(build(restored));QCOMPARE(restored.artifactsResult().value("rows"),rows);
  LawCaseBridge auditor(path,nullptr,true,nexvary::law::Role::Auditor);QVERIFY(auditor.openCase(id));QVERIFY(!auditor.buildArtifacts());QVERIFY(auditor.artifactsResult().isEmpty());
  QVERIFY(b.buildArtifacts());b.cancelArtifacts();QTRY_VERIFY_WITH_TIMEOUT(!b.artifactsBusy(),8000);QVERIFY(b.artifactsResult().isEmpty());QVERIFY(b.buildArtifacts());b.closeWorkspace();QTRY_VERIFY_WITH_TIMEOUT(!b.artifactsBusy(),8000);QVERIFY(b.artifactsResult().isEmpty());
  QVERIFY(b.openCase(id));LawCaseBridge other(path,nullptr,true);QVERIFY(other.openCase(id));QSignalSpy done(&b,&LawCaseBridge::artifactsCompleted);QVERIFY(b.buildArtifacts());auto m=metadata();m.insert("notes","Synthetic parallel edit");QVERIFY(other.updateCase(m));QVERIFY(done.wait(8000));QVERIFY(!done.last()[0].toBool());QVERIFY(b.artifactsResult().isEmpty());
  QVERIFY(b.openCase(id));QVERIFY(build(b));m.insert("notes","Changed after build");QVERIFY(other.updateCase(m));QVERIFY(!b.buildArtifacts());QVERIFY(b.artifactsResult().isEmpty());
 }
 void nativeGrantRevocation(){
  QTemporaryDir t;const auto registry=t.filePath("identities");const QString password="Synthetic index password 2026!";IdentityService admin(registry,true);QVERIFY(admin.bootstrap("admin","Synthetic admin",password));QVERIFY(admin.login("admin",password));QVERIFY(!admin.createUser("owner","Synthetic owner","investigator",password).isEmpty());const auto rid=admin.createUser("reviewer","Synthetic reviewer","reviewer",password);QVERIFY(!rid.isEmpty());
  IdentityService owner(registry,true);QVERIFY(owner.login("owner",password));const auto path=t.filePath("cases.db");LawCaseBridge b(path,nullptr,true,nexvary::law::Role::Investigator,nullptr,&owner);const auto id=b.createCase(metadata());const auto e=add(b,t.filePath("source.json"));QVERIFY(imported(b,e));QVERIFY(admin.setCaseGrant(id,rid,true));
  IdentityService ri(registry,true);QVERIFY(ri.login("reviewer",password));LawCaseBridge reviewer(path,nullptr,true,nexvary::law::Role::Reviewer,nullptr,&ri);QVERIFY(reviewer.openCase(id));QSignalSpy done(&reviewer,&LawCaseBridge::artifactsCompleted);QVERIFY(reviewer.buildArtifacts());QVERIFY(admin.setCaseGrant(id,rid,false));QVERIFY(done.wait(8000));QVERIFY(!done.last()[0].toBool());QVERIFY(reviewer.artifactsResult().isEmpty());QVERIFY(!reviewer.buildArtifacts());
  QVERIFY(b.buildArtifacts());owner.logout();QTRY_VERIFY_WITH_TIMEOUT(!b.artifactsBusy(),8000);QVERIFY(b.artifactsResult().isEmpty());QVERIFY(b.activeCase().isEmpty());QVERIFY(owner.login("owner",password));QVERIFY(!b.openCase("foreign-case"));QVERIFY(b.artifactsResult().isEmpty());
 }
};
QTEST_GUILESS_MAIN(ArtifactTests)
#include "artifact_tests.moc"
