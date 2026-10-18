#include "ForensicRecord.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <algorithm>
namespace mr::forensic {
namespace {
QString digest(const QJsonObject &entry) {
 auto copy=entry;copy.remove("entryHash");
 return QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(copy).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
}
QString utc(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString cell(const QJsonValue &value){return value.toVariant().toString().toHtmlEscaped();}
}
void appendAudit(QJsonObject &document,const QString &operation,const QString &outcome) {
 auto list=document.value("audit").toArray();
 QJsonObject entry{{"atUtc",utc()},{"operation",operation},{"outcome",outcome.left(2000)},
  {"operator",document.value("operator").toString()},{"sequence",list.size()+1},
  {"previousHash",list.isEmpty()?QString(64,'0'):list.last().toObject().value("entryHash").toString()}};
 entry.insert("entryHash",digest(entry));list.append(entry);document.insert("audit",list);
 document.insert("historyVersion",1);
}
bool validateHistory(const QJsonObject &document,QString &error) {
 const auto events=document.value("audit").toArray();
 if(!document.contains("historyVersion")) {
  for(const auto &event:events)if(event.toObject().contains("entryHash")){error="Missing audit history version.";return false;}
  return true; // Existing v1 containers are upgraded only after their evidence validates.
 }
 if(document.value("historyVersion").toInt()!=1){error="Unsupported audit history version.";return false;}
 QString previous(64,'0');int sequence=0;
 for(const auto &event:events) {
  const auto entry=event.toObject();
  const auto time=QDateTime::fromString(entry.value("atUtc").toString(),Qt::ISODateWithMs);
  if(entry.value("sequence").toInt()!=++sequence||entry.value("previousHash").toString()!=previous||
     entry.value("entryHash").toString()!=digest(entry)||!time.isValid()||time.offsetFromUtc()!=0) {
   error="Audit sequence/hash/UTC validation failed.";return false;
  }
  previous=entry.value("entryHash").toString();
 }
 return true;
}
void upgradeLegacy(QJsonObject &document) {
 if(!document.contains("historyVersion")) {
  const auto original=document.value("audit").toArray();QJsonArray upgraded;QString previous(64,'0');
  for(const auto &value:original) {
   auto entry=value.toObject();entry.insert("legacyUnverified",true);entry.insert("operator",QString());
   entry.insert("sequence",upgraded.size()+1);entry.insert("previousHash",previous);
   entry.insert("entryHash",digest(entry));previous=entry.value("entryHash").toString();upgraded.append(entry);
  }
  document.insert("audit",upgraded);document.insert("historyVersion",1);
  appendAudit(document,"legacy-migration","Historical entries re-chained on opening; original operator and historical integrity were not established.");
 }
 auto evidence=document.value("evidence").toArray();
 for(qsizetype i=0;i<evidence.size();++i){auto entry=evidence[i].toObject();if(entry.value("id").toString().isEmpty())entry.insert("id",QUuid::createUuid().toString(QUuid::WithoutBraces));if(!entry.contains("collectedBy"))entry.insert("collectedBy",QString());evidence[i]=entry;}
 document.insert("evidence",evidence);
}
QVariantList timeline(const QJsonObject &document) {
 QVariantList result;
 for(const auto &value:document.value("audit").toArray()) {
  const auto e=value.toObject();result.append(QVariantMap{{"atUtc",e.value("atUtc").toString()},{"category","case-operation"},
   {"event",e.value("operation").toString()},{"detail",e.value("outcome").toString()},{"operator",e.value("operator").toString()},{"evidenceId",QString()}});
 }
 for(const auto &value:document.value("evidence").toArray()) {
  const auto e=value.toObject();result.append(QVariantMap{{"atUtc",e.value("collectedAtUtc").toString()},{"category","collection-time"},
   {"event",e.value("kind").toString()},{"detail",e.value("name").toString()},{"operator",e.value("collectedBy").toString()},{"evidenceId",e.value("id").toString()}});
 }
 std::stable_sort(result.begin(),result.end(),[](const QVariant &a,const QVariant &b){
  return QDateTime::fromString(a.toMap().value("atUtc").toString(),Qt::ISODateWithMs)<QDateTime::fromString(b.toMap().value("atUtc").toString(),Qt::ISODateWithMs);
 });
 return result;
}
QString htmlReport(const QJsonObject &document,bool arabic) {
 auto label=[arabic](const char *en,const char *ar){return QString::fromUtf8(arabic?ar:en);};
 QString html=QStringLiteral("<!doctype html><html lang=\"%1\" dir=\"%2\"><head><meta charset=\"utf-8\"><title>NEXVARY-MR</title></head><body style=\"font-family:Arial,sans-serif;font-size:10pt\">").arg(arabic?"ar":"en",arabic?"rtl":"ltr");
 html+="<h1>"+label("Forensic case report","تقرير قضية الأدلة الرقمية")+"</h1><p>NEXVARY-MR 0.4.0 / native case engine</p>";
 for(const auto &key:QStringList{"id","title","ownerReference","operator","authorization","createdAtUtc"})html+="<p><b>"+key.toHtmlEscaped()+":</b> "+cell(document.value(key))+"</p>";
 html+="<p>"+utc()+" UTC</p><h2>"+label("Evidence and acquisition hashes","الأدلة وبصمات الجمع")+"</h2>";
 for(const auto &value:document.value("evidence").toArray()) {
  const auto e=value.toObject();html+="<h3>"+cell(e.value("name"))+"</h3>";
  for(const auto &key:QStringList{"id","kind","size","sha256","source","sourceSha256","offset","collectedAtUtc","collectedBy"})html+="<p><b>"+key+":</b> "+cell(e.value(key))+"</p>";
 }
 html+="<h2>"+label("Source analysis records","سجلات تحليل المصادر")+"</h2><pre>"+QString::fromUtf8(QJsonDocument(document.value("sources").toArray()).toJson()).toHtmlEscaped()+"</pre>";
 html+="<h2>"+label("Case activity timeline (not device activity)","الخط الزمني لعمليات القضية (وليس نشاط الهاتف)")+"</h2>";
 for(const auto &value:timeline(document)){const auto e=value.toMap();html+="<p>"+e.value("atUtc").toString().toHtmlEscaped()+" | "+e.value("event").toString().toHtmlEscaped()+" | "+e.value("detail").toString().toHtmlEscaped()+"</p>";}
 html+="<h2>"+label("Audit consistency chain","سلسلة اتساق سجل العمليات")+"</h2><pre>"+QString::fromUtf8(QJsonDocument(document.value("audit").toArray()).toJson()).toHtmlEscaped()+"</pre>";
 html+="<h2>"+label("Limitations","القيود")+"</h2><p>"+cell(document.value("limits"))+"</p><p>"+label("Plaintext export. Local audit hashes are not signatures and cannot independently detect a rewritten or truncated history. No court accreditation or complete device-activity timeline is claimed.","نسخة غير مشفرة. بصمات السجل المحلي ليست توقيعات ولا تكشف بمفردها إعادة كتابة السجل كاملًا أو اقتطاعه. لا يُدّعى اعتماد قضائي أو خط زمني شامل لنشاط الجهاز.")+"</p></body></html>";
 return html;
}
}
