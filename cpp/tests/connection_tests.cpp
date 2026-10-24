#include "ConnectionDiagnosis.hpp"
#include "UsbDriver.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTest>
using mr::diagnostics::ConnectionSnapshot;
namespace {
const QDateTime now=QDateTime::fromString("2026-10-24T08:00:00.000Z",Qt::ISODateWithMs);
QVariantMap observation(const QString &state="ok",int offset=0) {
 return {{"state",state},{"observedAtUtc",now.addMSecs(offset).toString(Qt::ISODateWithMs)}};
}
ConnectionSnapshot snapshot(const QString &state="device",const QString &transport="adb") {
 ConnectionSnapshot s;s.now=now;s.selectedSerial="SYNTHETIC42";
 s.devices={QVariantMap{{"serial",s.selectedSerial},{"transport",transport},{"state",state}}};
 s.scans={{transport,observation()}};return s;
}
QVariantMap finding(const QVariantMap &report,const QString &id) {
 for(const auto &entry:report.value("findings").toList())if(entry.toMap().value("id").toString()==id)return entry.toMap();
 return {};
}
QVariantMap node(const QString &serial,int code) {
 return {{"instanceId","USB\\VID_1234&PID_5678\\"+serial},{"problemCode",code},{"name","Synthetic Phone"}};
}
void setUsb(ConnectionSnapshot &s,const QVariantList &nodes) {
 s.usb=observation();s.usb.insert("inventory",QVariantMap{{"schemaVersion",1},{"devices",nodes},{"signatureInventoryAvailable",true}});
}
QByteArray inventoryBytes(const QJsonArray &nodes) {
 return QJsonDocument(QJsonObject{{"schemaVersion",1},{"devices",nodes}}).toJson();
}
}
class ConnectionTests:public QObject {
 Q_OBJECT
private slots:
 void initialStateHasNoHealthyClaim() {
  ConnectionSnapshot s;s.now=now;const auto r=mr::diagnostics::connectionDiagnosis(s);
  QVERIFY(!finding(r,"select-target").isEmpty());QVERIFY(finding(r,"no-transport-device").isEmpty());
  QCOMPARE(r.value("assessment").toString(),QString("observations-only"));
  QVERIFY(!r.value("physicalCauseVerified").toBool());QVERIFY(!r.value("interfaceBindingVerified").toBool());
 }
 void transportStates_data() {
  QTest::addColumn<QString>("state");QTest::addColumn<QString>("id");
  QTest::newRow("authorized")<<QString("device")<<QString("adb-observed");
  QTest::newRow("unauthorized")<<QString("unauthorized")<<QString("adb-unauthorized");
  QTest::newRow("offline")<<QString("offline")<<QString("adb-offline");
  for(const auto &state:{"recovery","sideload","rescue","no permissions","bootloader"})QTest::newRow(state)<<QString(state)<<QString("adb-mode");
 }
 void transportStates() {
  QFETCH(QString,state);QFETCH(QString,id);const auto r=mr::diagnostics::connectionDiagnosis(snapshot(state));
  const auto f=finding(r,id);QVERIFY(!f.isEmpty());QCOMPARE(f.value("confidence").toString(),QString("reported"));
  QVERIFY(!f.value("nextAr").toString().isEmpty());QVERIFY(!f.value("nextEn").toString().isEmpty());
  if(state!="device")QVERIFY(finding(r,"adb-observed").isEmpty());
 }
 void successfulEmptyIsNotACableDiagnosis() {
  ConnectionSnapshot s;s.now=now;s.scans={{"adb",observation()}};
  const auto f=finding(mr::diagnostics::connectionDiagnosis(s),"no-transport-device");
  QCOMPARE(f.value("confidence").toString(),QString("unknown"));
 }
 void failedAndRunningScansDoNotUseOldAuthorizedRows() {
  for(const auto &state:{"failed","running","unavailable"}) {
   auto s=snapshot();s.scans={{"adb",observation(state)}};const auto r=mr::diagnostics::connectionDiagnosis(s);
   QVERIFY(finding(r,"adb-observed").isEmpty());QVERIFY(finding(r,"no-transport-device").isEmpty());
   QVERIFY(!finding(r,"target-unresolved").isEmpty());
  }
 }
 void freshness_data() {
  QTest::addColumn<int>("offset");QTest::addColumn<bool>("accepted");
  QTest::newRow("now")<<0<<true;QTest::newRow("boundary")<<-120000<<true;
  QTest::newRow("expired")<<-120001<<false;QTest::newRow("future")<<1<<false;
 }
 void freshness() {
  QFETCH(int,offset);QFETCH(bool,accepted);auto s=snapshot();s.scans={{"adb",observation("ok",offset)}};
  QCOMPARE(!finding(mr::diagnostics::connectionDiagnosis(s),"adb-observed").isEmpty(),accepted);
 }
 void invalidTimeIsUnknown() {
  auto s=snapshot();s.scans={{"adb",QVariantMap{{"state","ok"},{"observedAtUtc","not-time"}}}};
  QVERIFY(finding(mr::diagnostics::connectionDiagnosis(s),"adb-observed").isEmpty());
 }
 void ambiguousSerialDoesNotChooseATransport() {
  auto s=snapshot();s.scans.insert("fastboot",observation());
  s.devices.append(QVariantMap{{"serial",s.selectedSerial},{"transport","fastboot"},{"state","fastboot"}});
  const auto r=mr::diagnostics::connectionDiagnosis(s);QVERIFY(!finding(r,"target-unresolved").isEmpty());
  QVERIFY(finding(r,"adb-observed").isEmpty());QVERIFY(finding(r,"fastboot-mode").isEmpty());
  s=snapshot("ambiguous");QVERIFY(!finding(mr::diagnostics::connectionDiagnosis(s),"target-unresolved").isEmpty());
 }
 void fastbootSubtypeRequiresFreshProfileForSameTarget() {
  auto s=snapshot("fastboot","fastboot");
  QVERIFY(finding(mr::diagnostics::connectionDiagnosis(s),"fastboot-mode").value("titleEn").toString().contains("unread"));
  s.profile={{"serial",s.selectedSerial},{"transport","fastboot"},{"is-userspace","yes"},{"verifiedAtUtc",observation().value("observedAtUtc")}};
  QVERIFY(finding(mr::diagnostics::connectionDiagnosis(s),"fastboot-mode").value("titleEn").toString().contains("fastbootd"));
  s.profile.insert("serial","OTHER");QVERIFY(finding(mr::diagnostics::connectionDiagnosis(s),"fastboot-mode").value("titleEn").toString().contains("unread"));
  s.profile.insert("serial",s.selectedSerial);s.profile.insert("verifiedAtUtc",observation("ok",-120001).value("observedAtUtc"));
  QVERIFY(finding(mr::diagnostics::connectionDiagnosis(s),"fastboot-mode").value("titleEn").toString().contains("unread"));
 }
 void correlatedErrorsCannotComeFromAnotherPhone() {
  auto s=snapshot();setUsb(s,{node(s.selectedSerial,28),node("OTHER",43)});
  const auto r=mr::diagnostics::connectionDiagnosis(s);QCOMPARE(r.value("usbScope").toString(),QString("serial-correlated"));
  QVERIFY(!finding(r,"pnp-28").isEmpty());QVERIFY(finding(r,"pnp-43").isEmpty());
  const auto n=r.value("nodes").toList().first().toMap();QCOMPARE(n.value("vid").toString(),QString("1234"));QCOMPARE(n.value("pid").toString(),QString("5678"));
  QVERIFY(!r.value("interfaceDescriptorsRead").toBool());
 }
 void noMatchDoesNotAttributeUnrelatedErrors() {
  auto s=snapshot();setUsb(s,{node("OTHER",43)});const auto r=mr::diagnostics::connectionDiagnosis(s);
  QVERIFY(finding(r,"pnp-43").isEmpty());QVERIFY(!finding(r,"usb-not-correlated").isEmpty());
 }
 void hostInventoryWorksWithoutATarget() {
  ConnectionSnapshot s;s.now=now;setUsb(s,{node("OTHER",52)});const auto r=mr::diagnostics::connectionDiagnosis(s);
  QCOMPARE(r.value("usbScope").toString(),QString("host-inventory-only"));
  QVERIFY(finding(r,"pnp-52").value("detailEn").toString().contains("not attributed"));
 }
 void problemCodes_data() {
  QTest::addColumn<int>("code");for(const int code:{10,22,28,31,43,52,65535})QTest::newRow(qPrintable(QString::number(code)))<<code;
 }
 void problemCodes() {
  QFETCH(int,code);auto s=snapshot();setUsb(s,{node(s.selectedSerial,code)});
  const auto f=finding(mr::diagnostics::connectionDiagnosis(s),"pnp-"+QString::number(code));QVERIFY(!f.isEmpty());
  QCOMPARE(f.value("confidence").toString(),QString("reported"));
 }
 void expiredInventoryAndUnknownCodesCannotPassAsHealthy() {
  auto s=snapshot();auto n=node(s.selectedSerial,0);n.remove("problemCode");setUsb(s,{n});
  auto r=mr::diagnostics::connectionDiagnosis(s);QVERIFY(finding(r,"pnp-0").isEmpty());QVERIFY(!r.value("physicalCauseVerified").toBool());
  s.usb.insert("observedAtUtc",observation("ok",-120001).value("observedAtUtc"));r=mr::diagnostics::connectionDiagnosis(s);
  QVERIFY(r.value("nodes").toList().isEmpty());QVERIFY(!finding(r,"usb-stale").isEmpty());
 }
 void malformedInventoryIsRejected_data() {
  QTest::addColumn<QByteArray>("bytes");const QJsonObject valid{{"instanceId","USB\\VID_1234&PID_5678\\SYNTHETIC42"}};
  QTest::newRow("bad-json")<<QByteArray("{");QTest::newRow("oversized")<<QByteArray(513*1024,'x');
  QTest::newRow("duplicate")<<inventoryBytes({valid,valid});
  for(const QJsonValue &value:{QJsonValue(-1),QJsonValue(1.5),QJsonValue(65536),QJsonValue("28"),QJsonValue(true)}) {
   auto n=valid;n.insert("problemCode",value);QTest::newRow(qPrintable(QJsonDocument(QJsonArray{value}).toJson(QJsonDocument::Compact)))<<inventoryBytes({n});
  }
  auto n=valid;n.insert("signed","yes");QTest::newRow("signature-type")<<inventoryBytes({n});
  n=valid;n.insert("name",QString("bad\nname"));QTest::newRow("control-character")<<inventoryBytes({n});
  QJsonArray many;for(int i=0;i<257;++i)many.append(QJsonObject{{"instanceId","USB\\ROOT\\"+QString::number(i)}});
  QTest::newRow("too-many")<<inventoryBytes(many);
 }
 void malformedInventoryIsRejected() {QFETCH(QByteArray,bytes);QVERIFY(mr::usb::inventory(bytes).isEmpty());}
};
QTEST_GUILESS_MAIN(ConnectionTests)
#include "connection_tests.moc"
