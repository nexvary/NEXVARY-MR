#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QVariantMap>
#include <cmath>

namespace mr::usb {
// Validate one bounded Windows snapshot before using any node for diagnosis.
// VID/PID/MI below are instance-ID declarations, not USB descriptor reads.
inline QVariantMap inventory(const QByteArray &bytes) {
 if(bytes.size()>512*1024)return {};
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
 if(error.error!=QJsonParseError::NoError||!doc.isObject())return {};
 const auto root=doc.object();const auto nodes=root.value("devices").toArray();
 if(root.value("schemaVersion").toDouble()!=1.0||!root.value("devices").isArray()||nodes.size()>256)return {};
 QMap<QString,QVariantMap> entries;
 for(const auto &value:nodes) {
  if(!value.isObject())return {};
  const auto node=value.toObject();const auto id=node.value("instanceId").toString().toUpper();
  if(!id.startsWith("USB\\")||id.size()>512||entries.contains(id))return {};
  QVariantMap safe;
  for(const auto &key:{"instanceId","parentId","name","service","infName","provider","version"}) {
   const auto v=node.value(key);if(!v.isNull()&&!v.isUndefined()&&!v.isString())return {};
   if(v.toString().size()>512)return {};
   for(const auto c:v.toString())if(c.isNull()||c.category()==QChar::Other_Control)return {};
   safe.insert(key,v.toString());
  }
  const auto signedValue=node.value("signed"),problem=node.value("problemCode");
  if(!signedValue.isNull()&&!signedValue.isUndefined()) {
   if(!signedValue.isBool())return {};
   safe.insert("signed",signedValue.toBool());
  }
  if(!problem.isNull()&&!problem.isUndefined()) {
   const double code=problem.toDouble(-1);
   if(!problem.isDouble()||!std::isfinite(code)||code<0||code>65535||std::floor(code)!=code)return {};
   safe.insert("problemCode",static_cast<int>(code));
  }
  entries.insert(id,safe);
 }
 QVariantList list;for(auto it=entries.cbegin();it!=entries.cend();++it)list.append(it.value());
 return {{"schemaVersion",1},{"devices",list},{"signatureInventoryAvailable",root.value("signatureInventoryAvailable").toBool()}};
}
inline QVariantMap correlate(const QByteArray &bytes,const QString &serial) {
 QVariantMap result{{"state","unavailable"},{"serial",serial},{"interfaceBindingVerified",false}};
 if(!QRegularExpression("^[A-Za-z0-9_-]{1,128}$").match(serial).hasMatch()) {
  result.insert("state","unsupported-serial");return result;
 }
 const auto parsed=inventory(bytes);if(parsed.isEmpty())return result;
 QMap<QString,QVariantMap> inventory;
 for(const auto &entry:parsed.value("devices").toList()) {
  const auto node=entry.toMap();inventory.insert(node.value("instanceId").toString().toUpper(),node);
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
 result.insert("signatureInventoryAvailable",parsed.value("signatureInventoryAvailable"));
 result.insert("scope","Exact USB instance serial and parent ancestry; ADB/Fastboot interface binding is not independently verified");
 return result;
}
}
