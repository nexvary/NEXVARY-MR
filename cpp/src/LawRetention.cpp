#include "LawRetention.hpp"
#include <QJsonDocument>
#include <QSet>
#include <QUuid>
namespace {
QDateTime date(const QVariant &v){const auto s=v.toString();return s.endsWith('Z')?QDateTime::fromString(s,Qt::ISODateWithMs):QDateTime();}
bool id(const QVariant &v){const auto s=v.toString();return !QUuid(s).isNull()&&QUuid(s).toString(QUuid::WithoutBraces)==s;}
}
namespace nexvary::law {
bool validRetention(const QVariantMap &r) {
 if(!r.contains("retention"))return true; // v1 case migration: absent means no policy, never disposal eligible.
 const auto g=r.value("retention").toMap();
 if(g.size()!=7||(g.value("schema").metaType().id()!=QMetaType::LongLong&&g.value("schema").metaType().id()!=QMetaType::Int&&g.value("schema").metaType().id()!=QMetaType::Double))return false;
 if(g.size()!=7||g.value("schema").toDouble()!=1)return false;
 QSet<QString> evidence,policies,unique;
 for(const auto &v:r.value("evidence").toList())evidence.insert(v.toMap().value("id").toString());
 for(const auto &key:{"policies","assignments","holds","archives","disposals","events"}){
  if(g.value(key).metaType().id()!=QMetaType::QVariantList||g.value(key).toList().size()>200)return false;
 }
 for(const auto &v:g.value("policies").toList()){
  const auto m=v.toMap();const auto days=m.value("days").toInt(),archive=m.value("archiveAfterDays").toInt();
  if(!m.contains("days")||!m.contains("archiveAfterDays")||m.value("days").metaType().id()==QMetaType::Bool||m.value("days").metaType().id()==QMetaType::QString||m.value("archiveAfterDays").metaType().id()==QMetaType::Bool||m.value("archiveAfterDays").metaType().id()==QMetaType::QString||m.value("days").toDouble()!=days||m.value("archiveAfterDays").toDouble()!=archive)return false;
  if(!id(m.value("id"))||policies.contains(m.value("id").toString())||days<1||days>36500||archive<0||archive>days||m.value("label").toString().trimmed().isEmpty()||m.value("authorityReference").toString().trimmed().isEmpty()||!date(m.value("atUtc")).isValid())return false;
  policies.insert(m.value("id").toString());
 }
 for(const auto &key:{"assignments","holds","archives","disposals"})for(const auto &v:g.value(key).toList()){
  const auto m=v.toMap();if(!evidence.contains(m.value("evidenceId").toString())||!date(m.value("atUtc")).isValid())return false;
  if(QString::fromLatin1(key)!="holds"&&!policies.contains(m.value("policyId").toString()))return false;
  if(QString::fromLatin1(key)=="assignments")continue;
  if(!id(m.value("id"))||unique.contains(m.value("id").toString()))return false;
  unique.insert(m.value("id").toString());
  if(QString::fromLatin1(key)=="holds"&&(!QStringList{"legal-hold","preservation-exception"}.contains(m.value("kind").toString())||m.value("reason").toString().trimmed().isEmpty()||(!m.value("releasedAtUtc").toString().isEmpty()&&!date(m.value("releasedAtUtc")).isValid())))return false;
  if(QString::fromLatin1(key)=="archives"){
   QVariantMap source;for(const auto &e:r.value("evidence").toList())if(e.toMap().value("id")==m.value("evidenceId"))source=e.toMap();
   if(m.value("sha256")!=source.value("sha256")||m.value("size").toLongLong()!=source.value("size").toLongLong()||m.value("deviceId")!=source.value("deviceId")||m.value("storage")!="protected-in-place")return false;
  }
  if(QString::fromLatin1(key)=="disposals"){QVariantMap source;for(const auto &e:r.value("evidence").toList())if(e.toMap().value("id")==m.value("evidenceId"))source=e.toMap();if(m.value("sourceSha256")!=source.value("sha256"))return false;}
  if(QString::fromLatin1(key)=="disposals"&&(m.value("actor").toString().isEmpty()||m.value("reason").toString().trimmed().isEmpty()||m.value("authorityReference").toString().trimmed().isEmpty()||
   (m.value("status")!="requested"&&(m.value("reviewedBy").toString().isEmpty()||m.value("reviewedBy")==m.value("actor")||!date(m.value("reviewedAtUtc")).isValid()||m.value("reviewReason").toString().trimmed().isEmpty()||m.value("reviewAuthorityReference").toString().trimmed().isEmpty()))))return false;
  if(QString::fromLatin1(key)=="disposals"&&!QStringList{"requested","approved","rejected"}.contains(m.value("status").toString()))return false;
 }
 return QJsonDocument::fromVariant(g).toJson(QJsonDocument::Compact).size()<=512*1024;
}
QVariantMap assessRetention(const QVariantMap &r,const QString &evidenceId,const QDateTime &asOf) {
 QVariantMap evidence,policy;const auto g=r.value("retention").toMap();QString policyId;
 for(const auto &v:r.value("evidence").toList())if(v.toMap().value("id")==evidenceId)evidence=v.toMap();
 for(const auto &v:g.value("assignments").toList())if(v.toMap().value("evidenceId")==evidenceId)policyId=v.toMap().value("policyId").toString();
 for(const auto &v:g.value("policies").toList())if(v.toMap().value("id")==policyId)policy=v.toMap();
 bool held=false;for(const auto &v:g.value("holds").toList()){const auto h=v.toMap();if(h.value("evidenceId")==evidenceId&&h.value("releasedAtUtc").toString().isEmpty())held=true;}
 const auto registered=date(evidence.value("registeredAtUtc"));
 const bool valid=validRetention(r)&&!evidence.isEmpty()&&!policy.isEmpty()&&registered.isValid()&&asOf.isValid()&&asOf.offsetFromUtc()==0&&asOf>=registered;
 const auto expiry=valid?registered.addDays(policy.value("days").toInt()):QDateTime();
 const bool expired=valid&&asOf>=expiry;
 return {{"evidenceId",evidenceId},{"policyId",policyId},{"policyAssigned",valid},{"held",held},{"expired",expired},
  {"expiryUtc",expiry.toString(Qt::ISODateWithMs)},{"archiveDue",valid&&asOf>=registered.addDays(policy.value("archiveAfterDays").toInt())},
  {"disposalEligible",expired&&!held&&(r.value("status")=="closed"||r.value("status")=="archived")},{"automaticDeletion",false},
  {"assessmentAtUtc",asOf.toUTC().toString(Qt::ISODateWithMs)},{"clockTrust","local OS clock; no trusted external time"}};
}
}
