#include "PcapSummary.hpp"
#include "RecoveryBridge.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
#include <QtEndian>
#include <limits>
namespace {
void put16(QByteArray &b,qsizetype at,quint16 value,bool little=true){if(little)qToLittleEndian(value,b.data()+at);else qToBigEndian(value,b.data()+at);}
void put32(QByteArray &b,qsizetype at,quint32 value,bool little=true){if(little)qToLittleEndian(value,b.data()+at);else qToBigEndian(value,b.data()+at);}
QByteArray header(bool little=true,bool nano=false){QByteArray b(24,'\0');put32(b,0,nano?0xa1b23c4dU:0xa1b2c3d4U,little);put16(b,4,2,little);put16(b,6,4,little);put32(b,16,65535,little);put32(b,20,1,little);return b;}
void packet(QByteArray &b,const QByteArray &data,quint32 seconds=2,quint32 fraction=5,bool little=true){const auto at=b.size();b.append(QByteArray(16,'\0'));put32(b,at,seconds,little);put32(b,at+4,fraction,little);put32(b,at+8,static_cast<quint32>(data.size()),little);put32(b,at+12,static_cast<quint32>(data.size()),little);b.append(data);}
QByteArray ipv4(){QByteArray b(14+20,'\0');put16(b,12,0x0800,false);b[14]='\x45';put16(b,16,20,false);b[23]=6;b.replace(26,4,QByteArray::fromHex("c0000201"));return b;}
bool save(const QString &path,const QByteArray &bytes){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();}
QByteArray read(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
}
class PcapTests final:public QObject {
 Q_OBJECT
private slots:
 void endianAndResolution_data(){QTest::addColumn<bool>("little");QTest::addColumn<bool>("nano");QTest::newRow("LE-us")<<true<<false;QTest::newRow("BE-us")<<false<<false;QTest::newRow("LE-ns")<<true<<true;QTest::newRow("BE-ns")<<false<<true;}
 void endianAndResolution(){
  QFETCH(bool,little);QFETCH(bool,nano);auto b=header(little,nano);packet(b,ipv4(),2,5,little);
  const auto r=mr::forensic::summarizeCapture(b);QVERIFY2(r.error.isEmpty(),qPrintable(r.error));
  QCOMPARE(r.report["packetCount"].toString(),QStringLiteral("1"));QCOMPARE(r.report["minimumTimestampNs"].toString(),nano?QStringLiteral("2000000005"):QStringLiteral("2000005000"));
  QCOMPARE(r.report["headerCounts"].toObject()["tcpProtocol"].toString(),QStringLiteral("1"));
  QVERIFY(!r.report["packetPayloadIncluded"].toBool(true));QVERIFY(!r.report["addressesIncluded"].toBool(true));QVERIFY(!r.report["decryptionPerformed"].toBool(true));
  QCOMPARE(r.report["sourceSha256"].toString(),QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex()));
 }
 void hostileRecords_data(){
  QTest::addColumn<int>("offset");QTest::addColumn<quint32>("value");QTest::addColumn<QString>("error");
  QTest::newRow("snap-zero")<<16<<quint32(0)<<QStringLiteral("snapshot-limit");
  QTest::newRow("snap-huge")<<16<<std::numeric_limits<quint32>::max()<<QStringLiteral("snapshot-limit");
  QTest::newRow("fraction")<<28<<quint32(1000000)<<QStringLiteral("invalid-timestamp-fraction");
  QTest::newRow("cap-overflow")<<32<<std::numeric_limits<quint32>::max()<<QStringLiteral("inconsistent-record-length");
  QTest::newRow("wire-smaller")<<36<<quint32(0)<<QStringLiteral("inconsistent-record-length");
 }
 void hostileRecords(){QFETCH(int,offset);QFETCH(quint32,value);QFETCH(QString,error);auto b=header();packet(b,ipv4());put32(b,offset,value);const auto r=mr::forensic::summarizeCapture(b);QCOMPARE(r.error,error);QVERIFY(r.report.isEmpty());}
 void truncationAndFormats(){
  auto b=header();packet(b,ipv4());for(qsizetype i=0;i<b.size();++i){if(i==24)continue;const auto r=mr::forensic::summarizeCapture(b.left(i));QVERIFY2(!r.error.isEmpty(),qPrintable(QString::number(i)));QVERIFY(r.report.isEmpty());}
  auto other=header();other.replace(0,4,QByteArray::fromHex("0a0d0d0a"));QCOMPARE(mr::forensic::summarizeCapture(other).error,QStringLiteral("classic-pcap-required"));
  other=header();put16(other,6,3);QCOMPARE(mr::forensic::summarizeCapture(other).error,QStringLiteral("unsupported-pcap-version"));
  b.append('x');QCOMPARE(mr::forensic::summarizeCapture(b).error,QStringLiteral("truncated-record-header"));
  QCOMPARE(mr::forensic::summarizeCapture(QByteArray(mr::forensic::maximumCaptureBytes+1,'\0')).error,QStringLiteral("capture-size-limit"));
 }
 void clocksAndPartialCapture(){
  auto b=header();packet(b,ipv4(),4);packet(b,QByteArray("PRIVATE-PAYLOAD"),1);put32(b,24+16+34+12,999);
  const auto r=mr::forensic::summarizeCapture(b);QVERIFY(r.error.isEmpty());QCOMPARE(r.report["packetCount"].toString(),QStringLiteral("2"));QCOMPARE(r.report["truncatedPackets"].toString(),QStringLiteral("1"));QCOMPARE(r.report["outOfOrderTimestamps"].toString(),QStringLiteral("1"));
  const auto json=QJsonDocument(r.report).toJson();QVERIFY(!json.contains("PRIVATE-PAYLOAD"));QVERIFY(!json.contains("192.0.2.1"));
  const auto empty=mr::forensic::summarizeCapture(header());QVERIFY(empty.error.isEmpty());QVERIFY(empty.report["minimumTimestampNs"].isNull());
 }
 void networkScope(){
  auto b=header();auto vlan=ipv4();vlan.insert(14,QByteArray::fromHex("00010800"));put16(vlan,12,0x8100,false);packet(b,vlan);
  QByteArray v6(54,'\0');put16(v6,12,0x86dd,false);v6[14]='\x60';v6[20]=17;packet(b,v6);
  packet(b,ipv4().left(10));auto invalid=ipv4();invalid[14]='\x41';packet(b,invalid);
  auto counts=mr::forensic::summarizeCapture(b).report["headerCounts"].toObject();QCOMPARE(counts["ipv4"].toString(),QStringLiteral("1"));QCOMPARE(counts["ipv6"].toString(),QStringLiteral("1"));QCOMPARE(counts["udpProtocol"].toString(),QStringLiteral("1"));QCOMPARE(counts["unparsedEthernetPackets"].toString(),QStringLiteral("2"));
  put32(b,20,113);const auto unknown=mr::forensic::summarizeCapture(b);QVERIFY(unknown.error.isEmpty());QVERIFY(!unknown.report["ethernetHeaderStatisticsAvailable"].toBool(true));QCOMPARE(unknown.report["headerCounts"].toObject()["ipv4"].toString(),QStringLiteral("0"));
 }
 void cancellationAndRecordBound(){
  auto token=std::make_shared<std::atomic_bool>(true);const auto r=mr::forensic::summarizeCapture(header(),token);QCOMPARE(r.error,QStringLiteral("cancelled"));QVERIFY(r.report.isEmpty());
  auto b=header();for(int i=0;i<200001;++i)packet(b,{});QCOMPARE(mr::forensic::summarizeCapture(b).error,QStringLiteral("record-count-limit"));
 }
 void caseConsentPersistenceAndSourcePreservation(){
  QTemporaryDir dir;QVERIFY(dir.isValid());const auto path=dir.filePath("synthetic-private.pcap");auto bytes=header();packet(bytes,ipv4());QVERIFY(save(path,bytes));
  RecoveryBridge bridge(dir.filePath("cases"),true);QVERIFY(!bridge.analyzeCapture(path,true));
  QVERIFY(bridge.createCase("Synthetic capture","Authorization fixture","Synthetic","No phone","","backup","none",true));QVERIFY(!bridge.analyzeCapture(path,true));
  QVERIFY(bridge.recordOperator("Synthetic operator","Fixture-only authorization",true));QVERIFY(!bridge.analyzeCapture(path,false));QVERIFY(bridge.evidence().isEmpty());
  QVERIFY(bridge.analyzeCapture(path,true));QTRY_VERIFY_WITH_TIMEOUT(!bridge.busy(),5000);QVERIFY2(bridge.error().isEmpty(),qPrintable(bridge.error()));QCOMPARE(bridge.evidence().size(),1);QCOMPARE(read(path),bytes);
  const auto preview=bridge.previewEvidence(0);QVERIFY(preview.contains("pcap-summary"));QVERIFY(!preview.contains("synthetic-private.pcap"));
  const auto output=dir.filePath("summary.json");QVERIFY(!bridge.exportEvidence(0,output,false));QVERIFY(bridge.exportEvidence(0,output,true));QVERIFY(!bridge.exportEvidence(0,path,true));QCOMPARE(read(path),bytes);
  QVERIFY(bridge.verifyEvidence());QTRY_VERIFY(!bridge.busy());QVERIFY(bridge.error().isEmpty());
  const auto casePath=bridge.casePath();QVERIFY(bridge.closeCase());QVERIFY(bridge.openCase(casePath));QTRY_VERIFY(!bridge.busy());QCOMPARE(bridge.evidence().size(),1);
  QVERIFY(save(path,"invalid"));QVERIFY(bridge.analyzeCapture(path,true));QTRY_VERIFY(!bridge.busy());QVERIFY(!bridge.error().isEmpty());QCOMPARE(bridge.evidence().size(),1);
  QVERIFY(save(path,bytes));QVERIFY(bridge.analyzeCapture(path,true));bridge.cancel();QTRY_VERIFY(!bridge.busy());QCOMPARE(bridge.evidence().size(),1);
 }
};
QTEST_GUILESS_MAIN(PcapTests)
#include "pcap_tests.moc"
