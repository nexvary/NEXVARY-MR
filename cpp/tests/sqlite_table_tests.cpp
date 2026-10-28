#include "SqliteTableReader.hpp"
#include "ChromiumHistoryReader.hpp"
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
