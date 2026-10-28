#include "NtfsMetadata.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QtEndian>
#include <QtTest>
#include <random>
#include <bit>
#include <limits>
namespace {
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
QVariantMap oracle(){QFile f(MR_NTFS_REFERENCE_PATH);if(!f.open(QIODevice::ReadOnly))qFatal("Missing synthetic NTFS oracle");return QJsonDocument::fromJson(f.readAll()).toVariant().toMap();}
QByteArray image(){return qUncompress(QByteArray::fromBase64(oracle().value("qCompressBase64").toByteArray()));}
mr::forensic::ReadOnlyEvidenceRequest request(const QByteArray &b){return {"00000000-0000-4000-8000-000000000001","00000000-0000-4000-8000-000000000002",sha(b),"internal-ntfs-metadata",b,std::make_shared<std::atomic_bool>(false)};}
void put16(QByteArray &b,qsizetype p,quint16 n){qToLittleEndian(n,reinterpret_cast<uchar*>(b.data()+p));}
void put32(QByteArray &b,qsizetype p,quint32 n){qToLittleEndian(n,reinterpret_cast<uchar*>(b.data()+p));}
void put64(QByteArray &b,qsizetype p,quint64 n){qToLittleEndian(n,reinterpret_cast<uchar*>(b.data()+p));}
qint64 recordOffset(int i){return oracle().value("recordOffsets").toMap().value(QString::number(i)).toLongLong();}
qsizetype attributeOffset(const QByteArray &b,int record,quint32 kind){
 const auto base=recordOffset(record);qsizetype p=base+56;
 for(int i=0;i<64&&p+24<=base+1024;++i){const auto type=qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(b.constData()+p));if(type==kind)return p;if(type==0xffffffffu)break;const auto n=qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(b.constData()+p+4));if(n<24)break;p+=n;}
 return -1;
}
}
class NtfsMetadataTests:public QObject {
 Q_OBJECT
private slots:
 void knownAnswerAndOriginalReferences(){
  const auto b=image();QCOMPARE(b.size(),4*1024*1024);QCOMPARE(sha(b),oracle().value("imageSha256").toString());
  auto r=request(b);const auto result=mr::forensic::inspectNtfsMetadata(r);QVERIFY2(result.ok(),qPrintable(result.errorCode));QCOMPARE(result.report,mr::forensic::inspectNtfsMetadata(r).report);QCOMPARE(r.verifiedBytes,b);
  const auto rows=result.report.value("findings").toList(),names=oracle().value("expectedNames").toList();QCOMPARE(rows.size(),4);
  for(int i=0;i<rows.size();++i){const auto f=rows[i].toMap(),d=f.value("details").toMap();QCOMPARE(f.value("value").toString(),names[i].toString());QCOMPARE(d.value("recordIndex").toInt(),16+i);QVERIFY(d.value("usaVerified").toBool());QByteArray raw;qint64 logical=0;for(const auto &v:d.value("rawRecordExtents").toList()){const auto e=v.toMap();QCOMPARE(e.value("recordByteOffset").toLongLong(),logical);raw+=b.mid(e.value("byteOffset").toLongLong(),e.value("lengthBytes").toLongLong());logical+=e.value("lengthBytes").toLongLong();}QCOMPARE(raw.size(),1024);QCOMPARE(sha(raw),d.value("rawRecordSha256").toString());QCOMPARE(d.value("names").toList()[0].toMap().value("createdFiletimeRaw").toString(),QString("133444736001234575"));}
  const auto resident=rows[0].toMap().value("details").toMap().value("dataStream").toMap();QCOMPARE(resident.value("reconstructedResidentSha256").toString(),oracle().value("residentSha256").toString());QCOMPARE(resident.value("logicalBytes"),oracle().value("residentBytes"));
  const auto fragmented=rows[1].toMap().value("details").toMap().value("dataStream").toMap().value("extents").toList();QCOMPARE(fragmented.size(),2);QCOMPARE(fragmented[0].toMap().value("byteOffset").toLongLong(),200*4096);QCOMPARE(fragmented[1].toMap().value("byteOffset").toLongLong(),100*4096);
  const auto deleted=rows[2].toMap().value("details").toMap();QVERIFY(deleted.value("deleted").toBool());const auto stream=deleted.value("dataStream").toMap();QCOMPARE(stream.value("recordedUnallocatedBytes").toLongLong(),4096);QCOMPARE(stream.value("recordedAllocatedBytes").toLongLong(),0);QCOMPARE(stream.value("recordedSparseLogicalBytes").toLongLong(),1904);QCOMPARE(stream.value("recovery").toString(),QString("not-performed"));
 }
 void independentFormattedNtfsImage(){
  const auto path=QString(MR_NTFS_REFERENCE_PATH).replace("reference.json","ntfs3g-reference.json");QFile f(path);QVERIFY(f.open(QIODevice::ReadOnly));const auto expected=QJsonDocument::fromJson(f.readAll()).toVariant().toMap();const auto b=qUncompress(QByteArray::fromBase64(expected.value("qCompressBase64").toByteArray()));QCOMPARE(sha(b),expected.value("imageSha256").toString());
  const auto result=mr::forensic::inspectNtfsMetadata(request(b));QVERIFY2(result.ok(),qPrintable(result.errorCode));const auto rows=result.report.value("findings").toList();const auto inventory=expected.value("expectedNamedUserRecords").toList();QCOMPARE(rows.size(),inventory.size());QVariantMap record;
  for(int i=0;i<rows.size();++i){const auto actual=rows[i].toMap();QCOMPARE(actual.value("value").toString(),inventory[i].toMap().value("name").toString());QCOMPARE(actual.value("details").toMap().value("recordIndex").toInt(),inventory[i].toMap().value("record").toInt());if(actual.value("value")=="known.txt")record=actual;}
  QVERIFY(!record.isEmpty());QCOMPARE(record.value("value").toString(),QString("known.txt"));const auto data=record.value("details").toMap().value("dataStream").toMap();QCOMPARE(data.value("logicalBytes").toLongLong(),3);QCOMPARE(data.value("reconstructedResidentSha256").toString(),expected.value("knownContentSha256").toString());
 }
 void malformedRecordsAndRuns_data(){QTest::addColumn<int>("mode");for(int i=0;i<21;++i)QTest::newRow(qPrintable(QString::number(i)))<<i;}
 void malformedRecordsAndRuns(){
  QFETCH(int,mode);auto b=image();const auto user=recordOffset(16),mft=recordOffset(0),volume=recordOffset(3);const auto data=attributeOffset(b,17,0x80);QVERIFY(data>0);
  if(mode==0)put16(b,11,4096);
  if(mode==1)b[13]=3;
  if(mode==2)put64(b,40,std::numeric_limits<quint64>::max());
  if(mode==3)b[64]=std::bit_cast<char>(quint8(239));
  if(mode==4)b[user+510]^=1;
  if(mode==5)put16(b,user+4,510);
  if(mode==6)put16(b,user+6,2);
  if(mode==7)put32(b,user+24,1025);
  if(mode==8)put32(b,user+56+4,0xfffffff8u);
  if(mode==9)put32(b,user+56,0x20);
  if(mode==10)b[volume+56+24+8]=4;
  if(mode==11){put16(b,data+12,1);}
  if(mode==12){b[data+64]=0x29;}
  if(mode==13){b[data+66]=std::bit_cast<char>(quint8(255));b[data+67]=std::bit_cast<char>(quint8(127));}
  if(mode==14){put64(b,data+56,6001);}
  if(mode==15){put64(b,mft+56+32,0);}
  if(mode==16){put16(b,user+56+24+66,0xd800);}
  if(mode==17){put64(b,mft+32,1);}
  if(mode==18)put16(b,mft+22,0);
  if(mode==19)put64(b,volume+32,1);
  if(mode==20)put64(b,recordOffset(6)+32,1);
  const auto result=mr::forensic::inspectNtfsMetadata(request(b));QVERIFY2(!result.ok(),qPrintable(QString::number(mode)));QVERIFY(!result.errorCode.isEmpty());QVERIFY(result.report.isEmpty());
 }
 void bitmapObservationsAndMissingBitmap(){
  auto b=image();const auto p=attributeOffset(b,6,0x80)+24;b[p+300/8]|=char(1<<(300%8));
  auto result=mr::forensic::inspectNtfsMetadata(request(b));QVERIFY2(result.ok(),qPrintable(result.errorCode));auto d=result.report.value("findings").toList()[2].toMap().value("details").toMap().value("dataStream").toMap();QCOMPARE(d.value("recordedAllocatedBytes").toLongLong(),4096);QCOMPARE(d.value("recovery").toString(),QString("not-performed"));
  b=image();b.replace(recordOffset(6),1024,QByteArray(1024,0));result=mr::forensic::inspectNtfsMetadata(request(b));QVERIFY2(result.ok(),qPrintable(result.errorCode));QCOMPARE(result.report.value("bitmapAvailable").toBool(),false);d=result.report.value("findings").toList()[2].toMap().value("details").toMap().value("dataStream").toMap();QCOMPARE(d.value("allocationObservation").toString(),QString("bitmap-unavailable"));QVERIFY(!d.contains("recordedUnallocatedBytes"));
 }
 void recordAndOutputLimits(){
  const auto o=oracle();
  for(const auto &entry:QList<QPair<QString,QString>>{{"tooManyRecordsQCompressBase64","ntfs-mft-bound"},{"outputLimitQCompressBase64","ntfs-output-limit"}}){
   const auto b=qUncompress(QByteArray::fromBase64(o.value(entry.first).toByteArray()));QCOMPARE(b.size(),4*1024*1024);
   const auto result=mr::forensic::inspectNtfsMetadata(request(b));QCOMPARE(result.errorCode,entry.second);QVERIFY(result.report.isEmpty());
  }
 }
 void hashCancellationAndMutations(){
  auto b=image();auto r=request(b);r.sourceSha256=QString(64,'0');QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("ntfs-input-or-hash"));
  r=request(b);r.cancelled->store(true);QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("cancelled"));r.cancelled->store(false);r.deadline=QDeadlineTimer(0);QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("worker-timeout"));
  for(const auto &bad:QList<QByteArray>{QByteArray(511,0),QByteArray(8*1024*1024+1,0),b.left(4096)})QVERIFY(!mr::forensic::inspectNtfsMetadata(request(bad)).ok());
  std::mt19937 random(0x4d465431);for(int i=0;i<256;++i){auto mutated=b;const auto off=recordOffset(i%2?16:0)+random()%1024;mutated[off]^=char(1u<<(random()%8));const auto result=mr::forensic::inspectNtfsMetadata(request(mutated));if(!result.ok()){QVERIFY(result.report.isEmpty());QVERIFY(!result.errorCode.isEmpty());}else QVERIFY(QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact).size()<=65536);}
 }
};
QTEST_GUILESS_MAIN(NtfsMetadataTests)
#include "ntfs_metadata_tests.moc"
