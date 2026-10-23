#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QVariantMap>

namespace mr::usb {
inline QVariantMap correlate(const QByteArray &bytes,const QString &serial) {
 QVariantMap result{{"state","unavailable"},{"serial",serial},{"interfaceBindingVerified",false}};
 if(!QRegularExpression("^[A-Za-z0-9_-]{1,128}$").match(serial).hasMatch()) {
  result.insert("state","unsupported-serial");return result;
 }
 if(bytes.size()>512*1024)return result;
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
 if(error.error!=QJsonParseError::NoError||!doc.isObject())return result;
 const auto root=doc.object();const auto nodes=root.value("devices").toArray();
 if(root.value("schemaVersion").toInt()!=1||!root.value("devices").isArray()||nodes.size()>256)return result;
 QMap<QString,QVariantMap> inventory;
 for(const auto &value:nodes) {
  if(!value.isObject())return result;
  const auto node=value.toObject();const auto id=node.value("instanceId").toString().toUpper();
  if(!id.startsWith("USB\\")||id.size()>512||inventory.contains(id))return result;
  QVariantMap safe;
  for(const auto &key:{"instanceId","parentId","name","service","infName","provider","version"}) {
   const auto v=node.value(key);if(!v.isNull()&&!v.isUndefined()&&!v.isString())return result;
   if(v.toString().size()>512)return result;
   safe.insert(key,v.toString());
  }
  if(node.value("signed").isBool())safe.insert("signed",node.value("signed").toBool());
  if(node.value("problemCode").isDouble())safe.insert("problemCode",node.value("problemCode").toInt(-1));
  inventory.insert(id,safe);
 }
 QStringList matches;
 const QRegularExpression physical("^USB\\\\VID_[0-9A-F]{4}&PID_[0-9A-F]{4}\\\\([^\\\\]+)$");
 for(auto it=inventory.cbegin();it!=inventory.cend();++it) {
  const auto match=physical.match(it.key());
  if(match.hasMatch()&&match.captured(1)==serial.toUpper())matches.append(it.key());
 }
 if(matches.size()!=1) {result.insert("state",matches.isEmpty()?"no-match":"ambiguous");return result;}
 const auto target=matches.first();QVariantList associated;
 for(auto it=inventory.cbegin();it!=inventory.cend();++it) {
  auto current=it.key();QSet<QString> seen;
  for(int depth=0;depth<16&&!current.isEmpty()&&!seen.contains(current);++depth) {
   if(current==target){associated.append(it.value());break;}
   seen.insert(current);current=inventory.value(current).value("parentId").toString().toUpper();
  }
 }
 result.insert("state","serial-correlated");result.insert("physicalInstanceId",target);result.insert("nodes",associated);
 result.insert("signatureInventoryAvailable",root.value("signatureInventoryAvailable").toBool());
 result.insert("scope","Exact USB instance serial and parent ancestry; ADB/Fastboot interface binding is not independently verified");
 return result;
}
}
