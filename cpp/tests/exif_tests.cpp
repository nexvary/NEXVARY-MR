#include "ExifSummary.hpp"
#include "ExifFixture.hpp"
#include "RecoveryBridge.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
#include <limits>
namespace {
QByteArray read(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
bool write(const QString &path,const QByteArray &bytes){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();}
}
class ExifTests final:public QObject {
 Q_OBJECT
private slots:
 void byteOrder_data(){QTest::addColumn<bool>("little");QTest::newRow("little")<<true;QTest::newRow("big")<<false;}
 void byteOrder(){
  QFETCH(bool,little);const auto bytes=syntheticExifJpeg(little);const auto r=mr::forensic::summarizeExif(bytes);
  QVERIFY2(r.error.isEmpty(),qPrintable(r.error));const auto fields=r.report["fields"].toObject();
  QCOMPARE(fields["make"].toString(),QString("Synthetic"));QCOMPARE(fields["orientation"].toInt(),6);
  QCOMPARE(fields["dateTimeOriginalRaw"].toString(),QString("2026:10:23 12:34:56"));
  QVERIFY(!r.report["timestampsConvertedToUtc"].toBool(true));QVERIFY(!r.report["metadataAuthenticated"].toBool(true));
  QCOMPARE(r.report["sourceSha256"].toString(),QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()));
 }
 void independentPillowReference(){
  const auto base=QStringLiteral(MR_EXIF_SAMPLE_DIR);const auto bytes=read(base+"/reference.jpg");QVERIFY(!bytes.isEmpty());
  const auto expected=QJsonDocument::fromJson(read(base+"/reference.json")).object();
  const auto r=mr::forensic::summarizeExif(bytes);QVERIFY2(r.error.isEmpty(),qPrintable(r.error));
  QCOMPARE(r.report["fields"].toObject(),expected["fields"].toObject());QCOMPARE(r.report["sourceSha256"],expected["sha256"]);
 }
 void invalidOffsetsAndTypes_data(){
  QTest::addColumn<int>("at");QTest::addColumn<quint32>("value");QTest::addColumn<bool>("shortValue");
  QTest::newRow("ifd-overflow")<<4<<std::numeric_limits<quint32>::max()<<false;
  QTest::newRow("ifd-header")<<4<<quint32(2)<<false;
  QTest::newRow("entry-count")<<8<<quint32(257)<<true;
  QTest::newRow("string-offset")<<18<<std::numeric_limits<quint32>::max()<<false;
  QTest::newRow("string-count")<<14<<std::numeric_limits<quint32>::max()<<false;
  QTest::newRow("wrong-type")<<12<<quint32(7)<<true;
  QTest::newRow("invalid-orientation")<<30<<quint32(9)<<true;
  QTest::newRow("ifd-cycle")<<42<<quint32(8)<<false;
  QTest::newRow("invalid-pointer-type")<<36<<quint32(3)<<true;
  QTest::newRow("duplicate-tag")<<22<<quint32(0x010f)<<true;
 }
 void invalidOffsetsAndTypes(){
  QFETCH(int,at);QFETCH(quint32,value);QFETCH(bool,shortValue);auto b=syntheticExifJpeg();
  if(shortValue)qToLittleEndian<quint16>(static_cast<quint16>(value),b.data()+12+at);else qToLittleEndian(value,b.data()+12+at);
  const auto r=mr::forensic::summarizeExif(b);QVERIFY(!r.error.isEmpty());QVERIFY(r.report.isEmpty());
 }
 void stringsAndExcludedMetadata(){
  auto b=syntheticExifJpeg();b[12+59]='X';QCOMPARE(mr::forensic::summarizeExif(b).error,QString("unterminated-ascii-tag"));
  b=syntheticExifJpeg();b[12+52]='\x01';QCOMPARE(mr::forensic::summarizeExif(b).error,QString("unsupported-ascii-value"));
  // Unknown MakerNote data is never dereferenced, parsed or copied to JSON.
  b=syntheticExifJpeg();qToLittleEndian<quint16>(0x927c,b.data()+12+66);
  b.replace(12+90,20,QByteArray("PRIVATE-OWNER-GPS!!!\0",20));
  auto r=mr::forensic::summarizeExif(b);QVERIFY(r.error.isEmpty());QVERIFY(!QJsonDocument(r.report).toJson().contains("PRIVATE-OWNER"));
  b=syntheticExifJpeg();qToLittleEndian<quint16>(0x8825,b.data()+12+34);
  r=mr::forensic::summarizeExif(b);QVERIFY(r.error.isEmpty());QVERIFY(r.report["gpsIfdDeclared"].toBool());QVERIFY(!r.report["gpsValuesIncluded"].toBool(true));
 }
 void markerBoundsAndNoExif(){
  const auto b=syntheticExifJpeg();for(qsizetype i=0;i<b.size();++i){const auto r=mr::forensic::summarizeExif(b.left(i));QVERIFY2(!r.error.isEmpty(),qPrintable(QString::number(i)));QVERIFY(r.report.isEmpty());}
  auto r=mr::forensic::summarizeExif(QByteArray::fromHex("ffd8ffd9"));QVERIFY(r.error.isEmpty());QVERIFY(!r.report["exifPresent"].toBool(true));
  auto duplicate=b.left(b.size()-2)+b.mid(2);QCOMPARE(mr::forensic::summarizeExif(duplicate).error,QString("multiple-exif-segments"));
  auto bad=b;bad[4]='\x7f';QVERIFY(!mr::forensic::summarizeExif(bad).error.isEmpty());
  QCOMPARE(mr::forensic::summarizeExif(QByteArray("not a jpeg")).error,QString("jpeg-required"));
  QCOMPARE(mr::forensic::summarizeExif(QByteArray(mr::forensic::maximumExifSourceBytes+1,'x')).error,QString("jpeg-size-limit"));
  QByteArray many=QByteArray::fromHex("ffd8");for(int i=0;i<4097;++i)many+=QByteArray::fromHex("ff01");many+=QByteArray::fromHex("ffd9");
  QCOMPARE(mr::forensic::summarizeExif(many).error,QString("jpeg-segment-limit"));
  auto cancelled=std::make_shared<std::atomic_bool>(true);QCOMPARE(mr::forensic::summarizeExif(b,cancelled).error,QString("cancelled"));
 }
 void caseWorkflow(){
  QTemporaryDir dir;QVERIFY(dir.isValid());RecoveryBridge bridge(dir.filePath("cases"),true);
  const auto path=dir.filePath("private-name.jpg");const auto bytes=syntheticExifJpeg();QVERIFY(write(path,bytes));
  QVERIFY(!bridge.analyzeExif(path,true));QVERIFY(bridge.createCase("Synthetic Exif","Test authorization","Synthetic","No phone","","backup","none",true));
  QVERIFY(!bridge.analyzeExif(path,true));QVERIFY(bridge.recordOperator("Synthetic operator","Fixture only",true));QVERIFY(!bridge.analyzeExif(path,false));
  QVERIFY(bridge.analyzeExif(path,true));QTRY_VERIFY_WITH_TIMEOUT(!bridge.busy(),5000);QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));
  QCOMPARE(bridge.evidence().size(),1);QCOMPARE(read(path),bytes);const auto preview=bridge.previewEvidence(0);QVERIFY(preview.contains("nexvary-exif-summary/v1"));QVERIFY(!preview.contains("private-name.jpg"));
  const auto output=dir.filePath("summary.json");QVERIFY(!bridge.exportEvidence(0,output,false));QVERIFY(bridge.exportEvidence(0,output,true));QVERIFY(!bridge.exportEvidence(0,path,true));
  QVERIFY(bridge.verifyEvidence());QTRY_VERIFY(!bridge.busy());QVERIFY(bridge.error().isEmpty());
  const auto casePath=bridge.casePath();QVERIFY(bridge.closeCase());QVERIFY(bridge.openCase(casePath));QTRY_VERIFY(!bridge.busy());QCOMPARE(bridge.evidence().size(),1);
  QVERIFY(write(path,"invalid"));QVERIFY(bridge.analyzeExif(path,true));QTRY_VERIFY(!bridge.busy());QVERIFY(!bridge.error().isEmpty());QCOMPARE(bridge.evidence().size(),1);
  QVERIFY(write(path,bytes));QVERIFY(bridge.analyzeExif(path,true));bridge.cancel();QTRY_VERIFY(!bridge.busy());QCOMPARE(bridge.evidence().size(),1);
 }
};
QTEST_GUILESS_MAIN(ExifTests)
#include "exif_tests.moc"
