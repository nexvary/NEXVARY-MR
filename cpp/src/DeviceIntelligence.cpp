#include "DeviceIntelligence.hpp"
#include "ConnectionDiagnosis.hpp"
#include "DeviceHealth.hpp"
namespace mr::diagnostics {
namespace {
bool current(const QString &time,const QDateTime &now,qint64 limit=connectionFreshnessMs) {
 const auto at=QDateTime::fromString(time,Qt::ISODateWithMs);
 return at.isValid()&&now.isValid()&&at.msecsTo(now)>=0&&at.msecsTo(now)<=limit;
}
bool unknown(const QString &value) {
 return value.isEmpty()||QStringList{"unknown","n/a","null","undefined","-"}.contains(value.toLower());
}
struct IdentityField {const char *id,*key,*ar,*en;};
constexpr IdentityField identityFields[]{
 {"manufacturer","ro.product.manufacturer","الشركة المصنعة","Manufacturer"},
 {"model","ro.product.model","الموديل المعلن","Reported model"},
 {"brand","ro.product.brand","العلامة التجارية","Brand"},
 {"device","ro.product.device","رمز الجهاز","Device codename"},
 {"chipset","ro.soc.model","المعالج المعلن","Reported SoC"},
 {"hardware","ro.hardware","منصة العتاد المعلنة","Reported hardware platform"},
 {"android","ro.build.version.release","إصدار Android","Android version"},
 {"api","ro.build.version.sdk","مستوى API","API level"},
 {"build","ro.build.fingerprint","بصمة البناء المعلنة","Reported build fingerprint"},
 {"patch","ro.build.version.security_patch","تاريخ التصحيح المعلن","Reported security patch"},
 {"boot","ro.boot.verifiedbootstate","حالة Verified Boot المعلنة","Reported Verified Boot state"},
 {"lock","ro.boot.vbmeta.device_state","حالة Bootloader المعلنة","Reported bootloader state"},
 {"flashLock","ro.boot.flash.locked","قيمة flash.locked المعلنة","Reported flash.locked"},
 {"encryption","ro.crypto.type","نوع التشفير المعلن","Reported encryption type"}
};
}
QVariantMap deviceIntelligence(const DeviceReading &r) {
 QVariantList connections,fields,capabilities,warnings;
 for(const auto &entry:r.devices)if(!r.selectedSerial.isEmpty()&&entry.toMap().value("serial").toString()==r.selectedSerial)connections.append(entry);
 const bool unique=connections.size()==1&&connections.first().toMap().value("state").toString()!="ambiguous";
 const auto connection=unique?connections.first().toMap():QVariantMap();
 const bool propertyTarget=unique&&connection.value("transport").toString()=="adb"&&connection.value("state").toString()=="device"&&r.properties.value("serial").toString()==r.selectedSerial;
 const bool propertiesCurrent=propertyTarget&&current(r.propertiesObservedAtUtc,r.now);
 const bool profileTarget=unique&&r.profile.value("serial").toString()==r.selectedSerial&&r.profile.value("transport")==connection.value("transport");
 bool consistent=true;
 if(propertiesCurrent)for(const auto &key:{"ro.product.manufacturer","ro.product.model","ro.build.fingerprint"})
  if(!unknown(r.properties.value(key).toString())&&!unknown(r.profile.value(key).toString())&&r.properties.value(key)!=r.profile.value(key))consistent=false;
 const bool profileUsable=connection.value("transport").toString()=="adb"?connection.value("state").toString()=="device":connection.value("transport").toString()=="fastboot"&&connection.value("state").toString()=="fastboot";
 const bool profileCurrent=profileTarget&&profileUsable&&consistent&&current(r.profile.value("verifiedAtUtc").toString(),r.now,60000);
 int reported=0;
 for(const auto &f:identityFields) {
  const auto value=r.properties.value(f.key).toString().trimmed();
  const QString state=!propertyTarget?"unavailable":!propertiesCurrent?"expired":unknown(value)?"missing":"reported";
  const bool hasValue=state=="reported";if(hasValue)++reported;
  fields.append(QVariantMap{{"id",f.id},{"key",f.key},{"labelAr",QString::fromUtf8(f.ar)},{"labelEn",f.en},
   {"state",state},{"value",hasValue?value:QString()},{"source",propertyTarget?"adb:shell getprop":"none"},
   {"observedAtUtc",propertyTarget?r.propertiesObservedAtUtc:QString()},{"confidence",hasValue?"device-reported":"unknown"}});
 }
 if(profileCurrent&&connection.value("transport").toString()=="fastboot") {
  const auto value=r.profile.value("product").toString().trimmed();const bool present=!unknown(value);
  fields.append(QVariantMap{{"id","fastbootProduct"},{"key","product"},{"labelAr","منتج Fastboot المعلن"},{"labelEn","Reported Fastboot product"},
   {"state",present?"reported":"missing"},{"value",present?value:QString()},{"source",present?"fastboot:getvar product":"none"},
   {"observedAtUtc",present?r.profile.value("verifiedAtUtc"):QVariant()},{"confidence",present?"device-reported":"unknown"}});
  if(present)++reported;
 }
 auto warn=[&](const QString &id,const QString &ar,const QString &en){warnings.append(QVariantMap{{"id",id},{"ar",ar},{"en",en}});};
 if(profileTarget&&!consistent)warn("profile-identity-conflict","هوية قراءة القدرات تختلف عن الخصائص الحالية؛ لم تدمج قراءات العتاد.","Capability identity differs from current properties; health readings were not merged.");
 if(propertiesCurrent) {
  const auto locked=r.properties.value("ro.boot.flash.locked").toString(),state=r.properties.value("ro.boot.vbmeta.device_state").toString();
  if((locked=="1"&&state=="unlocked")||(locked=="0"&&state=="locked"))
   warn("conflicting-lock-reports","تصريحات القفل متعارضة؛ راجع المصدر ولا تعتمد توافق الكتابة.","Lock declarations conflict; review the source and do not infer write compatibility.");
 }
 const QVariantMap unavailable{{"state","unavailable"},{"source","none"},{"confidence","unknown"}};
 QVariantMap health{{"battery",unavailable},{"storage",unavailable},{"partitions",unavailable}};
 if(profileCurrent&&connection.value("transport").toString()=="adb") {
  health=mr::service::deviceHealth(r.profile);
  for(const auto &category:{QStringLiteral("battery"),QStringLiteral("storage"),QStringLiteral("partitions")}) {
   auto item=health.value(category).toMap();const bool observed=item.value("state").toString()=="observed";
   const QString source=category=="battery"?"adb:shell dumpsys battery":category=="storage"?"adb:shell df -k /data":"adb:shell cat /proc/partitions";
   item.insert("source",observed?source:"none");item.insert("confidence",observed?"device-reported":"unknown");
   item.insert("observedAtUtc",observed?r.profile.value("verifiedAtUtc"):QVariant());health.insert(category,item);
  }
  if(health.value("battery").toMap().value("updatesStopped").toBool())
   warn("battery-updates-stopped","أبلغ Android عن إيقاف تحديثات البطارية؛ القيم قد تكون بديلة ولا تثبت قياسًا ماديًا.","Android reports stopped battery updates; values may be overridden and are not physical measurements.");
 }
 for(const auto &entry:r.services) {
  const auto service=entry.toMap();const bool implemented=service.value("implemented").metaType().id()==QMetaType::Bool&&service.value("implemented").toBool();
  const bool usable=connection.value("transport").toString()=="adb"?QStringList{"device","sideload"}.contains(connection.value("state").toString()):connection.value("transport").toString()=="fastboot"&&connection.value("state").toString()=="fastboot";
  const bool available=unique&&usable&&implemented&&service.value("available").metaType().id()==QMetaType::Bool&&service.value("available").toBool();
  capabilities.append(QVariantMap{{"id",service.value("id")},{"ar",service.value("ar")},{"en",service.value("en")},
   {"state",!implemented?"unsupported":available?"available-for-review":"not-available"},{"reason",service.value("reason")},
   {"source","native-service-policy"},{"executionApproved",false},{"modelTested",false}});
 }
 return {{"schemaVersion",1},{"assessment","device-reported-observations"},{"reportedFieldCount",reported},{"fields",fields},
  {"propertiesCurrent",propertiesCurrent},{"propertiesObservedAtUtc",propertyTarget?r.propertiesObservedAtUtc:QString()},
  {"profileCurrent",profileCurrent},{"health",health},{"capabilities",capabilities},{"warnings",warnings},
  {"usbScope",r.connection.value("usbScope","not-correlated")},{"attestationVerified",false},{"modelAcceptance","not-established"}};
}
}
