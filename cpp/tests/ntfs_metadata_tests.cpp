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
 void mappedOriginalBytesAndUsaReconstruction(){
  const auto b=image();auto r=request(b);const auto result=mr::forensic::inspectNtfsMetadata(r);QVERIFY2(result.ok(),qPrintable(result.errorCode));
  QCOMPARE(result.report.value("schema").toString(),QString("nexvary-mr-ntfs-metadata/2"));QCOMPARE(result.report.value("caseId").toString(),r.caseId);QCOMPARE(result.report.value("evidenceId").toString(),r.evidenceId);QCOMPARE(result.report.value("workingCopySha256").toString(),sha(b));
  const auto mapping=result.report.value("mftMapping").toList();QCOMPARE(mapping.size(),2);QCOMPARE(mapping[0].toMap().value("byteOffset").toLongLong(),16384);QCOMPARE(mapping[0].toMap().value("lengthBytes").toLongLong(),12288);QCOMPARE(mapping[1].toMap().value("byteOffset").toLongLong(),81920);qint64 streamOffset=0;
  for(const auto &v:mapping){const auto m=v.toMap();QCOMPARE(m.value("logicalByteOffset").toLongLong(),streamOffset);QVERIFY(m.value("byteOffset").toLongLong()>=0);streamOffset+=m.value("lengthBytes").toLongLong();}
  auto records=QVariantList{};const auto sources=result.report.value("metadataSources").toMap();for(const auto &key:{"mftBootstrap","volumeRecord","bitmapRecord"})records.append(sources.value(key));
  for(const auto &v:result.report.value("findings").toList())records.append(v.toMap().value("details"));
  for(const auto &v:records){
   const auto d=v.toMap();const auto index=d.value("recordIndex").toLongLong();const auto refs=d.value("rawRecordExtents").toList();QVERIFY(!refs.isEmpty());QCOMPARE(refs[0].toMap().value("byteOffset").toLongLong(),recordOffset(int(index)));QByteArray raw;qint64 copied=0;
   for(const auto &ref:refs){const auto extent=ref.toMap();const auto recordOffset=extent.value("recordByteOffset").toLongLong(),offset=extent.value("byteOffset").toLongLong(),length=extent.value("lengthBytes").toLongLong();QCOMPARE(recordOffset,copied);const auto logical=index*1024+recordOffset;
    bool found=false;for(const auto &entry:mapping){const auto map=entry.toMap();const auto start=map.value("logicalByteOffset").toLongLong(),n=map.value("lengthBytes").toLongLong();if(logical>=start&&logical<start+n){QCOMPARE(offset,map.value("byteOffset").toLongLong()+logical-start);QVERIFY(length<=n-(logical-start));found=true;}}
    QVERIFY(found);raw+=b.mid(offset,length);copied+=length;
   }
   QCOMPARE(raw.size(),1024);QCOMPARE(sha(raw),d.value("rawRecordSha256").toString());
   const auto usa=d.value("usa").toMap();const auto array=QByteArray::fromHex(usa.value("arrayHex").toByteArray());const auto p=usa.value("recordByteOffset").toLongLong();QCOMPARE(array,raw.mid(p,6));const auto replacements=usa.value("replacements").toList();QCOMPARE(replacements.size(),2);auto fixed=raw;
   for(int i=0;i<replacements.size();++i){const auto item=replacements[i].toMap();const auto off=item.value("recordByteOffset").toLongLong();QCOMPARE(off,(i+1)*512-2);QCOMPARE(item.value("originalHex").toByteArray(),raw.mid(off,2).toHex());QCOMPARE(raw.mid(off,2),array.left(2));const auto restored=QByteArray::fromHex(item.value("replacementHex").toByteArray());QCOMPARE(restored,array.mid(2*(i+1),2));fixed.replace(off,2,restored);}
   QCOMPARE(sha(fixed),d.value("reconstructedRecordSha256").toString());
  }
  QCOMPARE(r.verifiedBytes,b);
 }
 void replayAcceptsOnlyExactBoundReport(){
  const auto b=image();const auto r=request(b);const auto parsed=mr::forensic::inspectNtfsMetadata(r);QVERIFY(parsed.ok());const auto json=QJsonDocument::fromVariant(parsed.report).toJson(QJsonDocument::Compact);
  const auto accepted=mr::forensic::verifyNtfsMetadataReport(r,json,sha(json));QVERIFY2(accepted.ok(),qPrintable(accepted.errorCode));QCOMPARE(accepted.report,parsed.report);
  auto other=r;other.caseId="00000000-0000-4000-8000-000000000099";QCOMPARE(mr::forensic::verifyNtfsMetadataReport(other,json,sha(json)).errorCode,QString("ntfs-report-source-mismatch"));
  other=r;other.evidenceId=other.caseId;QCOMPARE(mr::forensic::verifyNtfsMetadataReport(other,json,sha(json)).errorCode,QString("ntfs-report-source-mismatch"));
  other=r;other.verifiedBytes[recordOffset(16)+510]^=1;other.sourceSha256=sha(other.verifiedBytes);const auto torn=mr::forensic::verifyNtfsMetadataReport(other,json,sha(json));QVERIFY(!torn.ok());QVERIFY(torn.report.isEmpty());
  other=r;other.verifiedBytes[1000]^=1;QCOMPARE(mr::forensic::verifyNtfsMetadataReport(other,json,sha(json)).errorCode,QString("ntfs-input-or-hash"));
  other=r;other.verifiedBytes[1000]^=1;other.sourceSha256=sha(other.verifiedBytes);QCOMPARE(mr::forensic::verifyNtfsMetadataReport(other,json,sha(json)).errorCode,QString("ntfs-report-source-mismatch"));
 }
 void rehashedReportForgery_data(){QTest::addColumn<int>("mode");for(int i=0;i<26;++i)QTest::newRow(qPrintable(QString::number(i)))<<i;}
 void rehashedReportForgery(){
  QFETCH(int,mode);auto r=request(image());auto parsed=mr::forensic::inspectNtfsMetadata(r);QVERIFY(parsed.ok());auto report=parsed.report;auto findings=report.value("findings").toList();auto finding=findings[0].toMap(),details=finding.value("details").toMap();
  if(mode==0)report.insert("caseId","00000000-0000-4000-8000-000000000099");
  if(mode==1)report.insert("evidenceId",r.caseId);
  if(mode==2)report.insert("sourceCommit",QString(40,'0'));
  if(mode==3)report.insert("parserVersion","nexvary-mr-ntfs-metadata/1");
  if(mode==4)report.insert("workingCopySha256",QString(64,'0'));
  if(mode==5)report.insert("bitmapAvailable",false);
  if(mode==6)report.insert("emptySlots",0);
  if(mode==7)report.insert("limitations",QStringList{});
  if(mode==8){auto mapping=report.value("mftMapping").toList();auto map=mapping[0].toMap();map.insert("byteOffset",map.value("byteOffset").toLongLong()+512);mapping[0]=map;report.insert("mftMapping",mapping);}
  if(mode==9){auto refs=details.value("rawRecordExtents").toList();auto ref=refs[0].toMap();ref.insert("byteOffset",ref.value("byteOffset").toLongLong()+512);refs[0]=ref;details.insert("rawRecordExtents",refs);}
  if(mode==10)details.insert("rawRecordSha256",QString(64,'0'));
  if(mode==11)details.insert("reconstructedRecordSha256",details.value("rawRecordSha256"));
  if(mode==12){auto usa=details.value("usa").toMap();usa.insert("recordByteOffset",58);details.insert("usa",usa);}
  if(mode==13){auto usa=details.value("usa").toMap();usa.insert("arrayHex","010000000000");details.insert("usa",usa);}
  if(mode==14){auto usa=details.value("usa").toMap();auto replacements=usa.value("replacements").toList();replacements.removeLast();usa.insert("replacements",replacements);details.insert("usa",usa);}
  if(mode==15)details.insert("recordIndex",QString::number(details.value("recordIndex").toLongLong()));
  if(mode==16)details.insert("deleted",!details.value("deleted").toBool());
  if(mode==17){auto names=details.value("names").toList();auto name=names[0].toMap();name.insert("createdFiletimeRaw","133444736001234576");names[0]=name;details.insert("names",names);}
  if(mode==18){auto data=details.value("dataStream").toMap();data.insert("recovery","complete");details.insert("dataStream",data);}
  if(mode==19)finding.insert("confidence","verified-recovery");
  if(mode==20)report.insert("unexpectedAuthority","signed-case-acceptance");
  if(mode==21){auto sources=report.value("metadataSources").toMap();sources.insert("bitmapBytesSha256",QString(64,'0'));report.insert("metadataSources",sources);}
  if(mode==25){auto parameters=report.value("analysisParameters").toMap();parameters.insert("volumeImageOffset",512);report.insert("analysisParameters",parameters);}
  finding.insert("details",details);findings[0]=finding;report.insert("findings",findings);auto json=QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact);
  if(mode==22)json=" "+json;
  if(mode==23)json.insert(1,"\"caseId\":\"00000000-0000-4000-8000-000000000099\",");
  if(mode==24)json="["+json+"]";
  const auto forged=mr::forensic::verifyNtfsMetadataReport(r,json,sha(json));QCOMPARE(forged.errorCode,QString("ntfs-report-source-mismatch"));QVERIFY(forged.report.isEmpty());
 }
 void replayBoundsBindingsAndCancellation(){
  auto r=request(image());const auto parsed=mr::forensic::inspectNtfsMetadata(r);QVERIFY(parsed.ok());const auto json=QJsonDocument::fromVariant(parsed.report).toJson(QJsonDocument::Compact);
  QCOMPARE(mr::forensic::verifyNtfsMetadataReport(r,json,QString(64,'0')).errorCode,QString("ntfs-report-hash"));
  for(const auto &bytes:QList<QByteArray>{QByteArray{},QByteArray(65537,'[')}){const auto rejected=mr::forensic::verifyNtfsMetadataReport(r,bytes,sha(bytes));QCOMPARE(rejected.errorCode,QString("ntfs-report-limit"));QVERIFY(rejected.report.isEmpty());}
  r.caseId="arbitrary";QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("ntfs-provenance-binding"));
  r=request(image());r.evidenceId.clear();QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("ntfs-provenance-binding"));
  r=request(image());r.verifyManifestPayloads=true;QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("ntfs-auxiliary-mode-unsupported"));
  r=request(image());r.sqliteFamilyMembers.insert("wal",{r.evidenceId,r.sourceSha256,r.verifiedBytes});QCOMPARE(mr::forensic::inspectNtfsMetadata(r).errorCode,QString("ntfs-auxiliary-mode-unsupported"));
  r=request(image());r.cancelled->store(true);QCOMPARE(mr::forensic::verifyNtfsMetadataReport(r,json,sha(json)).errorCode,QString("cancelled"));r.cancelled->store(false);r.deadline=QDeadlineTimer(0);QCOMPARE(mr::forensic::verifyNtfsMetadataReport(r,json,sha(json)).errorCode,QString("worker-timeout"));
 }
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
  const auto json=QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact);const auto verified=mr::forensic::verifyNtfsMetadataReport(request(b),json,sha(json));QVERIFY2(verified.ok(),qPrintable(verified.errorCode));QCOMPARE(verified.report,result.report);
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
