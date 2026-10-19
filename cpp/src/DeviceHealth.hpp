#pragma once
#include <QRegularExpression>
#include <QVariantMap>
#include <QVariantList>
#include <QSet>

namespace mr::service {
// Interpret bounded observations, never infer hardware health from missing data.
// Raw observations remain in the profile for independent review.
inline QVariantMap deviceHealth(const QVariantMap &profile) {
 QVariantMap result{{"schemaVersion",1},{"assessment","observations-only"},
                    {"transport",profile.value("transport")},
                    {"observedAtUtc",profile.value("verifiedAtUtc")}};
 QVariantMap battery{{"state","unavailable"}},storage{{"state","unavailable"}},partitions{{"state","unavailable"}};
 if(profile.value("transport").toString()=="adb") {
  QMap<QString,QString> values;QSet<QString> duplicate;
  const auto raw=profile.value("observation:battery").toString();
  battery.insert("updatesStopped",raw.contains("UPDATES STOPPED",Qt::CaseInsensitive));
  for(const auto &line:raw.split('\n')) {
   const auto pos=line.indexOf(':');if(pos<0)continue;
   const auto key=line.left(pos).trimmed();
   if(values.contains(key))duplicate.insert(key);
   values.insert(key,line.mid(pos+1).trimmed());
  }
  auto number=[&](const QString &key,int low,int high,int &value){
   bool ok=false;const auto text=values.value(key);
   if(duplicate.contains(key)||!QRegularExpression("^-?[0-9]+$").match(text).hasMatch())return false;
   value=text.toInt(&ok);return ok&&value>=low&&value<=high;
  };
  int level=0,scale=0,temp=0,status=0,health=0;
  if(number("level",0,100000,level)&&number("scale",1,100000,scale)&&level<=scale)
   battery.insert("chargePercent",100.0*level/scale);
  if(number("temperature",-1000,2000,temp))battery.insert("temperatureC",temp/10.0);
  if(number("status",1,5,status))battery.insert("androidStatusCode",status);
  if(number("health",1,7,health))battery.insert("androidHealthCode",health);
  if(battery.size()>2)battery.insert("state","observed");
  const auto disk=profile.value("observation:storage").toString();
  int rows=0;
  for(const auto &line:disk.split('\n')) {
   const auto f=line.trimmed().split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);
   if(f.size()!=6||f.last()!="/data")continue;
   ++rows;bool totalOk=false,usedOk=false,freeOk=false;
   const auto total=f[1].toLongLong(&totalOk),used=f[2].toLongLong(&usedOk),free=f[3].toLongLong(&freeOk);
   if(!totalOk||!usedOk||!freeOk||total<=0||used<0||free<0||used>total||free>total)continue;
   // df -k reports KiB; no byte multiplication and no reliance on rounded Use%.
   storage={{"state","observed"},{"mount","/data"},{"totalKiB",total},{"usedKiB",used},{"availableKiB",free}};
  }
  if(rows!=1)storage={{"state","unavailable"}};
  QVariantList entries;
  for(const auto &line:profile.value("observation:partitions").toString().split('\n')) {
   const auto f=line.trimmed().split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);
   if(f.size()!=4)continue;
   bool majorOk=false,minorOk=false,sizeOk=false;
   const auto major=f[0].toUInt(&majorOk),minor=f[1].toUInt(&minorOk);
   const auto blocks=f[2].toLongLong(&sizeOk);
   if(!majorOk||!minorOk||!sizeOk||blocks<0||!QRegularExpression("^[A-Za-z0-9_.-]{1,128}$").match(f[3]).hasMatch())continue;
   if(entries.size()>=256)break;
   entries.append(QVariantMap{{"name",f[3]},{"major",major},{"minor",minor},{"blocksKiB",blocks}});
  }
  if(!entries.isEmpty())partitions={{"state","observed"},{"entries",entries},{"scope","kernel-visible block devices; not a firmware flashing map"}};
 }
 result.insert("battery",battery);result.insert("storage",storage);result.insert("partitions",partitions);
 result.insert("usbDriver",QVariantMap{{"state","not-correlated"}});
 result.insert("hardwareValidation","Pending Hardware Validation");
 return result;
}
}
