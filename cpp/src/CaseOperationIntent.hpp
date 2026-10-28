#pragma once
#include <QDateTime>
#include <QRegularExpression>
#include <QUuid>
#include <QVariantMap>
#include <QStringList>

namespace mr::law {
inline bool caseIntent(const QVariantMap &r) {
 QStringList keys{"schema","operationId","requestId","caseId","evidenceId","expectedRevision","sourceSha256","caseBeforeSha256","reportSha256","analysisId","kind","preparedUtc","requester","requesterRole","recoveryState"};
 const bool external=r.value("schema")=="nexvary-case-operation/v2";
 if(external)keys.append("toolSha256");
 keys.sort();
 if(r.keys()!=keys || (!external && r.value("schema")!="nexvary-case-operation/v1"))return false;
 for(const auto &key:{"operationId","requestId","caseId","evidenceId","analysisId","requester"})
  if(r.value(key).metaType().id()!=QMetaType::QString || QUuid(r.value(key).toString()).isNull() || QUuid(r.value(key).toString()).toString(QUuid::WithoutBraces)!=r.value(key).toString())return false;
 QStringList digests{"sourceSha256","caseBeforeSha256"};
 if(external)digests.append("toolSha256");
 const auto state=r.value("recoveryState").toString();
 if(!external || state=="candidate" || state=="completed" || !r.value("reportSha256").toString().isEmpty())digests.append("reportSha256");
 else if(r.value("reportSha256").metaType().id()!=QMetaType::QString)return false;
 if(external && state=="prepared" && !r.value("reportSha256").toString().isEmpty())return false;
 for(const auto &key:digests)
  if(r.value(key).metaType().id()!=QMetaType::QString || !QRegularExpression("^[0-9a-f]{64}$").match(r.value(key).toString()).hasMatch())return false;
 const auto rev=r.value("expectedRevision");
 if(rev.metaType().id()==QMetaType::Bool || rev.metaType().id()==QMetaType::QString || rev.toDouble()!=rev.toLongLong() || rev.toLongLong()<1 || rev.toLongLong()>=10000)return false;
 const auto time=r.value("preparedUtc").toString();
 return time.endsWith('Z') && QDateTime::fromString(time,Qt::ISODateWithMs).isValid() &&
  (external?r.value("kind")=="partition-analysis":QStringList{"classic-pcap","jpeg-exif","workbench-export-manifest","chromium-bookmarks","dc-image-partitions","browser-sqlite-family"}.contains(r.value("kind").toString())) &&
  QStringList{"reviewer","investigator"}.contains(r.value("requesterRole").toString()) &&
  (external?QStringList{"prepared","candidate","completed","failed","unknown"}:QStringList{"prepared","completed","failed","unknown"}).contains(state);
}
inline bool caseIntentEvent(const QVariantMap &r,const QVariantMap &old,const QString &actor,const QString &role,const QString &result) {
 if(!caseIntent(r) || result!=r.value("recoveryState"))return false;
 if(result=="prepared")return old.isEmpty() && actor==r.value("requester") && role==r.value("requesterRole");
 if(!caseIntent(old) || !actor.isEmpty() || role!="system-observation")return false;
 const bool external=r.value("schema")=="nexvary-case-operation/v2";
 const auto previous=old.value("recoveryState").toString();
 if(previous!="prepared" && !(external && previous=="candidate"))return false;
 if(external && result=="completed" && previous!="candidate")return false;
 auto a=r,b=old;a.remove("recoveryState");b.remove("recoveryState");
 if(external && result=="candidate"){
  if(previous!="prepared")return false;
  a.remove("reportSha256");b.remove("reportSha256");
 }
 return a==b;
}
// A textual case reference alone grants nothing. The adapter separately verifies
// ownership/membership; the signed history also binds the original reviewed start.
inline bool caseIntentAuthority(const QVariantMap &r,const QVariantMap &operation,const QVariantMap &request,const QString &closure) {
 if(r.value("schema")!="nexvary-case-operation/v2" || (r.value("recoveryState")!="prepared" && r.value("recoveryState")!="candidate"))return true;
 return closure.isEmpty() && request.value("status")=="active" && operation.value("status")=="started" &&
  operation.value("kind")=="partition-analysis" && operation.value("capability")=="external-analysis" &&
  operation.value("operationId")==r.value("operationId") && operation.value("requestId")==r.value("requestId") && request.value("id")==r.value("requestId") &&
  operation.value("requester")==r.value("requester") && operation.value("requesterRole")==r.value("requesterRole") &&
  operation.value("caseRef")==r.value("caseId") && request.value("caseRef")==r.value("caseId") &&
  QDateTime::fromString(r.value("preparedUtc").toString(),Qt::ISODateWithMs)>=QDateTime::fromString(operation.value("startedUtc").toString(),Qt::ISODateWithMs);
}
inline bool caseIntentReceipt(const QVariantMap &receipt,const QVariantMap &intent){
 if(intent.value("schema")!="nexvary-case-operation/v2" || receipt.value("status")!="completed")return true;
 return intent.value("recoveryState")=="completed" && receipt.value("sourceSha256")==intent.value("sourceSha256") && receipt.value("outputSha256")==intent.value("reportSha256");
}

}
