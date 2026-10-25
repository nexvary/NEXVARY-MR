#include "LawCaseStore.hpp"
#include "LawRetention.hpp"
#include <QDateTime>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QUuid>
namespace {
QString now(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString uuid(){return QUuid::createUuid().toString(QUuid::WithoutBraces);}
bool text(const QVariant &v,int max=2048){if(v.metaType().id()!=QMetaType::QString)return false;const auto s=v.toString();if(s.trimmed().isEmpty()||s.toUtf8().size()>max)return false;for(const auto c:s)if(c.unicode()<32||c.unicode()==127||(c.unicode()>=0x202a&&c.unicode()<=0x202e)||(c.unicode()>=0x2066&&c.unicode()<=0x2069))return false;return true;}
}
bool LawCaseStore::retentionOperation(const QString &id,qint64 revision,const QString &operation,const QVariantMap &f,const QString &actor) {
 const QMap<QString,QStringList> fields{
  {"policy",{"label","days","archiveAfterDays","authorityReference"}},{"assign",{"evidenceId","policyId"}},
  {"hold",{"evidenceId","kind","reason","authorityReference"}},{"release",{"holdId","reason","authorityReference"}},
  {"archive",{"evidenceId"}},{"request-disposal",{"evidenceId","reason","authorityReference"}},
  {"review-disposal",{"requestId","decision","reason","authorityReference"}}};
 if(!fields.contains(operation)||f.size()!=fields.value(operation).size())return fail("Invalid retention operation or fields.");
 for(const auto &key:fields.value(operation))if(!f.contains(key)||(key!="days"&&key!="archiveAfterDays"&&!text(f.value(key))))return fail("Invalid bounded retention field.");
 return mutate(id,revision,"retention:"+operation,actor,[&](QVariantMap &r){
  auto g=r.value("retention").toMap();if(g.isEmpty())g={{"schema",1},{"policies",QVariantList()},{"assignments",QVariantList()},{"holds",QVariantList()},{"archives",QVariantList()},{"disposals",QVariantList()},{"events",QVariantList()}};
  for(const auto &key:{"policies","assignments","holds","archives","disposals","events"})if(g.value(key).toList().size()>=200)return fail("Retention history limit reached.");
  QVariantMap item;for(const auto &v:r.value("evidence").toList())if(v.toMap().value("id")==f.value("evidenceId"))item=v.toMap();
  const auto assessment=nexvary::law::assessRetention(r,f.value("evidenceId").toString(),QDateTime::currentDateTimeUtc());
  const auto append=[&](const QString &key,QVariantMap entry){auto rows=g.value(key).toList();entry.insert("atUtc",now());entry.insert("actor",actor);rows.append(entry);g.insert(key,rows);};
  if(operation=="policy"){
   for(const auto &key:{"days","archiveAfterDays"}){bool ok=false;const auto n=f.value(key).toLongLong(&ok);if(!ok||f.value(key).toDouble()!=static_cast<double>(n)||f.value(key).metaType().id()==QMetaType::QString||f.value(key).metaType().id()==QMetaType::Bool)return fail("Retention days must be integers.");}
   const auto days=f.value("days").toInt(),archive=f.value("archiveAfterDays").toInt();
   if(days<1||days>36500||archive<0||archive>days)return fail("Retention must be 1..36500 days; archive rule must be within retention.");
   auto policy=f;policy.insert("id",uuid());append("policies",policy);
  }else if(operation=="assign"){
   bool exists=false;for(const auto &v:g.value("policies").toList())if(v.toMap().value("id")==f.value("policyId"))exists=true;
   if(item.isEmpty()||!exists)return fail("Assign an existing policy to evidence in this case.");
   append("assignments",f);
  }else if(operation=="hold"){
   if(item.isEmpty()||!QStringList{"legal-hold","preservation-exception"}.contains(f.value("kind").toString()))return fail("Invalid evidence or preservation kind.");
   auto hold=f;hold.insert("id",uuid());append("holds",hold);
  }else if(operation=="release"){
   auto rows=g.value("holds").toList();bool found=false;for(auto &v:rows){auto h=v.toMap();if(h.value("id")==f.value("holdId")){if(!h.value("releasedAtUtc").toString().isEmpty())return fail("Preservation restriction already released.");h.insert("releasedAtUtc",now());h.insert("releasedBy",actor);h.insert("releaseReason",f.value("reason"));h.insert("releaseAuthorityReference",f.value("authorityReference"));v=h;found=true;}}
   if(!found)return fail("Preservation restriction not found.");
   g.insert("holds",rows);
  }else if(operation=="archive"){
   if(!assessment.value("archiveDue").toBool())return fail("No assigned archive rule is due.");
   const auto bytes=evidenceBytes(id,item);if(bytes.isEmpty())return false;
   append("archives",{{"id",uuid()},{"evidenceId",f.value("evidenceId")},{"deviceId",item.value("deviceId")},{"policyId",assessment.value("policyId")},{"sha256",item.value("sha256")},{"size",item.value("size")},{"storage","protected-in-place"}});
  }else if(operation=="request-disposal"){
   if(!assessment.value("disposalEligible").toBool())return fail("Disposal requires expired retention, closed case and no active preservation restriction.");
   for(const auto &v:g.value("disposals").toList())if(v.toMap().value("evidenceId")==f.value("evidenceId")&&v.toMap().value("status")!="rejected")return fail("An unresolved or approved disposal request already exists.");
   if(evidenceBytes(id,item).isEmpty())return false;
   auto request=f;request.insert("id",uuid());request.insert("policyId",assessment.value("policyId"));request.insert("sourceSha256",item.value("sha256"));request.insert("status","requested");append("disposals",request);
  }else if(operation=="review-disposal"){
   if(!QStringList{"approved","rejected"}.contains(f.value("decision").toString()))return fail("Choose approved or rejected.");
   auto rows=g.value("disposals").toList();bool found=false;for(auto &v:rows){auto request=v.toMap();if(request.value("id")!=f.value("requestId"))continue;
    if(request.value("status")!="requested"||request.value("actor")==actor)return fail("Review requires a different authenticated account and an unreviewed request.");
    const auto a=nexvary::law::assessRetention(r,request.value("evidenceId").toString(),QDateTime::currentDateTimeUtc());
    if(f.value("decision")=="approved"&&(!a.value("disposalEligible").toBool()||a.value("policyId")!=request.value("policyId")||readEvidence(id,request.value("evidenceId").toString()).isEmpty()))return fail("Preservation, policy or integrity changed; disposal approval blocked.");
    request.insert("status",f.value("decision"));request.insert("reviewedBy",actor);request.insert("reviewedAtUtc",now());request.insert("reviewReason",f.value("reason"));request.insert("reviewAuthorityReference",f.value("authorityReference"));v=request;found=true;
   }
   if(!found)return fail("Disposal request not found.");
   g.insert("disposals",rows);
  }
  // Retain the reviewed inputs in protected state. The linked audit binds all of them.
  append("events",{{"operation",operation},{"fields",f}});r.insert("retention",g);
  return nexvary::law::validRetention(r)||fail("Invalid retention state.");
 });
}
QVariantMap LawCaseStore::verifyArchive(const QString &id,const QString &archiveId) {
 const auto r=get(id);if(r.isEmpty())return {};
 for(const auto &v:r.value("retention").toMap().value("archives").toList()){
  const auto a=v.toMap();if(a.value("id")!=archiveId)continue;
  const auto bytes=readEvidence(id,a.value("evidenceId").toString());if(bytes.isEmpty())return {};
  const auto hash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
  if(hash!=a.value("sha256")||bytes.size()!=a.value("size").toLongLong()){fail("Archive inventory integrity mismatch.");return {};}
  return {{"archiveId",archiveId},{"evidenceId",a.value("evidenceId")},{"sha256",hash},{"verified",true},{"verifiedAtUtc",now()}};
 }
 fail("Archive record not found.");return {};
}
