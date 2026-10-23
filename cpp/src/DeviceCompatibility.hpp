#pragma once
#include "DeviceHealth.hpp"
#include "ServiceOperationPolicy.hpp"
#include <QDate>

namespace mr::service {
// An observation profile, never a model allowlist, attestation or OEM approval.
// Missing/contradictory values stay unknown; firmware can misreport properties.
inline QVariantMap deviceCompatibility(const QVariantMap &profile) {
 const auto transport=profile.value("transport").toString();
 QVariantMap result{{"schemaVersion",1},{"evidence","device-reported"},{"transport",transport},
  {"hardwareValidation","pending"},{"attestationVerified",false},{"oemOperationsApproved",false},
  {"buildPolicy",laboratoryBuild()?"laboratory-experimental":"production-read-only"}};
 auto value=[&](const char *key){return profile.value(QLatin1String(key)).toString();};
 auto integer=[](const QString &s,int low,int high){
  bool ok=false;const int n=s.toInt(&ok);
  return ok&&QRegularExpression("^[0-9]{1,4}$").match(s).hasMatch()&&n>=low&&n<=high?n:-1;
 };
 auto flag=[&](const char *key){const auto v=value(key);return v=="true"||v=="1"?"reported-enabled":v=="false"||v=="0"?"reported-disabled":"unknown";};
 auto enumerated=[&](const char *key,const QStringList &allowed){const auto v=value(key);return allowed.contains(v)?v:QStringLiteral("unknown");};
 QVariantMap platform{{"state","unavailable"}},layout,security,capabilities;
 if(transport=="adb") {
  const auto sdkText=value("ro.build.version.sdk");const int sdk=integer(sdkText,1,1000);
  if(!sdkText.isEmpty()&&sdk<0)platform.insert("state","invalid-api");
  if(sdk>=0) {
   platform.insert("apiLevel",sdk);
   const QMap<int,QString> versions{{21,"5.0"},{22,"5.1"},{23,"6.0"},{24,"7.0"},{25,"7.1"},{26,"8.0"},{27,"8.1"},{28,"9"},{29,"10"},{30,"11"},{31,"12"},{32,"12L"},{33,"13"},{34,"14"},{35,"15"},{36,"16"},{37,"17"}};
   platform.insert("state",versions.contains(sdk)?"recognized-api":"unrecognized-api");
   if(versions.contains(sdk))platform.insert("androidFamily",versions.value(sdk));
  }
  const auto codename=value("ro.build.version.codename"),previewText=value("ro.build.version.preview_sdk");
  const int preview=integer(previewText,0,1000);
  platform.insert("previewState",codename=="REL"&&preview==0?"reported-release":(!codename.isEmpty()&&codename!="REL")||preview>0?"reported-preview":"unknown");
  const int first=integer(value("ro.product.first_api_level"),1,1000);
  if(first>=0&&sdk>=first)platform.insert("firstApiLevel",first);
  platform.insert("firstApiConsistent",first>=0&&sdk>=first?"reported-consistent":first>=0&&sdk>=0?"inconsistent":"unknown");
  const auto page=value("observation:page-size").trimmed();
  if(QStringList{"4096","16384","65536"}.contains(page))platform.insert("pageSizeBytes",page.toInt());
  layout={{"dynamicPartitions",flag("ro.boot.dynamic_partitions")},{"virtualAb",flag("ro.virtual_ab.enabled")},
   {"abUpdates",flag("ro.build.ab_update")},{"slotSuffix",enumerated("ro.boot.slot_suffix",{"_a","_b"})}};
  security={{"verifiedBoot",enumerated("ro.boot.verifiedbootstate",{"green","yellow","orange","red"})},
   {"vbmetaDeviceState",enumerated("ro.boot.vbmeta.device_state",{"locked","unlocked"})},
   {"flashLocked",enumerated("ro.boot.flash.locked",{"0","1"})},
   {"encryptionState",enumerated("ro.crypto.state",{"encrypted","unencrypted"})},
   {"encryptionType",enumerated("ro.crypto.type",{"file","block"})}};
  const auto patch=value("ro.build.version.security_patch");const auto date=QDate::fromString(patch,Qt::ISODate);
  if(date.isValid()&&date.toString(Qt::ISODate)==patch)security.insert("securityPatch",patch);
  const bool identity=!value("ro.product.manufacturer").isEmpty()&&!value("ro.product.model").isEmpty();
  capabilities.insert("identity",identity?"observed":"unavailable");
  const auto health=deviceHealth(profile);
  for(const auto &key:{"battery","storage","partitions"})capabilities.insert(key,health.value(key).toMap().value("state"));
  capabilities.insert("pageSize",platform.contains("pageSizeBytes")?"observed":"unavailable");
 } else if(transport=="fastboot") {
  layout.insert("fastbootMode",value("is-userspace")=="yes"?"userspace-fastbootd":value("is-userspace")=="no"?"bootloader":"unknown");
  security.insert("bootloaderUnlocked",enumerated("unlocked",{"yes","no"}));
  capabilities.insert("identity",!value("product").isEmpty()?"observed":"unavailable");
 }
 result.insert("platform",platform);result.insert("layout",layout);result.insert("security",security);result.insert("capabilities",capabilities);
 return result;
}
// Explicit technical export projection; no raw property text or user data.
inline QVariantMap compatibilityReport(const QVariantMap &source) {
 QVariantMap result;
 for(const auto &key:{"schemaVersion","evidence","transport","hardwareValidation","attestationVerified","oemOperationsApproved","buildPolicy"})
  if(source.contains(key))result.insert(key,source.value(key));
 const QMap<QString,QStringList> fields{
  {"platform",{"state","apiLevel","androidFamily","previewState","firstApiLevel","firstApiConsistent","pageSizeBytes"}},
  {"layout",{"dynamicPartitions","virtualAb","abUpdates","slotSuffix","fastbootMode"}},
  {"security",{"verifiedBoot","vbmetaDeviceState","flashLocked","encryptionState","encryptionType","securityPatch","bootloaderUnlocked"}},
  {"capabilities",{"identity","battery","storage","partitions","pageSize"}}};
 for(auto it=fields.cbegin();it!=fields.cend();++it){QVariantMap map;const auto input=source.value(it.key()).toMap();for(const auto &key:it.value())if(input.contains(key))map.insert(key,input.value(key));result.insert(it.key(),map);}
 return result;
}
}
