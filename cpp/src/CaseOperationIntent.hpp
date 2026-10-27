#pragma once
#include <QDateTime>
#include <QRegularExpression>
#include <QUuid>
#include <QVariantMap>
#include <QStringList>

namespace mr::law {
inline bool caseIntent(const QVariantMap &r) {
 QStringList keys{"schema","operationId","requestId","caseId","evidenceId","expectedRevision","sourceSha256","caseBeforeSha256","reportSha256","analysisId","kind","preparedUtc","requester","requesterRole","recoveryState"};
 keys.sort();
 if(r.keys()!=keys || r.value("schema")!="nexvary-case-operation/v1")return false;
 for(const auto &key:{"operationId","requestId","caseId","evidenceId","analysisId","requester"})
  if(r.value(key).metaType().id()!=QMetaType::QString || QUuid(r.value(key).toString()).isNull() || QUuid(r.value(key).toString()).toString(QUuid::WithoutBraces)!=r.value(key).toString())return false;
 for(const auto &key:{"sourceSha256","caseBeforeSha256","reportSha256"})
  if(r.value(key).metaType().id()!=QMetaType::QString || !QRegularExpression("^[0-9a-f]{64}$").match(r.value(key).toString()).hasMatch())return false;
 const auto rev=r.value("expectedRevision");
 if(rev.metaType().id()==QMetaType::Bool || rev.metaType().id()==QMetaType::QString || rev.toDouble()!=rev.toLongLong() || rev.toLongLong()<1 || rev.toLongLong()>=10000)return false;
 const auto time=r.value("preparedUtc").toString();
 return time.endsWith('Z') && QDateTime::fromString(time,Qt::ISODateWithMs).isValid() &&
  QStringList{"classic-pcap","jpeg-exif"}.contains(r.value("kind").toString()) &&
  QStringList{"reviewer","investigator"}.contains(r.value("requesterRole").toString()) &&
  QStringList{"prepared","completed","failed","unknown"}.contains(r.value("recoveryState").toString());
}
inline bool caseIntentEvent(const QVariantMap &r,const QVariantMap &old,const QString &actor,const QString &role,const QString &result) {
 if(!caseIntent(r) || result!=r.value("recoveryState"))return false;
 if(result=="prepared")return old.isEmpty() && actor==r.value("requester") && role==r.value("requesterRole");
 if(old.value("recoveryState")!="prepared" || !actor.isEmpty() || role!="system-observation")return false;
 auto a=r,b=old;a.remove("recoveryState");b.remove("recoveryState");return a==b;
}
}
