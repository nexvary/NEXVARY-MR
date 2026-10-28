#include "SqliteTableReader.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>
#include <limits>
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
   if(mode==2){for(int i=0;i<9;++i)bad[pageSize+pointer+i]=char(255);}
   // Fixture cell: payload-size varint, rowid varint, record header-size, serials.
   if(mode==3)bad[pageSize+pointer+2]=char(127);
   if(mode==4)bad[pageSize+pointer+3]=10;
   if(mode==5){const auto url=bad.indexOf("https://",pageSize);QVERIFY(url>=0);bad[url]=char(255);}
   QVERIFY2(!mr::forensic::readProtectedSqliteTable(request(bad),2).ok(),qPrintable(QString::number(mode)));
  }
  std::mt19937 generator(0x51b7);
  for(int i=0;i<512;++i){
   auto mutated=image;const auto offset=generator()%quint32(mutated.size());mutated[int(offset)]^=char(1u<<(generator()%8));
   const auto result=mr::forensic::readProtectedSqliteTable(request(mutated),2);
   if(!result.ok())QVERIFY(result.rows.isEmpty());else QVERIFY(result.rows.size()<=4096);
  }
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
