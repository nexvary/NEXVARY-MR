#pragma once
#include <QVariantList>
#include <QVariantMap>
namespace nexvary::law {
// A projection of already collected observations. It cannot run a USB command,
// grant a capability, or establish key availability from a model/property name.
inline QVariantList assessmentFields(const QVariantMap &report) {
 struct Field {const char *id,*ar,*en;};
 static constexpr Field fields[]{
  {"manufacturer","الشركة المصنعة","Manufacturer"},{"model","الموديل","Model"},
  {"chipset","المعالج / SoC","Chipset / SoC"},{"hardware","منصة العتاد","Hardware platform"},
  {"build","إصدار النظام الثابت","Firmware fingerprint"},{"android","إصدار Android","Android version"},
  {"ios","إصدار iOS","iOS version"},{"patch","مستوى تصحيح الأمان","Security patch level"},
  {"lock","حالة Bootloader","Bootloader state"},{"boot","حالة Verified Boot","Verified Boot state"},
  {"encryption","نوع تشفير البيانات","Data encryption type"},
  {"tee","TEE / Secure Enclave","TEE / Secure Enclave"}
 };
 QVariantList rows;
 for(const auto &f:fields){
  QVariantMap observed;
  for(const auto &entry:report.value("fields").toList())if(entry.toMap().value("id").toString()==f.id){observed=entry.toMap();break;}
  const bool fresh=report.value("propertiesCurrent").toBool()&&observed.value("state")=="reported"&&!observed.value("value").toString().isEmpty();
  rows.append(QVariantMap{{"id",QStringLiteral("device-")+f.id},{"ar",QString::fromUtf8(f.ar)},{"en",f.en},
   {"state",fresh?"Device-reported":"Unknown"},{"evidenceClass",fresh?"reported":"research-hypothesis"},
   {"fact",fresh?observed.value("value").toString()+QStringLiteral(" | ")+observed.value("source").toString()+QStringLiteral(" | ")+observed.value("observedAtUtc").toString():QStringLiteral("No fresh supported observation. No value inferred from manufacturer or model.")},
   {"confidence",fresh?"device-reported-not-attested":"unknown"},{"executionApproved",false},{"physicalDeviceTested",false}});
 }
 rows.append(QVariantMap{{"id","acquisition-risk"},{"ar","مخاطر الاستخلاص"},{"en","Acquisition risk assessment"},{"state","Review required"},{"evidenceClass","research-hypothesis"},
  {"fact","Risk remains unassessed for this exact device. Preserve power/connection state; do not unlock, reboot, flash, install agents or retry an unavailable interface on the basis of these observations."},{"executionApproved",false},{"physicalDeviceTested",false}});
 return rows;
}
}
