#include "SqliteTableReader.hpp"
#include "ChromiumHistoryReader.hpp"
#include "ChromiumDownloadsReader.hpp"
#include "ChromiumDownloads.hpp"
#include "ChromiumHistory.hpp"
#include "SqliteEvidenceFamily.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>
#include <limits>
#include <bit>
#include <random>

namespace {
QString sha(const QByteArray &bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
mr::forensic::ReadOnlyEvidenceRequest request(const QByteArray &bytes){
 return {"00000000-0000-4000-8000-000000000001","00000000-0000-4000-8000-000000000002",sha(bytes),"browser-sqlite-family",bytes,std::make_shared<std::atomic_bool>(false)};
}
void be32(QByteArray &bytes,int offset,quint32 value){qToBigEndian(value,reinterpret_cast<uchar*>(bytes.data()+offset));}
}
class SqliteTableTests:public QObject {
 Q_OBJECT
private slots:


 void downloadsRuntimeCases_data(){
  QTest::addColumn<int>("mode");
  const char *names[]={"absent-chain-table","reordered-columns","orphan-chain","chain-gap","negative-index","unknown-column","blob-url","text-size","null-target","row-limit","per-chain-index-limit","missing-composite-key","reversed-composite-key","duplicate-chain-index","output-limit","overflow-paths","existing-target-untouched","empty-tables","chain-table-without-rowid","chain-row-limit","shared-table-root"};
  for(int i=0;i<21;++i)QTest::newRow(names[i])<<i;
 }
 void downloadsRuntimeCases(){
  QFETCH(int,mode);QTemporaryDir temp;QVERIFY(temp.isValid());const auto path=temp.filePath("History"),marker=temp.filePath("untouched-file.bin");
  {QFile f(marker);QVERIFY(f.open(QIODevice::WriteOnly));QCOMPARE(f.write("untouched-content"),qint64(17));}
  mr::forensic::ReadOnlyEvidenceRequest r;QList<QVariantList> expected;
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","downloads-runtime");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
   QVERIFY(q.exec("PRAGMA page_size=512"));
   QVERIFY(q.exec(mode==1?"CREATE TABLE downloads(target_path TEXT,state INTEGER,id INTEGER PRIMARY KEY,total_bytes INTEGER,current_path TEXT,start_time INTEGER,end_time INTEGER,received_bytes INTEGER)":"CREATE TABLE downloads(id INTEGER PRIMARY KEY,current_path TEXT,target_path TEXT,start_time INTEGER,end_time INTEGER,received_bytes INTEGER,total_bytes INTEGER,state INTEGER)"));
   if(mode==5)QVERIFY(q.exec("ALTER TABLE downloads ADD COLUMN unknown INTEGER"));
   if(mode!=0){
    QString key=mode==11||mode==13?QString{}:mode==12?",PRIMARY KEY(chain_index,id)":",PRIMARY KEY(id,chain_index)";
    QVERIFY(q.exec("CREATE TABLE downloads_url_chains(id INTEGER NOT NULL,chain_index INTEGER NOT NULL,url LONGVARCHAR NOT NULL"+key+")"+(mode==18?" WITHOUT ROWID":"")));
    if(mode==20){QVERIFY(q.exec("PRAGMA writable_schema=ON"));QVERIFY(q.exec("UPDATE sqlite_master SET rootpage=(SELECT rootpage FROM sqlite_master WHERE name='downloads') WHERE name='downloads_url_chains'"));}
    if(mode==13){
     QVERIFY(q.exec("PRAGMA writable_schema=ON"));
     QVERIFY(q.exec("UPDATE sqlite_master SET sql='CREATE TABLE downloads_url_chains(id INTEGER NOT NULL,chain_index INTEGER NOT NULL,url LONGVARCHAR NOT NULL,PRIMARY KEY(id,chain_index))' WHERE name='downloads_url_chains'"));
    }
   }
   if(mode!=17){
    QVERIFY(q.prepare("INSERT INTO downloads(id,current_path,target_path,start_time,end_time,received_bytes,total_bytes,state) VALUES(?,?,?,?,?,?,?,?)"));
    const int count=mode==9?129:mode==14?40:mode==19?9:1;
    for(int i=0;i<count;++i){
     q.bindValue(0,i+1);q.bindValue(1,mode==15?QString(1500,QChar(0x0639)):QString("../unresolved-part.bin"));
     q.bindValue(2,mode==8?QVariant{}:QVariant(mode==16?marker:mode==14?QString(2000,'x'):QString("C:\\synthetic\\target.bin")));
     q.bindValue(3,Q_INT64_C(11644473600000001));q.bindValue(4,std::numeric_limits<qint64>::max());
     q.bindValue(5,mode==7?QVariant(QString("wrong-storage-type")):QVariant(Q_INT64_C(9007199254740993)));q.bindValue(6,-1);q.bindValue(7,4);QVERIFY(q.exec());
    }
    if(mode!=0){
     QVERIFY(q.prepare("INSERT INTO downloads_url_chains VALUES(?,?,?)"));
     const int chains=mode==13?2:mode==19?513:1;
     for(int i=0;i<chains;++i){q.bindValue(0,mode==2?999:mode==19?i/64+1:1);q.bindValue(1,mode==3?1:mode==4?-1:mode==10?64:mode==13?0:mode==19?i%64:0);q.bindValue(2,mode==6?QVariant(QByteArray("https://blob.invalid/")):QVariant(QString("https://source.invalid/file")));QVERIFY(q.exec());}
    }
   }
   QVERIFY(q.exec("SELECT id,current_path,target_path,start_time,end_time,received_bytes,total_bytes,state FROM downloads ORDER BY id"));while(q.next()){QVariantList row;for(int i=0;i<8;++i)row.append(q.value(i));expected.append(row);}q.finish();db.close();
  }
  QSqlDatabase::removeDatabase("downloads-runtime");QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));r=request(f.readAll());
  const auto result=mr::forensic::readProtectedChromiumDownloads(r);
  const QMap<int,QString> errors{{2,"downloads-orphan-chain"},{3,"downloads-chain-gap"},{4,"downloads-chain-index-or-url"},{5,"downloads-schema-unsupported"},{6,"downloads-chain-record-type-or-limit"},{7,"downloads-record-type-or-limit"},{8,"downloads-record-type-or-limit"},{9,"downloads-result-limit"},{10,"downloads-chain-index-or-url"},{11,"downloads-schema-unsupported"},{12,"downloads-schema-unsupported"},{13,"downloads-chain-index-or-url"},{14,"downloads-output-limit"},{18,"downloads-schema-unsupported"},{19,"downloads-chain-result-limit"},{20,"downloads-schema-shared-root"}};
  if(errors.contains(mode)){QCOMPARE(result.errorCode,errors.value(mode));QVERIFY(result.report.isEmpty());return;}
  QVERIFY2(result.ok(),qPrintable(result.errorCode));const auto rows=result.report.value("observations").toList();QCOMPARE(rows.size(),expected.size());
  for(int i=0;i<rows.size();++i){const auto row=rows[i].toMap(),fields=row.value("fields").toMap();const auto e=expected[i];
   QCOMPARE(fields.value("id").toString(),e[0].toString());QCOMPARE(fields.value("current_path"),e[1]);QCOMPARE(fields.value("target_path"),e[2]);
   QCOMPARE(fields.value("received_bytes").toString(),QString("9007199254740993"));QCOMPARE(fields.value("total_bytes").toString(),QString("-1"));
   const auto times=row.value("times").toMap();QCOMPARE(times.value("start_time").toMap().value("unixMicroseconds").toString(),QString("1"));
   QCOMPARE(times.value("end_time").toMap().value("unixMicroseconds").toString(),QString::number(std::numeric_limits<qint64>::max()-Q_INT64_C(11644473600000000)));
   if(mode==0){QCOMPARE(row.value("chainStatus").toString(),QString("table-not-supplied"));QVERIFY(row.value("urlChain").toList().isEmpty());}
   if(mode==15){QVERIFY(row.value("sourceExtents").toList().size()>3);const auto family=mr::forensic::inspectSqliteEvidenceFamily(r,{});for(const auto &v:row.value("sourceExtents").toList()){const auto x=v.toMap();QCOMPARE(r.verifiedBytes.mid(x.value("byteOffset").toLongLong(),x.value("lengthBytes").toLongLong()),family.committedPages.mid(x.value("imageByteOffset").toLongLong(),x.value("lengthBytes").toLongLong()));}}
  }
  QFile m(marker);QVERIFY(m.open(QIODevice::ReadOnly));QCOMPARE(m.readAll(),QByteArray("untouched-content"));
 }
 void downloadsWalOracle(){
  QFile f(MR_DOWNLOADS_REFERENCE_PATH);QVERIFY(f.open(QIODevice::ReadOnly));const auto fixture=QJsonDocument::fromJson(f.readAll()).toVariant().toMap();
  auto r=request(QByteArray::fromBase64(fixture.value("mainBase64").toByteArray()));
  for(const auto &role:QStringList{"wal","shm"}){const auto bytes=QByteArray::fromBase64(fixture.value(role+"Base64").toByteArray());r.sqliteFamilyMembers.insert(role,{role=="wal"?"00000000-0000-4000-8000-000000000003":"00000000-0000-4000-8000-000000000004",sha(bytes),bytes});}
  auto adapterRequest=r;adapterRequest.adapter="chromium-downloads";
  const auto signedInput=mr::forensic::analyzeReadOnlyEvidence(adapterRequest);QVERIFY2(signedInput.ok(),qPrintable(signedInput.errorCode));
  QMap<QString,QVariantMap> inventory{{r.evidenceId,{{"id",r.evidenceId},{"sha256",r.sourceSha256},{"size",r.verifiedBytes.size()}}}};
  for(const auto &member:r.sqliteFamilyMembers)inventory.insert(member.evidenceId,{{"id",member.evidenceId},{"sha256",member.sha256},{"size",member.bytes.size()}});
  QVERIFY(mr::forensic::validateRetainedChromiumDownloads(signedInput.report,inventory).ok());
  auto forged=signedInput.report;forged.insert("chainRows",999);QVERIFY(!mr::forensic::validateRetainedChromiumDownloads(forged,inventory).ok());
  const auto before=r.verifiedBytes,wal=r.sqliteFamilyMembers["wal"].bytes;
  const auto parsed=mr::forensic::readProtectedChromiumDownloads(r);QVERIFY2(parsed.ok(),qPrintable(parsed.errorCode));
  QCOMPARE(parsed.report,mr::forensic::readProtectedChromiumDownloads(r).report);
  const auto family=mr::forensic::inspectSqliteEvidenceFamily(r,{});QVERIFY(family.analysis.ok());
  QCOMPARE(sha(family.committedPages),fixture.value("snapshotSha256").toString());
  const auto observations=parsed.report.value("observations").toList(),expected=fixture.value("expected").toList();QCOMPARE(observations.size(),expected.size());
  bool fromMain=false,fromWal=false;
  const auto references=[&](const QVariantList &refs){
   QVERIFY(!refs.isEmpty());
   for(const auto &v:refs){const auto x=v.toMap();const bool w=x.value("role")=="wal";fromWal|=w;fromMain|=!w;const auto &bytes=w?wal:r.verifiedBytes;
    const auto off=x.value("byteOffset").toLongLong(),len=x.value("lengthBytes").toLongLong();QVERIFY(off>=0&&len>0&&off+len<=bytes.size());
    QCOMPARE(x.value("sourceSha256").toString(),sha(bytes));QCOMPARE(bytes.mid(off,len),family.committedPages.mid(x.value("imageByteOffset").toLongLong(),len));
   }
  };
  for(int i=0;i<observations.size();++i){const auto o=observations[i].toMap();QCOMPARE(o.value("fields").toMap(),expected[i].toMap());references(o.value("sourceExtents").toList());
   QCOMPARE(o.value("times").toMap().value("start_time").toMap().value("unixMicroseconds").toString(),QString("1700000000123456"));
   const auto end=o.value("times").toMap().value("end_time").toMap();QCOMPARE(end.value("status").toString(),QString("unset-zero"));QVERIFY(!end.contains("unixMicroseconds"));
   const auto chains=o.value("urlChain").toList();if(i==0){QCOMPARE(chains.size(),2);for(int j=0;j<chains.size();++j){const auto c=chains[j].toMap();const auto e=fixture.value("expectedChains").toList()[j].toList();QCOMPARE(c.value("rowid").toString(),e[0].toString());QCOMPARE(c.value("chainIndex").toString(),e[2].toString());QCOMPARE(c.value("url").toString(),e[3].toString());references(c.value("sourceExtents").toList());}}else QVERIFY(chains.isEmpty());
  }
  for(const auto &t:parsed.report.value("downloadsSchema").toMap())references(t.toMap().value("schemaSourceExtents").toList());
  QVERIFY(fromMain);QVERIFY(fromWal);QCOMPARE(r.verifiedBytes,before);QCOMPARE(r.sqliteFamilyMembers["wal"].bytes,wal);
  r.sqliteFamilyMembers["wal"].bytes[40]^=1;const auto bad=mr::forensic::readProtectedChromiumDownloads(r);QVERIFY(!bad.ok());QVERIFY(bad.report.isEmpty());
  auto image=request(family.committedPages);std::mt19937 mutations(0x444f574e);
  for(int i=0;i<256;++i){auto b=image.verifiedBytes;b[int(mutations()%quint32(b.size()))]^=char(1u<<(mutations()%8));auto mutated=request(b);mutated.adapter="chromium-downloads";const auto result=mr::forensic::analyzeReadOnlyEvidence(mutated);if(!result.ok())QVERIFY(result.report.isEmpty());else QVERIFY(QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact).size()<=65536);}
  image.cancelled->store(true);QCOMPARE(mr::forensic::readProtectedChromiumDownloads(image).errorCode,QString("cancelled"));image.cancelled->store(false);image.deadline=QDeadlineTimer(0);QVERIFY(!mr::forensic::readProtectedChromiumDownloads(image).ok());
 }
 void historyWalOracleAndProvenance(){
  QFile f(MR_HISTORY_REFERENCE_PATH);QVERIFY(f.open(QIODevice::ReadOnly));const auto fixture=QJsonDocument::fromJson(f.readAll()).toVariant().toMap();
  auto r=request(QByteArray::fromBase64(fixture.value("mainBase64").toByteArray()));r.adapter="chromium-history";
  const auto wal=QByteArray::fromBase64(fixture.value("walBase64").toByteArray());r.sqliteFamilyMembers.insert("wal",{"00000000-0000-4000-8000-000000000003",sha(wal),wal});
  const auto parsed=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY2(parsed.ok(),qPrintable(parsed.errorCode));
  QCOMPARE(parsed.report,mr::forensic::analyzeReadOnlyEvidence(r).report);
  const auto rows=parsed.report.value("findings").toList(),expected=fixture.value("expected").toList();QCOMPARE(rows.size(),expected.size());
  bool walUsed=false;
  const auto snapshot=mr::forensic::inspectSqliteEvidenceFamily(r,{});QVERIFY(snapshot.analysis.ok());
  for(int i=0;i<rows.size();++i){
   const auto finding=rows[i].toMap(),d=finding.value("details").toMap();const auto e=expected[i].toList();
   QCOMPARE(d.value("visitRecord").toMap().value("rowid").toString(),e[0].toString());QCOMPARE(d.value("urlId").toString(),e[1].toString());QCOMPARE(finding.value("value").toString(),e[2].toString());QCOMPARE(d.value("title").toString(),e[3].toString());
   for(const auto &pair:QList<QPair<QString,int>>{{"visitTimeRaw",4},{"visitCount",5},{"typedCount",6},{"lastVisitTimeRaw",7},{"transitionRaw",8},{"fromVisitRaw",9}})QCOMPARE(d.value(pair.first).toString(),e[pair.second].toString());
   for(const auto &name:QStringList{"visitRecord","urlRecord"}){
    const auto ref=d.value(name).toMap();const auto raw=ref.value("rawExtents").toList(),image=ref.value("imageExtents").toList();QCOMPARE(raw.size(),image.size());
    for(int j=0;j<raw.size();++j){const auto o=raw[j].toMap(),v=image[j].toMap();const bool w=o.value("role")=="wal";walUsed|=w;const auto source=w?wal:r.verifiedBytes;QCOMPARE(source.mid(o.value("byteOffset").toLongLong(),o.value("lengthBytes").toLongLong()),snapshot.committedPages.mid(v.value("imageByteOffset").toLongLong(),v.value("lengthBytes").toLongLong()));}
   }
  }
  QVERIFY(walUsed);QCOMPARE(rows[0].toMap().value("details").toMap().value("timestampUtc").toString(),QString("2023-11-14T22:13:20.123Z"));
  QMap<QString,QVariantMap> inventory{{r.evidenceId,{{"id",r.evidenceId},{"sha256",r.sourceSha256},{"size",r.verifiedBytes.size()}}},{r.sqliteFamilyMembers["wal"].evidenceId,{{"id",r.sqliteFamilyMembers["wal"].evidenceId},{"sha256",sha(wal)},{"size",wal.size()}}}};
  const auto retained=mr::forensic::validateRetainedChromiumHistory(parsed.report,inventory);QVERIFY2(retained.ok(),qPrintable(retained.errorCode));
  for(int mode=0;mode<5;++mode){
   auto bad=parsed.report;auto changedRows=rows;auto v=changedRows[0].toMap(),d=v.value("details").toMap();
   if(mode==0)d.insert("timestampUtc","2000-01-01T00:00:00.000Z");
   if(mode==1)d.insert("urlId","99");
   if(mode==2){auto record=d.value("visitRecord").toMap();auto extents=record.value("rawExtents").toList();auto extent=extents[0].toMap();extent.insert("byteOffset",999999);extents[0]=extent;record.insert("rawExtents",extents);d.insert("visitRecord",record);}
   if(mode==3)d.insert("visitTimeRaw","001");
   if(mode==4)bad.insert("visitRows",99);
   v.insert("details",d);changedRows[0]=v;bad.insert("findings",changedRows);QVERIFY(!mr::forensic::validateRetainedChromiumHistory(bad,inventory).ok());
  }
  auto wrong=inventory;wrong.remove(r.sqliteFamilyMembers["wal"].evidenceId);QVERIFY(!mr::forensic::validateRetainedChromiumHistory(parsed.report,wrong).ok());
  const auto image=QByteArray::fromBase64(fixture.value("snapshotBase64").toByteArray());
  for(const auto &change:QList<QPair<QByteArray,QByteArray>>{{"PRIMARY KEY","PRIMARY xyz"},{"LONGVARCHAR","FLOAT      "}}){
   auto b=image;const auto offset=b.indexOf(change.first);QVERIFY(offset>=0);b.replace(offset,change.first.size(),change.second);auto invalid=request(b);invalid.adapter="chromium-history";QVERIFY(!mr::forensic::analyzeReadOnlyEvidence(invalid).ok());
  }
  std::mt19937 mutations(0x48495354);
  for(int i=0;i<128;++i){auto b=image;b[int(mutations()%quint32(b.size()))]^=char(1u<<(mutations()%8));auto mutated=request(b);mutated.adapter="chromium-history";const auto result=mr::forensic::analyzeReadOnlyEvidence(mutated);if(!result.ok())QVERIFY(result.report.isEmpty());else QVERIFY(QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact).size()<=65536);}
  r.cancelled->store(true);QVERIFY(!mr::forensic::analyzeReadOnlyEvidence(r).ok());r.cancelled->store(false);r.deadline=QDeadlineTimer(0);QVERIFY(!mr::forensic::analyzeReadOnlyEvidence(r).ok());
 }
 void historyUnsupportedAndLimits(){
  for(int mode=0;mode<8;++mode){
   QTemporaryDir temp;QVERIFY(temp.isValid());const auto path=temp.filePath("History");
   {
    auto db=QSqlDatabase::addDatabase("QSQLITE","history-negative");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
    QVERIFY(q.exec(mode==0?"CREATE TABLE urls(id INTEGER PRIMARY KEY,url TEXT)":"CREATE TABLE urls(id INTEGER PRIMARY KEY,url TEXT,title TEXT,visit_count INTEGER,typed_count INTEGER,last_visit_time INTEGER,hidden INTEGER)"));
    if(mode!=1)QVERIFY(q.exec("CREATE TABLE visits(id INTEGER PRIMARY KEY,url INTEGER,visit_time INTEGER,from_visit INTEGER,transition INTEGER)"));
    if(mode!=0){QVERIFY(q.exec("INSERT INTO urls VALUES(7,'https://example.invalid','title',1,0,13344473600123456,0)"));}
    if(mode!=1){
     const auto url=mode==2?"999":mode==3?"'7'":"7";
     for(int i=0;i<(mode==4?129:mode==7?100:1);++i)QVERIFY(q.exec(QString("INSERT INTO visits VALUES(%1,%2,13344473600123456,0,0)").arg(i+1).arg(url)));
     if(mode==3)QVERIFY(q.exec("UPDATE visits SET visit_time='not-an-integer'"));
    }
    if(mode==5)QVERIFY(q.exec("UPDATE urls SET title=zeroblob(16)"));
    if(mode==6)QVERIFY(q.exec("UPDATE urls SET url='https://' || replace(hex(zeroblob(1100)), '00', 'aa')"));
    db.close();
   }
   QSqlDatabase::removeDatabase("history-negative");QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));auto r=request(f.readAll());r.adapter="chromium-history";const auto result=mr::forensic::analyzeReadOnlyEvidence(r);QVERIFY2(!result.ok(),qPrintable(QString::number(mode)));QVERIFY(result.report.isEmpty());
  }
  QCOMPARE(mr::forensic::chromiumHistoryTime("0"),QString{});QCOMPARE(mr::forensic::chromiumHistoryTime("9223372036854775807"),QString{});
 }
 void committedWalOracle(){
  QFile file(MR_SQLITE_FAMILY_REFERENCE_PATH);QVERIFY(file.open(QIODevice::ReadOnly));const auto fixture=QJsonDocument::fromJson(file.readAll()).toVariant().toMap();
  auto r=request(QByteArray::fromBase64(fixture.value("mainBase64").toByteArray()));
  const auto wal=QByteArray::fromBase64(fixture.value("walBase64").toByteArray());r.sqliteFamilyMembers.insert("wal",{"00000000-0000-4000-8000-000000000003",sha(wal),wal});
  const auto before=r.verifiedBytes;
  for(const auto root:{2u,3u}){
   const auto result=mr::forensic::readProtectedSqliteTable(r,root);QVERIFY2(result.ok(),qPrintable(result.errorCode));
   const auto expected=fixture.value(root==2?"expectedUrls":"expectedVisits").toList();QCOMPARE(result.rows.size(),expected.size());
   for(int i=0;i<expected.size();++i){
    auto fields=result.rows[i].fields;QVERIFY(!fields.isEmpty());QVERIFY(fields[0].isNull());fields[0]=result.rows[i].rowid;
    const auto row=expected[i].toList();QCOMPARE(fields.size(),row.size());
    for(int j=0;j<row.size();++j)QCOMPARE(fields[j].toString(),row[j].toString());
    QVERIFY(!result.rows[i].imageExtents.isEmpty());
   }
  }
  QCOMPARE(before,r.verifiedBytes);
  r.sqliteFamilyMembers["wal"].bytes[40]^=1;QVERIFY(!mr::forensic::readProtectedSqliteTable(r,2).ok());
 }
 void runtimeOracle_data(){QTest::addColumn<int>("pageSize");for(int size:{512,1024,4096,65536})QTest::newRow(qPrintable(QString::number(size)))<<size;}
 void runtimeOracle(){
  QFETCH(int,pageSize);QTemporaryDir temp;QVERIFY(temp.isValid());const auto path=temp.filePath("fixture.sqlite");
  QList<QVariantList> expected;
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","table-oracle");db.setDatabaseName(path);QVERIFY(db.open());
   QSqlQuery query(db);QVERIFY(query.exec("PRAGMA page_size="+QString::number(pageSize)));
   QVERIFY(query.exec("CREATE TABLE records(id INTEGER PRIMARY KEY, value TEXT, n INTEGER, blob BLOB)"));
   QVERIFY(db.transaction());QVERIFY(query.prepare("INSERT INTO records VALUES(?,?,?,?)"));
   for(int i=0;i<300;++i){
    const qint64 id=i==0?std::numeric_limits<qint64>::min():i==299?std::numeric_limits<qint64>::max():i-150;
    const QString text=i==10?QString(12000,QChar(0x0639)):QString::fromUtf8("نص-")+QString::number(i);
    const qint64 n=i==11?std::numeric_limits<qint64>::min():i==12?std::numeric_limits<qint64>::max():qint64(i)*-123456789;
    query.bindValue(0,id);query.bindValue(1,text);query.bindValue(2,n);query.bindValue(3,QByteArray::fromHex("00ff10"));QVERIFY(query.exec());
   }
   QVERIFY(db.commit());QVERIFY(query.exec("SELECT id,value,n,blob FROM records ORDER BY id"));
   while(query.next())expected.append({query.value(0).toString(),query.value(1),query.value(2).toString(),query.value(3)});
   query.finish();db.close();
  }
  QSqlDatabase::removeDatabase("table-oracle");QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));const auto bytes=file.readAll();auto r=request(bytes);
  const auto result=mr::forensic::readProtectedSqliteTable(r,2);QVERIFY2(result.ok(),qPrintable(result.errorCode));QCOMPARE(result.rows.size(),expected.size());
  for(int i=0;i<expected.size();++i){auto fields=result.rows[i].fields;fields[0]=result.rows[i].rowid;QCOMPARE(fields,expected[i]);}
  if(pageSize<65536){
   bool tested=false;
   for(const auto &row:result.rows)if(row.imageExtents.size()>2){
    const auto overflowOffset=row.imageExtents[1].toMap().value("imageByteOffset").toInt()-4;
    auto bad=bytes;be32(bad,overflowOffset,quint32(overflowOffset/pageSize+1));
    const auto rejected=mr::forensic::readProtectedSqliteTable(request(bad),2);
    QCOMPARE(rejected.errorCode,QString("sqlite-table-page-reference"));QVERIFY(rejected.rows.isEmpty());tested=true;break;
   }
   QVERIFY(tested);
  }
  QCOMPARE(r.verifiedBytes,bytes);
  for(quint32 root:{0u,quint32(bytes.size()/pageSize+1)})QVERIFY(!mr::forensic::readProtectedSqliteTable(r,root).ok());
  if(uchar(bytes[pageSize])==5){auto bad=bytes;be32(bad,pageSize+8,2);QVERIFY(!mr::forensic::readProtectedSqliteTable(request(bad),2).ok());}
  auto encoding=bytes;be32(encoding,56,2);QCOMPARE(mr::forensic::readProtectedSqliteTable(request(encoding),2).errorCode,QString("sqlite-table-encoding-unsupported"));
  r.cancelled->store(true);QCOMPARE(mr::forensic::readProtectedSqliteTable(r,2).errorCode,QString("cancelled"));r.cancelled->store(false);r.deadline=QDeadlineTimer(0);QVERIFY(!mr::forensic::readProtectedSqliteTable(r,2).ok());
 }
 void malformedCellsAndRecords(){
  QFile file(MR_SQLITE_FAMILY_REFERENCE_PATH);QVERIFY(file.open(QIODevice::ReadOnly));const auto fixture=QJsonDocument::fromJson(file.readAll()).toVariant().toMap();
  const auto image=QByteArray::fromBase64(fixture.value("snapshotBase64").toByteArray());
  const int pageSize=1024,pointer=qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(image.constData()+pageSize+8));
  for(int mode=0;mode<6;++mode){
   auto bad=image;
   if(mode==0)bad[pageSize]=2; // Index b-tree, not a rowid table.
   if(mode==1){bad[pageSize+8]=0;bad[pageSize+9]=1;}
   if(mode==2){for(int i=0;i<9;++i)bad[pageSize+pointer+i]=std::bit_cast<char>(quint8(255));}
   // Fixture cell: payload-size varint, rowid varint, record header-size, serials.
   if(mode==3)bad[pageSize+pointer+2]=char(127);
   if(mode==4)bad[pageSize+pointer+3]=10;
   if(mode==5){const auto url=bad.indexOf("https://",pageSize);QVERIFY(url>=0);bad[url]=std::bit_cast<char>(quint8(255));}
   QVERIFY2(!mr::forensic::readProtectedSqliteTable(request(bad),2).ok(),qPrintable(QString::number(mode)));
  }
  std::mt19937 generator(0x51b7);
  for(int i=0;i<512;++i){
   auto mutated=image;const auto offset=generator()%quint32(mutated.size());mutated[int(offset)]^=char(1u<<(generator()%8));
   const auto result=mr::forensic::readProtectedSqliteTable(request(mutated),2);
   if(!result.ok())QVERIFY(result.rows.isEmpty());else QVERIFY(result.rows.size()<=4096);
  }
 }

 void historySchemaJoinOracle_data(){
  QTest::addColumn<int>("variant");
  QTest::newRow("committed-wal")<<0;
  QTest::newRow("urls-only")<<1;
  QTest::newRow("reordered-columns")<<2;
  QTest::newRow("unknown-column")<<3;
  QTest::newRow("integer-as-text")<<4;
  QTest::newRow("result-bound")<<5;
  QTest::newRow("output-bound")<<6;
  QTest::newRow("missing-urls")<<7;
  QTest::newRow("empty-visits-no-fallback")<<8;
  QTest::newRow("without-rowid")<<9;
  QTest::newRow("blob-url")<<10;
  QTest::newRow("null-and-zero-time")<<11;
  QTest::newRow("quoted-identifier-rejected")<<12;
  QTest::newRow("unsupported-visits-no-fallback")<<13;
  QTest::newRow("nullable-url-and-title")<<14;
  QTest::newRow("negative-time-preserved")<<15;
  QTest::newRow("current-layout-with-sync-columns")<<16;
  QTest::newRow("overflow-source-references")<<17;
 }
 void historySchemaJoinOracle(){
  QFETCH(int,variant);QTemporaryDir temp;QVERIFY(temp.isValid());const auto path=temp.filePath("History");
  mr::forensic::ReadOnlyEvidenceRequest r;QList<QVariantList> expected;
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","history-oracle");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
   QVERIFY(q.exec("PRAGMA page_size=1024"));QVERIFY(q.exec("PRAGMA journal_mode=WAL"));QVERIFY(q.exec("PRAGMA wal_autocheckpoint=0"));
   const QString urlSchema=variant==2?"CREATE TABLE urls(title TEXT,hidden INTEGER,id INTEGER PRIMARY KEY,url TEXT,typed_count INTEGER,last_visit_time INTEGER,visit_count INTEGER)":
    variant==9?"CREATE TABLE urls(id INTEGER PRIMARY KEY,url TEXT,title TEXT,visit_count INTEGER,typed_count INTEGER,last_visit_time INTEGER,hidden INTEGER) WITHOUT ROWID":
    variant==12?"CREATE TABLE urls(\"id\" INTEGER PRIMARY KEY,url TEXT,title TEXT,visit_count INTEGER,typed_count INTEGER,last_visit_time INTEGER,hidden INTEGER)":
    "CREATE TABLE urls(id INTEGER PRIMARY KEY AUTOINCREMENT,url LONGVARCHAR,title LONGVARCHAR,visit_count INTEGER DEFAULT 0 NOT NULL,typed_count INTEGER DEFAULT 0 NOT NULL,last_visit_time INTEGER NOT NULL,hidden INTEGER DEFAULT 0 NOT NULL)";
   QVERIFY(q.exec(urlSchema));
   if(variant==3)QVERIFY(q.exec("ALTER TABLE urls ADD COLUMN unknown INTEGER"));
   if(variant!=1)QVERIFY(q.exec("CREATE TABLE visits(id INTEGER PRIMARY KEY AUTOINCREMENT,url INTEGER NOT NULL,visit_time INTEGER NOT NULL,from_visit INTEGER,transition INTEGER DEFAULT 0 NOT NULL,segment_id INTEGER,visit_duration INTEGER DEFAULT 0 NOT NULL,incremented_omnibox_typed_score BOOLEAN DEFAULT FALSE NOT NULL)"));
   if(variant==16){
    QVERIFY(q.exec("DROP TABLE visits"));
    QVERIFY(q.exec("CREATE TABLE visits(id INTEGER PRIMARY KEY AUTOINCREMENT,url INTEGER NOT NULL,visit_time INTEGER NOT NULL,from_visit INTEGER,external_referrer_url TEXT,transition INTEGER DEFAULT 0 NOT NULL,segment_id INTEGER,visit_duration INTEGER DEFAULT 0 NOT NULL,incremented_omnibox_typed_score BOOLEAN DEFAULT FALSE NOT NULL,opener_visit INTEGER,originator_cache_guid TEXT,originator_visit_id INTEGER,originator_from_visit INTEGER,originator_opener_visit INTEGER,is_known_to_sync BOOLEAN DEFAULT FALSE NOT NULL,consider_for_ntp_most_visited BOOLEAN DEFAULT FALSE NOT NULL,visited_link_id INTEGER DEFAULT 0 NOT NULL,app_id TEXT)"));
   }
   if(variant==13)QVERIFY(q.exec("ALTER TABLE visits ADD COLUMN unknown INTEGER"));
   QVERIFY(q.exec("PRAGMA wal_checkpoint(TRUNCATE)"));
   QVERIFY(q.prepare("INSERT INTO urls(id,url,title,visit_count,typed_count,last_visit_time,hidden) VALUES(?,?,?,?,?,?,?)"));
   q.addBindValue(Q_INT64_C(9007199254740993));q.addBindValue(variant==10?QVariant(QByteArray("https://blob.invalid")):QVariant(QString("https://example.invalid/")));
   q.addBindValue(variant==6?QString(64000,'x'):variant==17?QString(4000,QChar(0x0639)):QString::fromUtf8("عنوان تجريبي"));q.addBindValue(2);q.addBindValue(1);q.addBindValue(Q_INT64_C(11644473600000001));q.addBindValue(0);QVERIFY(q.exec());
   if(variant!=1&&variant!=8){
    QVERIFY(q.prepare("INSERT INTO visits(id,url,visit_time,from_visit,transition) VALUES(?,?,?,?,?)"));
    const int count=variant==5?129:variant==11?2:1;
    for(int i=0;i<count;++i){
     q.bindValue(0,Q_INT64_C(9007199254741000)+i);q.bindValue(1,Q_INT64_C(9007199254740993));
     q.bindValue(2,variant==4?QVariant(QString("not-an-integer")):variant==11?QVariant(i==0?Q_INT64_C(0):std::numeric_limits<qint64>::max()):QVariant(Q_INT64_C(13300000000000001)));
     q.bindValue(3,QVariant{});q.bindValue(4,std::numeric_limits<int>::min());QVERIFY(q.exec());
    }
    // Keep unmatched visits explicit rather than fabricating a URL or a visit.
    QVERIFY(q.exec("INSERT INTO visits(url,visit_time,transition) VALUES(999,123,0)"));
   }
   if(variant==7)QVERIFY(q.exec("DROP TABLE urls"));
   if(variant==14)QVERIFY(q.exec("UPDATE urls SET url=NULL,title=NULL"));
   if(variant==15)QVERIFY(q.exec("UPDATE visits SET visit_time=-1"));
   if(variant!=1&&variant!=7){
    QVERIFY(q.exec("SELECT v.id,u.id,u.url,u.title,v.visit_time,v.from_visit,v.transition FROM visits v JOIN urls u ON u.id=v.url ORDER BY v.id"));
    while(q.next()){QVariantList row;for(int i=0;i<7;++i)row.append(q.value(i));expected.append(row);}q.finish();
   }
   const auto read=[](const QString &p){QFile f(p);if(!f.open(QIODevice::ReadOnly))return QByteArray{};return f.readAll();};
   r=request(read(path));const auto wal=read(path+"-wal"),shm=read(path+"-shm");QVERIFY(!wal.isEmpty());QVERIFY(!shm.isEmpty());
   r.sqliteFamilyMembers.insert("wal",{"00000000-0000-4000-8000-000000000003",sha(wal),wal});
   r.sqliteFamilyMembers.insert("shm",{"00000000-0000-4000-8000-000000000004",sha(shm),shm});
   q.finish();db.close();
  }
  QSqlDatabase::removeDatabase("history-oracle");const auto before=r.verifiedBytes,walBefore=r.sqliteFamilyMembers["wal"].bytes;
  const auto out=mr::forensic::readProtectedChromiumHistory(r);
  const QMap<int,QString> errors{{3,"history-schema-unsupported"},{4,"history-visit-record"},{5,"history-result-limit"},{6,"history-output-limit"},{7,"history-urls-missing"},{9,"history-schema-unsupported"},{10,"history-url-record"},{12,"history-schema-unsupported"},{13,"history-schema-unsupported"}};
  if(errors.contains(variant)){QCOMPARE(out.errorCode,errors.value(variant));QVERIFY(out.report.isEmpty());return;}
  QVERIFY2(out.ok(),qPrintable(out.errorCode));
  QCOMPARE(QJsonDocument::fromVariant(out.report).toJson(QJsonDocument::Compact),QJsonDocument::fromVariant(mr::forensic::readProtectedChromiumHistory(r).report).toJson(QJsonDocument::Compact));
  const auto observations=out.report.value("observations").toList();
  QCOMPARE(observations.size(),variant==1?1:expected.size());
  const auto family=mr::forensic::inspectSqliteEvidenceFamily(r,{});QVERIFY(family.analysis.ok());
  bool walReference=false;
  for(int i=0;i<observations.size();++i){
   const auto item=observations[i].toMap();
   QCOMPARE(item.value("urlId").toString(),QString("9007199254740993"));
   QCOMPARE(item.value("lastVisit").toMap().value("unixMicroseconds").toString(),QString("1"));
   if(variant==1){QCOMPARE(item.value("kind").toString(),QString("chromium-url-metadata"));QVERIFY(!item.contains("visitTime"));}
   else{
    const auto e=expected[i];QCOMPARE(item.value("visitId").toString(),e[0].toString());if(e[2].isNull())QVERIFY(item.value("url").isNull());else QCOMPARE(item.value("url"),e[2]);
    if(e[3].isNull())QVERIFY(item.value("title").isNull());else QCOMPARE(item.value("title"),e[3]);
    QCOMPARE(item.value("visitTime").toMap().value("rawMicroseconds1601").toString(),e[4].toString());
    QVERIFY(item.value("fromVisit").isNull());QCOMPARE(item.value("transition").toString(),e[6].toString());
    if(variant==15){const auto t=item.value("visitTime").toMap();QCOMPARE(t.value("status").toString(),QString("unsupported-negative"));QCOMPARE(t.value("rawMicroseconds1601").toString(),QString("-1"));QVERIFY(!t.contains("unixMicroseconds"));}
    if(variant==11){const auto t=item.value("visitTime").toMap();if(i==0){QCOMPARE(t.value("status").toString(),QString("unset-zero"));QVERIFY(!t.contains("unixMicroseconds"));}else QCOMPARE(t.value("unixMicroseconds").toString(),QString::number(std::numeric_limits<qint64>::max()-Q_INT64_C(11644473600000000)));}
   }
   for(const auto &key:{"urlSourceExtents","urlSchemaExtents","visitSourceExtents","visitSchemaExtents"})for(const auto &value:item.value(key).toList()){
    const auto x=value.toMap();const bool wal=x.value("role")=="wal";const auto &source=wal?r.sqliteFamilyMembers["wal"].bytes:r.verifiedBytes;
    const auto offset=x.value("byteOffset").toLongLong(),length=x.value("lengthBytes").toLongLong(),imageOffset=x.value("imageByteOffset").toLongLong();
    QVERIFY(offset>=0&&length>0&&offset+length<=source.size());QCOMPARE(source.mid(offset,length),family.committedPages.mid(imageOffset,length));
    QCOMPARE(x.value("sourceSha256").toString(),sha(source));walReference|=wal;
   }
  }
  if(!observations.isEmpty())QVERIFY(walReference);
  QCOMPARE(out.report.value("unmatchedVisitCount").toInt(),variant==1||variant==8?0:1);
  QCOMPARE(r.verifiedBytes,before);QCOMPARE(r.sqliteFamilyMembers["wal"].bytes,walBefore);
  if(variant==0){
   const auto image=family.committedPages;std::mt19937 random(0x48495354);
   for(int j=0;j<256;++j){
    auto mutated=image;mutated[int(random()%quint32(mutated.size()))]^=char(1u<<(random()%8));
    const auto observed=mr::forensic::readProtectedChromiumHistory(request(mutated));
    if(!observed.ok())QVERIFY(observed.report.isEmpty());
    else QVERIFY(QJsonDocument::fromVariant(observed.report).toJson(QJsonDocument::Compact).size()<=64*1024);
   }
  }
  r.cancelled->store(true);QCOMPARE(mr::forensic::readProtectedChromiumHistory(r).errorCode,QString("cancelled"));r.cancelled->store(false);
  r.deadline=QDeadlineTimer(0);QVERIFY(!mr::forensic::readProtectedChromiumHistory(r).ok());r.deadline=QDeadlineTimer(5000);
  r.sqliteFamilyMembers["wal"].bytes[40]^=1;QVERIFY(!mr::forensic::readProtectedChromiumHistory(r).ok());
 }

 void resourceLimits(){
  QTemporaryDir temp;QVERIFY(temp.isValid());const auto path=temp.filePath("limits.sqlite");
  {
   auto db=QSqlDatabase::addDatabase("QSQLITE","limits");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
   QVERIFY(q.exec("CREATE TABLE many(id INTEGER PRIMARY KEY)"));
   QVERIFY(q.exec("WITH RECURSIVE x(n) AS (VALUES(1) UNION ALL SELECT n+1 FROM x WHERE n<4097) INSERT INTO many SELECT n FROM x"));
   QVERIFY(q.exec("CREATE TABLE big(value TEXT)"));QVERIFY(q.exec("INSERT INTO big VALUES(hex(zeroblob(40000)))"));
   q.finish();db.close();
  }
  QSqlDatabase::removeDatabase("limits");QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));const auto r=request(file.readAll());
  const auto schema=mr::forensic::readProtectedSqliteTable(r,1);QVERIFY(schema.ok());
  for(const auto &row:schema.rows){
   const auto root=row.fields[3].toUInt();const auto result=mr::forensic::readProtectedSqliteTable(r,root);
   QCOMPARE(result.errorCode,QString("sqlite-table-resource-limit"));QVERIFY(result.rows.isEmpty());
  }
 }
};
QTEST_GUILESS_MAIN(SqliteTableTests)
#include "sqlite_table_tests.moc"
