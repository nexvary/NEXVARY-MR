#include "DeviceIntelligence.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>
using mr::diagnostics::DeviceReading;
namespace {
const auto now=QDateTime::fromString("2026-10-24T09:00:00.000Z",Qt::ISODateWithMs);
DeviceReading reading() {
 DeviceReading r;r.now=now;r.selectedSerial="SYNTHETIC42";r.propertiesObservedAtUtc=now.toString(Qt::ISODateWithMs);
 r.devices={QVariantMap{{"serial",r.selectedSerial},{"transport","adb"},{"state","device"}}};
 r.properties={{"serial",r.selectedSerial},{"ro.product.manufacturer","Synthetic Lab"},{"ro.product.model","Training Phone"},{"ro.hardware","fallback-platform"}};
 return r;
}
QVariantMap field(const QVariantMap &report,const QString &id) {
 for(const auto &entry:report.value("fields").toList())if(entry.toMap().value("id").toString()==id)return entry.toMap();
 return {};
}
bool warning(const QVariantMap &report,const QString &id) {
 for(const auto &entry:report.value("warnings").toList())if(entry.toMap().value("id").toString()==id)return true;
 return false;
}
}
class IntelligenceTests:public QObject {
 Q_OBJECT
private slots:
 void valuesHaveSourceAndCollectionTime() {
  const auto r=reading();const auto report=mr::diagnostics::deviceIntelligence(r);const auto model=field(report,"model");
  QCOMPARE(model.value("value").toString(),QString("Training Phone"));QCOMPARE(model.value("state").toString(),QString("reported"));
  QCOMPARE(model.value("source").toString(),QString("adb:shell getprop"));QCOMPARE(model.value("observedAtUtc").toString(),r.propertiesObservedAtUtc);
  QCOMPARE(model.value("confidence").toString(),QString("device-reported"));QVERIFY(!report.value("attestationVerified").toBool());
  QCOMPARE(report.value("modelAcceptance").toString(),QString("not-established"));
 }
 void unknownSocIsNotFilledFromAnotherPropertyOrCatalog() {
  const auto report=mr::diagnostics::deviceIntelligence(reading());QCOMPARE(field(report,"chipset").value("state").toString(),QString("missing"));
  QVERIFY(field(report,"chipset").value("value").toString().isEmpty());QCOMPARE(field(report,"hardware").value("value").toString(),QString("fallback-platform"));
 }
 void unavailableTarget_data() {
  QTest::addColumn<QString>("state");for(const auto &state:{"offline","unauthorized","recovery","sideload","ambiguous"})QTest::newRow(state)<<QString(state);
 }
 void unavailableTarget() {
  QFETCH(QString,state);auto r=reading();r.devices={QVariantMap{{"serial",r.selectedSerial},{"transport","adb"},{"state",state}}};
  const auto report=mr::diagnostics::deviceIntelligence(r);QCOMPARE(report.value("reportedFieldCount").toInt(),0);QVERIFY(field(report,"model").value("value").toString().isEmpty());
 }
 void wrongAndAmbiguousTargetsCannotInheritIdentity() {
  auto r=reading();r.selectedSerial="OTHER";QCOMPARE(mr::diagnostics::deviceIntelligence(r).value("reportedFieldCount").toInt(),0);
  r=reading();r.devices.append(QVariantMap{{"serial",r.selectedSerial},{"transport","fastboot"},{"state","fastboot"}});
  QCOMPARE(mr::diagnostics::deviceIntelligence(r).value("reportedFieldCount").toInt(),0);
  r=reading();r.properties.insert("serial","OTHER");QCOMPARE(mr::diagnostics::deviceIntelligence(r).value("reportedFieldCount").toInt(),0);
 }
 void expiredAndFuturePropertiesHaveNoCurrentValues_data() {
  QTest::addColumn<int>("offset");QTest::newRow("expired")<<-120001;QTest::newRow("future")<<1;
 }
 void expiredAndFuturePropertiesHaveNoCurrentValues() {
  QFETCH(int,offset);auto r=reading();r.propertiesObservedAtUtc=now.addMSecs(offset).toString(Qt::ISODateWithMs);
  const auto report=mr::diagnostics::deviceIntelligence(r);QCOMPARE(field(report,"model").value("state").toString(),QString("expired"));
  QVERIFY(field(report,"model").value("value").toString().isEmpty());QVERIFY(!report.value("propertiesCurrent").toBool());
 }
 void missingValuesRetainTheirReadSource() {
  auto r=reading();r.properties.insert("ro.soc.model","unknown");const auto f=field(mr::diagnostics::deviceIntelligence(r),"chipset");
  QCOMPARE(f.value("state").toString(),QString("missing"));QCOMPARE(f.value("source").toString(),QString("adb:shell getprop"));
  QCOMPARE(f.value("confidence").toString(),QString("unknown"));
 }
 void conflictingLockDeclarationsAreFlagged() {
  auto r=reading();r.properties.insert("ro.boot.flash.locked","1");r.properties.insert("ro.boot.vbmeta.device_state","unlocked");
  QVERIFY(warning(mr::diagnostics::deviceIntelligence(r),"conflicting-lock-reports"));
 }
 void healthRequiresFreshSameTargetProbe() {
  auto r=reading();r.profile={{"serial",r.selectedSerial},{"transport","adb"},{"verifiedAtUtc",r.propertiesObservedAtUtc},
   {"observation:battery","level: 0\nscale: 100\ntemperature: 0\n"},
   {"observation:storage","Filesystem 1K-blocks Used Available Use% Mounted on\n/dev/test 102400 51200 51200 50% /data"},
   {"observation:partitions","179 0 1024 mmcblk0\n"}};
  const auto health=mr::diagnostics::deviceIntelligence(r).value("health").toMap();
  const auto battery=health.value("battery").toMap();QCOMPARE(battery.value("chargePercent").toDouble(),0.0);QCOMPARE(battery.value("temperatureC").toDouble(),0.0);
  QCOMPARE(battery.value("source").toString(),QString("adb:shell dumpsys battery"));
  QCOMPARE(health.value("storage").toMap().value("availableKiB").toLongLong(),51200);
  QCOMPARE(health.value("partitions").toMap().value("entries").toList().size(),1);
  r.profile.insert("serial","OTHER");QCOMPARE(mr::diagnostics::deviceIntelligence(r).value("health").toMap().value("battery").toMap().value("state").toString(),QString("unavailable"));
  r.profile.insert("serial",r.selectedSerial);r.profile.insert("verifiedAtUtc",now.addMSecs(-120001).toString(Qt::ISODateWithMs));
  QVERIFY(!mr::diagnostics::deviceIntelligence(r).value("profileCurrent").toBool());
 }
 void batteryOverridesAreExplained() {
  auto r=reading();r.profile={{"serial",r.selectedSerial},{"transport","adb"},{"verifiedAtUtc",r.propertiesObservedAtUtc},{"observation:battery","UPDATES STOPPED -- use 'reset' to restart\nlevel: 80\nscale: 100\n"}};
  QVERIFY(warning(mr::diagnostics::deviceIntelligence(r),"battery-updates-stopped"));
 }
 void mixedBuildReadingsAreNotMerged() {
  auto r=reading();r.properties.insert("ro.build.fingerprint","new-build");
  r.profile={{"serial",r.selectedSerial},{"transport","adb"},{"verifiedAtUtc",r.propertiesObservedAtUtc},{"ro.build.fingerprint","old-build"},{"observation:battery","level: 80\nscale: 100\n"}};
  const auto report=mr::diagnostics::deviceIntelligence(r);QVERIFY(!report.value("profileCurrent").toBool());
  QVERIFY(warning(report,"profile-identity-conflict"));QCOMPARE(report.value("health").toMap().value("battery").toMap().value("state").toString(),QString("unavailable"));
 }
 void fastbootProductIsNotAnAndroidModelAssertion() {
  auto r=reading();r.devices={QVariantMap{{"serial",r.selectedSerial},{"transport","fastboot"},{"state","fastboot"}}};
  r.profile={{"serial",r.selectedSerial},{"transport","fastboot"},{"verifiedAtUtc",r.propertiesObservedAtUtc},{"product","synthetic_product"}};
  const auto report=mr::diagnostics::deviceIntelligence(r);QCOMPARE(field(report,"fastbootProduct").value("value").toString(),QString("synthetic_product"));
  QVERIFY(field(report,"model").value("value").toString().isEmpty());QVERIFY(field(report,"android").value("value").toString().isEmpty());
 }
 void serviceAvailabilityNeverApprovesExecutionOrHardware() {
  auto r=reading();r.services={QVariantMap{{"id","probe"},{"implemented",true},{"available",true}},QVariantMap{{"id","not-real"},{"implemented",false},{"available",true}},QVariantMap{{"id","malformed"},{"implemented",true},{"available","true"}}};
  const auto rows=mr::diagnostics::deviceIntelligence(r).value("capabilities").toList();
  QCOMPARE(rows[0].toMap().value("state").toString(),QString("available-for-review"));QCOMPARE(rows[1].toMap().value("state").toString(),QString("unsupported"));QCOMPARE(rows[2].toMap().value("state").toString(),QString("not-available"));
  for(const auto &row:rows){QVERIFY(!row.toMap().value("executionApproved").toBool());QVERIFY(!row.toMap().value("modelTested").toBool());}
 }
 void unrelatedPrivatePropertiesAreNotProjected() {
  auto r=reading();r.properties.insert("persist.private","PRIVATE-SENTINEL");r.properties.insert("ro.serialno","PRIVATE-SERIAL");
  const auto bytes=QJsonDocument(QJsonObject::fromVariantMap(mr::diagnostics::deviceIntelligence(r))).toJson();
  QVERIFY(!bytes.contains("PRIVATE-SENTINEL"));QVERIFY(!bytes.contains("PRIVATE-SERIAL"));
 }
};
QTEST_GUILESS_MAIN(IntelligenceTests)
#include "intelligence_tests.moc"
