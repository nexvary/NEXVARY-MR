#include "ForensicAdapters.hpp"
#include "SqliteEvidenceFamily.hpp"
#include "CaseVault.hpp"
#include <QScopeGuard>
#include "CaseArtifacts.hpp"
#include "CaseTimeline.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>
#include <random>

namespace {
const QByteArray bookmarks=R"({"roots":{"bookmark_bar":{"type":"folder","name":"Lab","children":[{"type":"url","name":"Synthetic","url":"https://example.invalid/","date_added":"13300000000000000"}]}}})";
const QByteArray manifest=R"({"entries":[{"path":"evidence.bin","sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}]})";
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
mr::forensic::ReadOnlyEvidenceRequest request(const QString &adapter,const QByteArray &bytes){return {"00000000-0000-4000-8000-000000000001","00000000-0000-4000-8000-000000000002",sha(bytes),adapter,bytes,std::make_shared<std::atomic_bool>(false)};}
QVariantMap familyOracle(){QFile f(MR_SQLITE_FAMILY_REFERENCE_PATH);if(!f.open(QIODevice::ReadOnly))qFatal("Missing independent SQLite family oracle");return QJsonDocument::fromJson(f.readAll()).toVariant().toMap();}
QByteArray familyPart(const QString &key){return QByteArray::fromBase64(familyOracle().value(key).toString().toLatin1());}
mr::forensic::ReadOnlyEvidenceRequest familyRequest(const QString &walKey="walBase64"){
 auto r=request("browser-sqlite-family",familyPart("mainBase64"));const auto wal=familyPart(walKey),shm=familyPart("shmBase64");
 r.sqliteFamilyMembers.insert("wal",{"00000000-0000-4000-8000-000000000003",sha(wal),wal});r.sqliteFamilyMembers.insert("shm",{"00000000-0000-4000-8000-000000000004",sha(shm),shm});return r;
}
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
  QCOMPARE(mr::forensic::forensicAdapters().size(),4);
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
 void manifestPayloadVerification(){
  const QByteArray payload="synthetic package bytes";
  const auto bytes=QJsonDocument::fromVariant(QVariantMap{{"entries",QVariantList{QVariantMap{{"path","folder/evidence.bin"},{"sha256",sha(payload)}}}}}).toJson(QJsonDocument::Compact);
  auto r=request("workbench-export-manifest",bytes);r.verifyManifestPayloads=true;
  auto result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY(result.ok());QCOMPARE(result.report.value("missingFiles").toInt(),1);QVERIFY(!result.report.value("payloadVerified").toBool());
  r.manifestPayloads.insert("folder/evidence.bin",{"00000000-0000-4000-8000-000000000003",sha(payload),payload});
  result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY(result.ok());QVERIFY(result.report.value("payloadVerified").toBool());QCOMPARE(result.report.value("verifiedFiles").toInt(),1);
  QCOMPARE(QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact),QJsonDocument::fromVariant(mr::forensic::analyzeReadOnlyEvidence(r).report).toJson(QJsonDocument::Compact));
  const auto detail=result.report.value("findings").toList()[0].toMap().value("details").toMap();QCOMPARE(detail.value("originalSha256").toString(),sha(payload));QCOMPARE(detail.value("workingCopySha256").toString(),sha(payload));
  r.manifestPayloads["folder/evidence.bin"].bytes="tampered";
  QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("manifest-payload-hash-mismatch"));
  r.manifestPayloads["folder/evidence.bin"].sha256=sha("tampered");result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY(result.ok());QCOMPARE(result.report.value("tamperedFiles").toInt(),1);QVERIFY(!result.report.value("payloadVerified").toBool());
  r.manifestPayloads["folder/evidence.bin"].bytes=QByteArray(8*1024*1024+1,'x');QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("manifest-payload-limit"));
 }
 void manifestRawReferencesAndNoSymlinkResolution(){
  QTemporaryDir dir;QFile target(dir.filePath("outside-secret"));QVERIFY(target.open(QIODevice::WriteOnly));target.write("abc");target.close();
  const auto link=dir.filePath("linked-payload");
#ifdef Q_OS_WIN
  // QFile::link creates Windows shortcuts, not POSIX symlinks. A real local
  // matching file still proves that manifest paths are not opened on Windows.
  QVERIFY(QFile::copy(target.fileName(),link));
#else
  QVERIFY(QFile::link(target.fileName(),link));QVERIFY(QFileInfo(link).isSymLink());
#endif
  const auto originalDirectory=QDir::currentPath();QVERIFY(QDir::setCurrent(dir.path()));
  const QString abcHash="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
  const QVariantMap entry{{"path","linked-payload"},{"sha256",abcHash}};
  // A local file (POSIX: actual symlink) has the path and matching bytes.
  // Without a protected evidence binding it must stay missing, never opened.
  for(const auto &key:QStringList{"","files","evidence","entries"}){
   const auto bytes=QJsonDocument::fromVariant(key.isEmpty()?QVariant(QVariantList{entry}):QVariant(QVariantMap{{key,QVariantList{entry}}})).toJson();
   auto r=request("workbench-export-manifest",bytes);r.verifyManifestPayloads=true;
   auto result=mr::forensic::analyzeReadOnlyEvidence(r);
   // Restore the process cwd before assertions that can return from the test.
   const bool restored=QDir::setCurrent(originalDirectory);QVERIFY(restored);
   QVERIFY(result.ok());QCOMPARE(result.report.value("missingFiles").toInt(),1);QVERIFY(!result.report.value("payloadVerified").toBool());
   QCOMPARE(result.report.value("findings").toList()[0].toMap().value("reference").toString(),(key.isEmpty()?QString():"/"+key)+"/0");
   r.manifestPayloads.insert("linked-payload",{"00000000-0000-4000-8000-000000000003",abcHash,"abc"});
   result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY(result.ok());QVERIFY(result.report.value("payloadVerified").toBool());
   QVERIFY(QDir::setCurrent(dir.path()));
  }
  QVERIFY(QDir::setCurrent(originalDirectory));QVERIFY(target.open(QIODevice::ReadOnly));QCOMPARE(target.readAll(),QByteArray("abc"));
 }
 void manifestUnsafePathsAndDuplicateRejection(){
  for(const auto &path:QStringList{"../secret","/etc/passwd","C:/secret","a/../b","a/./b","a//b","a\\b","a:","a/","a.","NUL","COM1.txt",QString("a")+QChar(0)}){
   QVERIFY2(!mr::forensic::safeManifestPath(path),qPrintable(path));
   const auto bytes=QJsonDocument::fromVariant(QVariantList{QVariantMap{{"path",path},{"sha256",QString(64,'a')}}}).toJson();
   auto r=request("workbench-export-manifest",bytes);r.verifyManifestPayloads=true;QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("manifest-unsafe-path"));
  }
  for(const auto &path:QStringList{"a.bin","folder/a-1.bin"})QVERIFY(mr::forensic::safeManifestPath(path));
  auto r=request("workbench-export-manifest",QJsonDocument::fromVariant(QVariantList{QVariantMap{{"path","a"},{"sha256",QString(64,'a')}},QVariantMap{{"path","A"},{"sha256",QString(64,'a')}}}).toJson());r.verifyManifestPayloads=true;
  QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("manifest-duplicate-path"));
 }
 void manifestCasePayloadReceiptAndIsolation(){
  QTemporaryDir dir;LawCaseBridge law(dir.filePath("cases.db"),nullptr,true);const auto id=law.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=law.addDevice({{"model","Synthetic"},{"custodian","A"}});
  const QByteArray payload="synthetic protected payload";
  QFile f(dir.filePath("payload"));QVERIFY(f.open(QIODevice::WriteOnly));f.write(payload);f.close();const auto e=law.addEvidence(dev,f.fileName(),"Synthetic payload","A",true);QVERIFY(!e.isEmpty());
  const auto bytes=QJsonDocument::fromVariant(QVariantList{QVariantMap{{"path","payload.bin"},{"sha256",sha(payload)}}}).toJson();
  QFile m(dir.filePath("manifest"));QVERIFY(m.open(QIODevice::WriteOnly));m.write(bytes);m.close();const auto manifestId=law.addEvidence(dev,m.fileName(),"Synthetic manifest","A",true);QVERIFY(!manifestId.isEmpty());
  QVERIFY(!law.verifyManifestEvidence(manifestId,{{"../payload",e}}));
  QVERIFY(law.verifyManifestEvidence(manifestId,{{"payload.bin",e}}));QTRY_VERIFY(!law.forensicBusy());const auto result=law.forensicResult();QVERIFY2(!result.isEmpty(),qPrintable(law.error()));
  const auto report=QJsonDocument::fromJson(result.value("reportJson").toString().toUtf8()).object();QVERIFY(report.value("payloadVerified").toBool());QVERIFY(law.verifyAudit());
  QVariantMap source;for(const auto &entry:law.activeCase().value("evidence").toList())if(entry.toMap().value("id")==manifestId)source=entry.toMap();
  QVERIFY(mr::forensic::retainedForensicReport(result,source,id).ok());
  for(const auto &field:QStringList{"verifiedFiles","analysisParameters"}){
   auto forgedReport=report.toVariantMap();if(field=="verifiedFiles")forgedReport.insert(field,99);else forgedReport.remove(field);
   const auto forgedBytes=QJsonDocument::fromVariant(forgedReport).toJson(QJsonDocument::Compact);auto forged=result;forged.insert("reportJson",QString::fromUtf8(forgedBytes));forged.insert("reportSha256",sha(forgedBytes));
   QVERIFY(!mr::forensic::retainedForensicReport(forged,source,id).ok());
  }
  const auto projected=nexvary::law::projectArtifacts(law.activeCase(),{},std::make_shared<std::atomic_bool>(false));QVERIFY(!projected.contains("errorCode"));QCOMPARE(projected.value("rows").toList().size(),1);
  const auto count=law.activeCase().value("analyses").toList().size();
  QVERIFY(law.verifyManifestEvidence(manifestId,{{"payload.bin",e}}));law.cancelForensicAnalysis();QTRY_VERIFY(!law.forensicBusy());QCOMPARE(law.activeCase().value("analyses").toList().size(),count);
  const auto other=law.createCase(metadata());QVERIFY(!other.isEmpty());QVERIFY(!law.verifyManifestEvidence(manifestId,{{"payload.bin",e}}));QVERIFY(law.openCase(id));
  LawCaseBridge restart(dir.filePath("cases.db"),nullptr,true);QVERIFY(restart.openCase(id));QCOMPARE(restart.activeCase().value("analyses").toList().size(),count);
 }
 void sqliteFamilyRuntimeOracleAndEndianness(){
  for(const auto &variant:QStringList{"walBase64","bigEndianWalBase64","uncommittedWalBase64"}){
   const auto r=familyRequest(variant);const auto before=r.verifiedBytes;const auto wal=r.sqliteFamilyMembers.value("wal").bytes;
   const auto snapshot=mr::forensic::inspectSqliteEvidenceFamily(r,{});QVERIFY2(snapshot.analysis.ok(),qPrintable(snapshot.analysis.errorCode));QCOMPARE(snapshot.committedPages,familyPart("snapshotBase64"));
   const auto result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY2(result.ok(),qPrintable(result.errorCode));const auto summary=result.report.value("sqliteFamily").toMap();
   QCOMPARE(summary.value("snapshotSha256").toString(),familyOracle().value("snapshotSha256").toString());QCOMPARE(summary.value("commitFrames").toInt(),2);QCOMPARE(summary.value("shmUsedForReplay").toBool(),false);
   QCOMPARE(summary.value("uncommittedFrames").toInt(),variant=="uncommittedWalBase64"?1:0);QCOMPARE(r.verifiedBytes,before);QCOMPARE(r.sqliteFamilyMembers.value("wal").bytes,wal);
   QCOMPARE(QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact),QJsonDocument::fromVariant(mr::forensic::analyzeReadOnlyEvidence(r).report).toJson(QJsonDocument::Compact));
  }
  auto r=familyRequest();auto &shm=r.sqliteFamilyMembers["shm"];shm.bytes=QByteArray(32,42);shm.sha256=sha(shm.bytes);const auto ignored=mr::forensic::inspectSqliteEvidenceFamily(r,{});QVERIFY(ignored.analysis.ok());QCOMPARE(ignored.committedPages,familyPart("snapshotBase64"));
  r.sqliteFamilyMembers.clear();const auto missing=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY(missing.ok());QCOMPARE(missing.report.value("sqliteFamily").toMap().value("snapshotSha256").toString(),sha(r.verifiedBytes));QVERIFY(missing.report.value("warnings").toStringList().join(' ').contains("missing sidecar"));
 }
 void sqliteFamilyMalformedAndBounds(){
  auto r=familyRequest();r.sqliteFamilyMembers["wal"].bytes[70]^=1;QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("sqlite-family-member-hash"));
  const auto damaged=[&](int offset,const QString &expected){auto q=familyRequest();auto &w=q.sqliteFamilyMembers["wal"];w.bytes[offset]^=1;w.sha256=sha(w.bytes);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(q).errorCode,expected);};
  damaged(24,"sqlite-family-wal-header-checksum");damaged(40,"sqlite-family-frame-salt");damaged(70,"sqlite-family-frame-checksum");
  r=familyRequest();auto &w=r.sqliteFamilyMembers["wal"];w.bytes.chop(1);w.sha256=sha(w.bytes);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("sqlite-family-incomplete-wal"));
  for(const auto &variant:QStringList{"missingGrowthWalBase64","shrinkGrowWalBase64"})QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(familyRequest(variant)).errorCode,QString("sqlite-family-missing-committed-page"));
  r=familyRequest();r.sqliteFamilyMembers["wal"].evidenceId=r.evidenceId;QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("sqlite-family-member-provenance"));
  r=familyRequest();r.sqliteFamilyMembers.remove("wal");QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("sqlite-family-orphan-shm"));
  r=familyRequest();r.sqliteFamilyMembers["wal"].bytes=QByteArray(8*1024*1024,'x');QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("sqlite-family-input-limit"));
  r=familyRequest();auto &many=r.sqliteFamilyMembers["wal"];many.bytes=many.bytes.left(32)+QByteArray(4097*(1024+24),0);many.sha256=sha(many.bytes);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("sqlite-family-frame-limit"));
  r=familyRequest();r.cancelled->store(true);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("cancelled"));r.cancelled->store(false);r.deadline=QDeadlineTimer(0);QCOMPARE(mr::forensic::analyzeReadOnlyEvidence(r).errorCode,QString("worker-timeout"));
  for(const auto &b:QList<QByteArray>{QByteArray("SQLite format 3"),familyPart("mainBase64").chopped(1)})QVERIFY(!mr::forensic::analyzeReadOnlyEvidence(request("browser-sqlite-family",b)).ok());
 }
 void sqliteFamilyBoundedMutations(){
  std::mt19937 random(0x534c514c);for(int i=0;i<256;++i){auto r=familyRequest();auto &w=r.sqliteFamilyMembers["wal"];w.bytes[int(random()%quint32(w.bytes.size()))]^=char(1u<<(random()%8));w.sha256=sha(w.bytes);const auto result=mr::forensic::analyzeReadOnlyEvidence(r);if(result.ok()){QVERIFY(result.report.value("findings").toList().size()==1);QVERIFY(result.report.value("sqliteFamily").toMap().value("snapshotBytes").toInt()<=8*1024*1024);}}
 }
 void sqliteFamilyCaseBindingsRetentionAndCancellation(){
  QTemporaryDir dir;const QString password="Synthetic SQLite family 2026!";IdentityService identity(dir.filePath("identity"),true);QVERIFY(identity.bootstrap("admin","Synthetic administrator",password));QVERIFY(identity.login("admin",password));QVERIFY(!identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty());QVERIFY(identity.login("investigator",password));
  GovernmentOperationPolicy policy(&identity);LawCaseBridge law(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);law.setOperationPolicy(&policy);const auto id=law.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=law.addDevice({{"model","Synthetic"},{"custodian","A"}});
  const auto add=[&](const QString &name,const QByteArray &bytes){QFile f(dir.filePath(name));if(!f.open(QIODevice::WriteOnly)||f.write(bytes)!=bytes.size())return QString();f.close();return law.addEvidence(dev,f.fileName(),"Synthetic SQLite member","A",true);};
  const auto main=add("History",familyPart("mainBase64")),wal=add("History-wal",familyPart("walBase64")),shm=add("History-shm",familyPart("shmBase64"));QVERIFY(!main.isEmpty()&&!wal.isEmpty()&&!shm.isEmpty());
  QVERIFY(law.analyzeBrowserSqliteFamily(main,wal,shm));QTRY_VERIFY(!law.forensicBusy());const auto run=law.forensicResult();QVERIFY2(!run.isEmpty(),qPrintable(law.error()));QVERIFY(!run.value("operationId").toString().isEmpty());QVERIFY(law.verifyAudit());
  const auto record=law.activeCase();QMap<QString,QVariantMap> inventory;for(const auto &v:record.value("evidence").toList())inventory.insert(v.toMap().value("id").toString(),v.toMap());
  QVERIFY(mr::forensic::retainedForensicReport(run,inventory.value(main),id,inventory).ok());auto foreign=inventory;foreign.remove(wal);QVERIFY(!mr::forensic::retainedForensicReport(run,inventory.value(main),id,foreign).ok());
  const auto projected=nexvary::law::projectArtifacts(record,{},std::make_shared<std::atomic_bool>(false));QVERIFY(!projected.contains("errorCode"));QCOMPARE(projected.value("rows").toList().size(),1);
  const auto timeline=nexvary::law::projectTimeline(record,{},std::make_shared<std::atomic_bool>(false));QVERIFY(!timeline.contains("errorCode"));QCOMPARE(timeline.value("events").toList().size(),0);
  auto changedReport=QJsonDocument::fromJson(run.value("reportJson").toString().toUtf8()).toVariant().toMap();auto summary=changedReport.value("sqliteFamily").toMap();auto raw=summary.value("rawArtifact").toMap();raw.insert("byteOffset",0);summary.insert("rawArtifact",raw);changedReport.insert("sqliteFamily",summary);
  const auto forgedBytes=QJsonDocument::fromVariant(changedReport).toJson(QJsonDocument::Compact);auto forged=run;forged.insert("reportJson",QString::fromUtf8(forgedBytes));forged.insert("reportSha256",sha(forgedBytes));QVERIFY(!mr::forensic::retainedForensicReport(forged,inventory.value(main),id,inventory).ok());
  // Completion is queued to this thread. Change only the auxiliary protected
  // BLOB after dispatch, then keep it changed until callback revalidation runs.
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","sqlite-family-tamper");db.setDatabaseName(dir.filePath("cases.db"));QVERIFY(db.open());const auto closeDb=qScopeGuard([&db]{db.close();});QSqlQuery q(db);q.prepare("SELECT payload FROM law_evidence WHERE id=? AND case_id=?");q.addBindValue(wal);q.addBindValue(id);QVERIFY(q.exec()&&q.next());const auto original=q.value(0).toByteArray();q.finish();
   QString vaultError;const auto plain=mr::recovery::openCaseBytes(original,true,vaultError);QCOMPARE(plain,familyPart("walBase64"));
   const auto trailer=mr::recovery::openCaseBytes(original+"tampered",true,vaultError);qInfo()<<"Envelope trailer preserves decoded bytes:"<<(trailer==plain);
   auto altered=plain;altered[70]^=1;QVERIFY(sha(altered)!=sha(plain));const auto sealed=mr::recovery::sealCase(altered,true,vaultError);QVERIFY2(!sealed.isEmpty(),qPrintable(vaultError));
   QVERIFY(law.analyzeBrowserSqliteFamily(main,wal,shm));q.prepare("UPDATE law_evidence SET payload=? WHERE id=? AND case_id=?");q.addBindValue(sealed);q.addBindValue(wal);q.addBindValue(id);QVERIFY(q.exec());QVERIFY(!law.verifyEvidence(wal));QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QCOMPARE(law.activeCase().value("analyses").toList().size(),1);
   q.prepare("UPDATE law_evidence SET payload=? WHERE id=? AND case_id=?");q.addBindValue(original);q.addBindValue(wal);q.addBindValue(id);QVERIFY(q.exec());db.close();
  }
  QSqlDatabase::removeDatabase("sqlite-family-tamper");QVERIFY(law.verifyEvidence(wal));
  QVERIFY(law.analyzeBrowserSqliteFamily(main,wal,shm));law.cancelForensicAnalysis();QTRY_VERIFY(!law.forensicBusy());QCOMPARE(law.activeCase().value("analyses").toList().size(),1);
  const auto other=law.createCase(metadata());QVERIFY(!other.isEmpty());const auto otherDev=law.addDevice({{"model","Other"},{"custodian","A"}});QVERIFY(!otherDev.isEmpty());QVERIFY(!law.analyzeBrowserSqliteFamily(main,wal,shm));
  QFile otherFile(dir.filePath("Other-main"));QVERIFY(otherFile.open(QIODevice::WriteOnly));otherFile.write(familyPart("mainBase64"));otherFile.close();const auto otherMain=law.addEvidence(otherDev,otherFile.fileName(),"Other synthetic main","A",true);QVERIFY(!otherMain.isEmpty());QVERIFY(!law.analyzeBrowserSqliteFamily(otherMain,wal,{}));QVERIFY(law.openCase(id));
  const auto anchor=policy.auditStatus();LawCaseBridge restart(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);restart.setOperationPolicy(&policy);QVERIFY(restart.openCase(id));QCOMPARE(restart.activeCase().value("analyses").toList().size(),1);QVERIFY(restart.verifyAudit());QCOMPARE(policy.auditStatus(),anchor);
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
   const auto bytes=adapter=="chromium-bookmarks"?bookmarks:adapter=="workbench-export-manifest"?manifest:adapter=="browser-sqlite-family"?familyPart("mainBase64"):mbr();QFile f(dir.filePath(adapter));QVERIFY(f.open(QIODevice::WriteOnly));QCOMPARE(f.write(bytes),bytes.size());f.close();const auto e=law.addEvidence(device,f.fileName(),"Synthetic source","A",true);QVERIFY2(!e.isEmpty(),qPrintable(law.error()));
   QVERIFY(law.analyzeForensicEvidence(e,adapter));QTRY_VERIFY_WITH_TIMEOUT(!law.forensicBusy(),10000);const auto result=law.forensicResult();QVERIFY2(!result.isEmpty(),qPrintable(law.error()));QCOMPARE(result.value("sourceSha256").toString(),sha(bytes));QVERIFY(!result.value("operationId").toString().isEmpty());
   const auto intent=policy.caseOperationStates().last().toMap();QVERIFY(!intent.isEmpty());QVERIFY(law.verifyAudit());QVERIFY(law.verifyEvidence(e));
  }
  const auto record=law.activeCase();const auto cancelled=std::make_shared<std::atomic_bool>(false);
  const auto artifacts=nexvary::law::projectArtifacts(record,{},cancelled);QVERIFY2(!artifacts.contains("errorCode"),qPrintable(artifacts.value("errorCode").toString()));QCOMPARE(artifacts.value("rows").toList().size(),4);
  const auto timeline=nexvary::law::projectTimeline(record,{},cancelled);QVERIFY2(!timeline.contains("errorCode"),qPrintable(timeline.value("errorCode").toString()));QCOMPARE(timeline.value("events").toList().size(),1);QCOMPARE(timeline.value("normalizedCount").toInt(),1);
  auto badRecord=record;auto runs=record.value("analyses").toList();auto badRun=runs[0].toMap();auto report=QJsonDocument::fromJson(badRun.value("reportJson").toString().toUtf8()).object().toVariantMap();report.insert("caseId","00000000-0000-4000-8000-000000000099");const auto changed=QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact);badRun.insert("reportJson",QString::fromUtf8(changed));badRun.insert("reportSha256",sha(changed));runs[0]=badRun;badRecord.insert("analyses",runs);QVERIFY(nexvary::law::projectArtifacts(badRecord,{},cancelled).contains("errorCode"));QVERIFY(nexvary::law::projectTimeline(badRecord,{},cancelled).contains("errorCode"));
  QCOMPARE(law.activeCase().value("analyses").toList().size(),4);QCOMPARE(policy.caseOperationStates().size(),4);const auto anchor=policy.auditStatus();
  LawCaseBridge restarted(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);restarted.setOperationPolicy(&policy);QVERIFY(restarted.openCase(id));QCOMPARE(restarted.activeCase().value("analyses").toList().size(),4);QCOMPARE(policy.auditStatus(),anchor);
 }
 void grantRevocationLogoutAndAuditFailure_data(){
  QTest::addColumn<int>("mode");QTest::newRow("bookmarks")<<0;QTest::newRow("manifest-content")<<1;QTest::newRow("sqlite-family")<<2;
 }
 void grantRevocationLogoutAndAuditFailure(){
  QFETCH(int,mode);
  QTemporaryDir dir;const QString password="Synthetic forensic boundary 2026!";IdentityService identity(dir.filePath("identity"),true);QVERIFY(identity.bootstrap("admin","Synthetic admin",password));QVERIFY(identity.login("admin",password));
  const auto owner=identity.createUser("owner","Synthetic owner","investigator",password),peer=identity.createUser("peer","Synthetic peer","investigator",password);QVERIFY(!owner.isEmpty()&&!peer.isEmpty());QVERIFY(identity.login("owner",password));
  GovernmentOperationPolicy policy(&identity);LawCaseBridge law(dir.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);law.setOperationPolicy(&policy);const auto id=law.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=law.addDevice({{"model","Synthetic"},{"custodian","A"}});
  QFile f(dir.filePath("Bookmarks"));QVERIFY(f.open(QIODevice::WriteOnly));f.write(mode==1?manifest:mode==2?familyPart("mainBase64"):bookmarks);f.close();const auto e=law.addEvidence(dev,f.fileName(),"Synthetic","A",true);QVERIFY(!e.isEmpty());
  const auto start=[&]{return mode==1?law.verifyManifestEvidence(e,{{"evidence.bin",e}}):mode==2?law.analyzeBrowserSqliteFamily(e,{},{}):law.analyzeForensicEvidence(e,"chromium-bookmarks");};
  QVERIFY(start());identity.logout();QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QVERIFY(identity.login("owner",password));QVERIFY(law.openCase(id));QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
  IdentityService administrator(dir.filePath("identity"),true);QVERIFY(administrator.login("admin",password));QVERIFY(administrator.setCaseGrant(id,peer,true));QVERIFY(identity.login("peer",password));QVERIFY(law.openCase(id));QVERIFY(start());QVERIFY(administrator.setCaseGrant(id,peer,false));QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());
  QVERIFY(identity.login("owner",password));QVERIFY(law.openCase(id));QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
  QFile journal(identity.policyJournalPath());QVERIFY(journal.open(QIODevice::Append));journal.write("X");journal.close();QVERIFY(start());QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
 }
 void cancellationLogoutAndCrossCaseRefuseAcceptance(){
  QTemporaryDir dir;LawCaseBridge law(dir.filePath("cases.db"),nullptr,true);const auto id=law.createCase(metadata());QVERIFY(!id.isEmpty());const auto dev=law.addDevice({{"model","Synthetic"},{"custodian","A"}});QFile f(dir.filePath("Bookmarks"));QVERIFY(f.open(QIODevice::WriteOnly));f.write(bookmarks);f.close();const auto e=law.addEvidence(dev,f.fileName(),"Synthetic bookmarks","A",true);QVERIFY(!e.isEmpty());
  QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));law.cancelForensicAnalysis();QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
  const auto other=law.createCase(metadata());QVERIFY(!other.isEmpty());QVERIFY(!law.analyzeForensicEvidence(e,"chromium-bookmarks"));QVERIFY(law.openCase(id));QVERIFY(law.analyzeForensicEvidence(e,"chromium-bookmarks"));law.closeWorkspace();QTRY_VERIFY(!law.forensicBusy());QVERIFY(law.forensicResult().isEmpty());QVERIFY(law.openCase(id));QCOMPARE(law.activeCase().value("analyses").toList().size(),0);
 }
};
QTEST_GUILESS_MAIN(ForensicAdapterTests)
#include "forensic_adapter_tests.moc"
