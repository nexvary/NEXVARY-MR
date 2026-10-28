#include "ForensicAdapters.hpp"
#include "CaseArtifacts.hpp"
#include "CaseTimeline.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>
#include <random>

namespace {
const QByteArray bookmarks=R"({"roots":{"bookmark_bar":{"type":"folder","name":"Lab","children":[{"type":"url","name":"Synthetic","url":"https://example.invalid/","date_added":"13300000000000000"}]}}})";
const QByteArray manifest=R"({"entries":[{"path":"evidence.bin","sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}]})";
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
mr::forensic::ReadOnlyEvidenceRequest request(const QString &adapter,const QByteArray &bytes){return {"00000000-0000-4000-8000-000000000001","00000000-0000-4000-8000-000000000002",sha(bytes),adapter,bytes,std::make_shared<std::atomic_bool>(false)};}
void u32(QByteArray &b,int p,quint32 v){qToLittleEndian(v,reinterpret_cast<uchar*>(b.data()+p));}
void u64(QByteArray &b,int p,quint64 v){qToLittleEndian(v,reinterpret_cast<uchar*>(b.data()+p));}
quint32 crc(const QByteArray &b){quint32 c=~0u;for(uchar x:b){c^=x;for(int i=0;i<8;++i)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return ~c;}
QByteArray mbr(){QByteArray b(512*64,0);b.replace(510,2,QByteArray::fromHex("55aa"));b[450]=7;u32(b,454,8);u32(b,458,16);return b;}
QByteArray gpt(){
 auto b=mbr();b.replace(450,1,QByteArray::fromHex("ee"));u32(b,454,1);u32(b,458,63);
 QByteArray table(128*4,0);table[0]=1;table[16]=2;u64(table,32,8);u64(table,40,23);
 b.replace(512*2,512,table);b.replace(512*62,512,table);
 for(const quint64 lba:{quint64(1),quint64(63)}){
  QByteArray h(512,0);h.replace(0,8,"EFI PART");u32(h,8,0x10000);u32(h,12,92);u64(h,24,lba);u64(h,32,lba==1?63:1);u64(h,40,3);u64(h,48,61);u64(h,72,lba==1?2:62);u32(h,80,4);u32(h,84,128);u32(h,88,crc(table));u32(h,16,crc(h.left(92)));b.replace(qsizetype(lba*512),512,h);
 }return b;
}
QVariantMap metadata(){return {{"caseNumber","Synthetic forensic integration"},{"investigatingAgency","Synthetic lab"},{"investigatorName","Synthetic investigator"},{"seizedDeviceDescription","Synthetic fixture"},{"authorityReference","Synthetic authority"},{"issuingAuthority","Lab owner"},{"scope","Synthetic fixture only"}};}
}
class ForensicAdapterTests:public QObject {
 Q_OBJECT
private slots:
 void descriptorsAndDestructiveRejection(){
  QCOMPARE(mr::forensic::forensicAdapters().size(),3);
  for(const auto &id:mr::forensic::forensicAdapters()){const auto d=mr::forensic::forensicSource(id);QCOMPARE(d.upstreamCommit.size(),40);QCOMPARE(d.maxFindings,128);QVERIFY(!d.permissionBasis.isEmpty());}
  for(const auto &command:{"format","erase","repair","write","rebuild-partitions","dc.format","powershell","../../Operations.cpp"})QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(request(command,mbr())).errorCode,QString("adapter-not-allowlisted"));
 }
 void provenanceHashInputAndCancellation(){
  auto r=request("chromium-bookmarks",bookmarks);r.sourceSha256=QString(64,'0');QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("source-hash-mismatch"));
  r=request("chromium-bookmarks",bookmarks);r.caseId="other-case";QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("invalid-provenance"));
  r=request("chromium-bookmarks",bookmarks);r.cancelled->store(true);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("cancelled"));
  QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(request("chromium-bookmarks",QByteArray(8*1024*1024+1,'x'))).errorCode,QString("input-limit"));
 }
 void cooperativeDeadline(){
  auto r=request("chromium-bookmarks",bookmarks);r.deadline=QDeadlineTimer(0);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("worker-timeout"));
 }
 void bookmarksKnownAnswerAndReadOnly(){
  const auto r=request("chromium-bookmarks",bookmarks);const auto before=r.verifiedBytes;
  const auto result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY2(result.ok(),qPrintable(result.errorCode));QCOMPARE(r.verifiedBytes,before);
  const auto rows=result.report.value("findings").toList();QCOMPARE(rows.size(),1);const auto f=rows[0].toMap();
  QCOMPARE(f.value("value").toString(),QString("https://example.invalid/"));QCOMPARE(f.value("details").toMap().value("timestampUtc").toString(),QString("2022-06-18T04:26:40.000Z"));
  QCOMPARE(f.value("details").toMap().value("dateAddedRaw").toString(),QString("13300000000000000"));QCOMPARE(result.report.value("sourceSha256").toString(),sha(bookmarks));
 }
 void corruptAmbiguousDeepAndOversizedBookmarks(){
  for(const auto &bytes:QList<QByteArray>{"SQLite format 3", "{", "{}", R"({"roots":{},"roots":{}})", R"({"roots":{"a":{"type":"url","name":"x","url":"x","date_added":12}}})"})QVERIFY(!mr::forensic::analyzeReadOnlyEvidence(request("chromium-bookmarks",bytes)).ok());
  QByteArray deep="0";for(int i=0;i<42;++i)deep="["+deep+"]";QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(request("chromium-bookmarks",deep)).errorCode,QString("ambiguous-or-deep-json"));
  auto text=bookmarks;text.replace("Synthetic",QByteArray(513,'x'));QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(request("chromium-bookmarks",text)).errorCode,QString("bookmark-text-limit"));
 }
 void manifestStructureIsNotPayloadVerification(){
  const auto result=mr::forensic::analyzeReadOnlyEvidence(request("workbench-export-manifest",manifest));QVERIFY(result.ok());QVERIFY(result.report.value("structurallyValid").toBool());QVERIFY(!result.report.value("payloadVerified").toBool());
  auto damaged=manifest;damaged.replace(QByteArray(64,'a'),"123");const auto bad=mr::forensic::analyzeReadOnlyEvidence(request("workbench-export-manifest",damaged));QVERIFY(bad.ok());QVERIFY(!bad.report.value("structurallyValid").toBool());
  for(const auto &bytes:QList<QByteArray>{"{}",R"({"entries":{}})",R"({"entries":[{"path":"a","sha256":""}]})",R"({"files":[],"entries":[]})"}){
   const auto r=mr::forensic::analyzeReadOnlyEvidence(request("workbench-export-manifest",bytes));QVERIFY(!r.ok()||!r.report.value("structurallyValid").toBool());
  }
 }
 void partitionKnownAnswers(){
  for(const auto &bytes:QList<QByteArray>{mbr(),gpt()}){
   const auto before=bytes;const auto result=mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",bytes));QVERIFY2(result.ok(),qPrintable(result.errorCode));QCOMPARE(bytes,before);
   const auto rows=result.report.value("findings").toList();QCOMPARE(rows.size(),1);const auto extent=rows[0].toMap().value("details").toMap();QCOMPARE(extent.value("offsetBytes").toULongLong(),qulonglong(4096));QCOMPARE(extent.value("lengthBytes").toULongLong(),qulonglong(8192));
  }
 }
 void truncatedCorruptProtectiveAndBackupGpt(){
  QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",mbr().chopped(1))).errorCode,QString("image-size-or-sector-alignment"));
  auto b=gpt();b[512+16]^=1;auto result=mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",b));QVERIFY(result.ok());QVERIFY(result.report.value("warnings").toStringList().join(' ').contains("Backup GPT"));
  b[63*512+16]^=1;QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",b)).errorCode,QString("gpt-checksum-or-bounds"));
  b=mbr();u32(b,454,63);u32(b,458,16);result=mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",b));QVERIFY(result.ok());QCOMPARE(result.report.value("findings").toList().size(),0);QVERIFY(result.report.value("hasRejectedStructures").toBool());
 }
 void overlapAndEbrCycle(){
  auto b=mbr();b[466]=7;u32(b,470,10);u32(b,474,2);const auto overlap=mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",b));QVERIFY(overlap.ok());QCOMPARE(overlap.report.value("findings").toList().size(),1);QVERIFY(overlap.report.value("hasRejectedStructures").toBool());
  b=mbr();b[450]=15;u32(b,454,8);u32(b,458,40);const int p=8*512;b.replace(p+510,2,QByteArray::fromHex("55aa"));b[p+466]=15;u32(b,p+470,1);const int q=9*512;b.replace(q+510,2,QByteArray::fromHex("55aa"));b[q+466]=15;u32(b,q+470,1);
  const auto loop=mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",b));QVERIFY(loop.ok());QVERIFY(loop.report.value("warnings").toStringList().join(' ').contains("cycle"));
 }
 void boundedMutationCorpus(){
  std::mt19937 random(0x4e455856);for(int i=0;i<512;++i){auto b=i%2?mbr():gpt();for(int j=0;j<8;++j)b[int(random()%quint32(b.size()))]=char(random()&255);const auto r=mr::forensic::analyzeReadOnlyEvidence(request("dc-image-partitions",b));if(r.ok()){QVERIFY(r.report.value("findings").toList().size()<=128);QVERIFY(QJsonDocument::fromVariant(r.report).toJson(QJsonDocument::Compact).size()<=65536);}}
 }
 void authenticatedCaseReceiptAndRecovery(){
  QTemporaryDir dir;const QString password="Synthetic forensic integration 2026!";IdentityService identity(dir.filePath("identity"),true);QVERIFY(identity.bootstrap("admin","Synthetic admin",password));QVERIFY(identity.login("admin",password));QVERIFY(!identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty());QVERIFY(identity.login("investigator",password));
  GovernmentOperationPolicy policy(&identity);LawCaseBridge law(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);law.setOperationPolicy(&policy);
  const auto id=law.createCase(metadata());QVERIFY2(!id.isEmpty(),qPrintable(law.error()));const auto device=law.addDevice({{"model","Synthetic"},{"custodian","A"}});QVERIFY(!device.isEmpty());
  for(const auto &adapter:mr::forensic::forensicAdapters()){
   const auto bytes=adapter=="chromium-bookmarks"?bookmarks:adapter=="workbench-export-manifest"?manifest:mbr();QFile f(dir.filePath(adapter));QVERIFY(f.open(QIODevice::WriteOnly));QCOMPARE(f.write(bytes),bytes.size());f.close();const auto e=law.addEvidence(device,f.fileName(),"Synthetic source","A",true);QVERIFY2(!e.isEmpty(),qPrintable(law.error()));
   QVERIFY(law.analyzeForensicEvidence(e,adapter));QTRY_VERIFY_WITH_TIMEOUT(!law.forensicBusy(),10000);const auto result=law.forensicResult();QVERIFY2(!result.isEmpty(),qPrintable(law.error()));QCOMPARE(result.value("sourceSha256").toString(),sha(bytes));QVERIFY(!result.value("operationId").toString().isEmpty());
   const auto intent=policy.caseOperationStates().last().toMap();QVERIFY(!intent.isEmpty());QVERIFY(law.verifyAudit());QVERIFY(law.verifyEvidence(e));
  }
  const auto record=law.activeCase();const auto cancelled=std::make_shared<std::atomic_bool>(false);
  const auto artifacts=nexvary::law::projectArtifacts(record,{},cancelled);QVERIFY2(!artifacts.contains("errorCode"),qPrintable(artifacts.value("errorCode").toString()));QCOMPARE(artifacts.value("rows").toList().size(),3);
  const auto timeline=nexvary::law::projectTimeline(record,{},cancelled);QVERIFY2(!timeline.contains("errorCode"),qPrintable(timeline.value("errorCode").toString()));QCOMPARE(timeline.value("events").toList().size(),1);QCOMPARE(timeline.value("normalizedCount").toInt(),1);
  auto badRecord=record;auto runs=record.value("analyses").toList();auto badRun=runs[0].toMap();auto report=QJsonDocument::fromJson(badRun.value("reportJson").toString().toUtf8()).object().toVariantMap();report.insert("caseId","00000000-0000-4000-8000-000000000099");const auto changed=QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact);badRun.insert("reportJson",QString::fromUtf8(changed));badRun.insert("reportSha256",sha(changed));runs[0]=badRun;badRecord.insert("analyses",runs);QVERIFY(nexvary::law::projectArtifacts(badRecord,{},cancelled).contains("errorCode"));QVERIFY(nexvary::law::projectTimeline(badRecord,{},cancelled).contains("errorCode"));
  QCOMPARE(law.activeCase().value("analyses").toList().size(),3);QCOMPARE(policy.caseOperationStates().size(),3);const auto anchor=policy.auditStatus();
  LawCaseBridge restarted(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);restarted.setOperationPolicy(&policy);QVERIFY(restarted.openCase(id));QCOMPARE(restarted.activeCase().value("analyses").toList().size(),3);QCOMPARE(policy.auditStatus(),anchor);
 }
 void grantRevocationLogoutAndAuditFailure(){
  QTemporaryDir dir;const QString password="Synthetic forensic boundary 2026!";IdentityService identity(dir.filePath("identity"),true);QVERIFY(identity.bootstrap("admin","Synthetic admin",password));QVERIFY(identity.login("admin",password));
  const auto owner=identity.createUser("owner","Synthetic owner","investigator",password),peer=identity.createUser("peer","Synthetic peer","investigator",password);QVERIFY(!owner.isEmpty()&&!peer.isEmpty());QVERIFY(identity.login("owner",password));
  GovernmentOperationPolicy policy(&identity);LawCaseBridge law(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);law.setOperationPolicy(&policy);const auto id=law.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=law.addDevice({{"model","Synthetic"},{"custodian","A"}});
  QFile f(dir.filePath("Bookmarks"));QVERIFY(f.open(QIODevice::WriteOnly));f.write(bookmarks);f.close();const auto e=law.addEvidence(dev,f.fileName(),"Synthetic","A",true);QVERIFY(!e.isEmpty());
  QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));identity.logout();QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QVERIFY(identity.login("owner",password));QVERIFY(law.openCase(id));QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
  IdentityService administrator(dir.filePath("identity"),true);QVERIFY(administrator.login("admin",password));QVERIFY(administrator.setCaseGrant(id,peer,true));QVERIFY(identity.login("peer",password));QVERIFY(law.openCase(id));QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));QVERIFY(administrator.setCaseGrant(id,peer,false));QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());
  QVERIFY(identity.login("owner",password));QVERIFY(law.openCase(id));QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
  QFile journal(identity.policyJournalPath());QVERIFY(journal.open(QIODevice::Append));journal.write("X");journal.close();QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
 }
 void cancellationLogoutAndCrossCaseRefuseAcceptance(){
  QTemporaryDir dir;LawCaseBridge law(dir.filePath("cases.db"),nullptr,true);const auto id=law.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=law.addDevice({{"model","Synthetic"},{"custodian","A"}});QFile f(dir.filePath("Bookmarks"));QVERIFY(f.open(QIODevice::WriteOnly));f.write(bookmarks);f.close();const auto e=law.addEvidence(dev,f.fileName(),"Synthetic bookmarks","A",true);QVERIFY(!e.isEmpty());
  QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));law.cancelForensicAnalysis();QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
  const auto other=law.createCase(metadata());QVERIFY(!other.isEmpty());QVERIFY(!law.analyzeForensicEvidence(e,"chromium-bookmarks"));QVERIFY(law.openCase(id));QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));law.closeWorkspace();QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QVERIFY(law.openCase(id));QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
 }
};
QTEST_GUILESS_MAIN(ForensicAdapterTests)
#include "forensic_adapter_tests.moc"
