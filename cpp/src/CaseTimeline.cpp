#include "CaseTimeline.hpp"
#include "ForensicAdapters.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QTimeZone>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
namespace nexvary::law {
namespace {
QString digest(const QByteArray &bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
QVariantMap rejected(const QString &code){return {{"errorCode",code}};}
QVariantMap timeValue(const QString &raw,bool shortened,const QString &kind){
 QVariantMap value{{"originalTime",raw},{"utc",QString()},{"offset",QString()},{"timeState","unresolved"},{"confidence","source-reported-unverified"}};
 const auto reason=[&](const QString &text){value.insert("reason",text);return value;};
 if(shortened)return reason("shortened-source-value");
 if(kind=="logcat-threadtime-raw")return reason("missing-year-and-timezone");
 static const QRegularExpression iso(QStringLiteral("\\A([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})(?:\\.([0-9]{1,9}))?(Z|[+-][0-9]{2}:[0-9]{2})\\z"));
 const auto m=iso.match(raw);
 if(!m.hasMatch())return reason("unsupported-format-or-missing-timezone");
 const QDate date(m.captured(1).toInt(),m.captured(2).toInt(),m.captured(3).toInt());
 const auto fraction=m.captured(7),zone=m.captured(8);
 value.insert("offset",zone);
 if(!date.isValid())return reason("invalid-calendar-date");
 if(fraction.size()>3)return reason("unsupported-submillisecond-precision");
 const QTime time(m.captured(4).toInt(),m.captured(5).toInt(),m.captured(6).toInt(),fraction.isEmpty()?0:fraction.leftJustified(3,'0').toInt());
 if(!time.isValid())return reason("invalid-time-or-leap-second");
 if(zone=="-00:00")return reason("unknown-offset");
 int minutes=0;
 if(zone!="Z"){
  const int h=zone.mid(1,2).toInt(),n=zone.mid(4,2).toInt();
  if(h>14||n>59||(h==14&&n!=0))return reason("offset-out-of-range");
  minutes=(h*60+n)*(zone.startsWith('-')?-1:1);
 }
 const auto utc=QDateTime(date,time,QTimeZone(0)).addSecs(-minutes*60).toUTC();
 if(!utc.isValid()||utc.date().year()<1||utc.date().year()>9999)return reason("utc-out-of-range");
 value.insert("utc",utc.toString(Qt::ISODateWithMs));value.insert("timeState","normalized");
 value.insert("reason","explicit-offset-literal-only");return value;
}
}
QVariantMap projectTimeline(const QVariantMap &record,const QString &query,const std::shared_ptr<std::atomic_bool> &cancelled){
 if(query.size()>256)return rejected("query-limit");
 const auto stopped=[&]{return cancelled&&cancelled->load();};
 const auto runs=record.value("analyses").toList(),evidence=record.value("evidence").toList();
 if(record.value("id").toString().isEmpty()||runs.size()>100||evidence.size()>1000)return rejected("snapshot-limit-or-missing-case");
 static const QRegularExpression validHash(QStringLiteral("\\A[0-9a-f]{64}\\z"));
 QMap<QString,QVariantMap> sources;
 for(const auto &v:evidence){const auto e=v.toMap();const auto id=e.value("id").toString();if(id.isEmpty()||sources.contains(id)||!validHash.match(e.value("sha256").toString()).hasMatch())return rejected("invalid-evidence-inventory");sources.insert(id,e);}
 QVariantList events;int candidates=0,normalized=0,unresolved=0,omitted=0,truncatedRuns=0;QSet<QString> seen;
 static const QRegularExpression candidate(QStringLiteral("\\A[0-9]{4}-[0-9]{2}-[0-9]{2}(?:T| |$)"));
 for(const auto &v:runs){
  if(stopped())return rejected("cancelled");
  const auto run=v.toMap();const auto parser=run.value("parser").toString();
  if(mr::forensic::forensicAdapters().contains(parser)){
   const auto id=run.value("id").toString(),eid=run.value("evidenceId").toString();const auto source=sources.value(eid);
   if(id.isEmpty()||seen.contains(id))return rejected("analysis-inventory");
   seen.insert(id);
   const auto checked=mr::forensic::retainedForensicReport(run,source,record.value("id").toString(),sources);if(!checked.ok())return rejected(checked.errorCode);
   if(parser!="chromium-bookmarks"&&parser!="chromium-history")continue;
   const bool history=parser=="chromium-history";
   const auto findings=checked.report.value("findings").toList();
   for(qsizetype i=0;i<findings.size();++i){
    if(stopped())return rejected("cancelled");
    const auto finding=findings[i].toMap(),details=finding.value("details").toMap();const auto timestamp=details.value("timestampUtc").toString();
    const auto raw=details.value(history?"visitTimeRaw":"dateAddedRaw").toString();
    if(raw.isEmpty())continue;
    ++candidates;auto row=timeValue(timestamp.isEmpty()?raw:timestamp,false,"browser-bookmark-date-added");
    row.insert("sourceTimeLiteral",raw);row.insert("eventMeaning",history?"source-recorded-visit; not proof of human viewing":"recorded-bookmark-date-added; not a visit");
    if(history){row.insert("visitRecord",details.value("visitRecord"));row.insert("urlRecord",details.value("urlRecord"));}
    row.insert("id",id+":"+QString::number(i));row.insert("analysisId",id);row.insert("caseId",record.value("id"));row.insert("evidenceId",eid);row.insert("deviceId",source.value("deviceId"));row.insert("sourceName",source.value("name"));row.insert("sourceSha256",source.value("sha256"));row.insert("reportSha256",run.value("reportSha256"));row.insert("reference",finding.value("reference"));row.insert("kind",history?"browser-history-visit":"browser-bookmark-date-added");
    if(!query.isEmpty()&&!QString::fromUtf8(QJsonDocument::fromVariant(row).toJson(QJsonDocument::Compact)).contains(query,Qt::CaseInsensitive))continue;
    if(events.size()>=timelineEventLimit){++omitted;continue;}if(row.value("timeState")=="normalized")++normalized;else ++unresolved;events.append(row);
   }
   continue;
  }
  if(!parser.startsWith("authorized-import/"))continue;
  const auto id=run.value("id").toString(),eid=run.value("evidenceId").toString();const auto source=sources.value(eid);
  const auto bytes=run.value("reportJson").toString().toUtf8();
  if(id.isEmpty()||seen.contains(id)||source.isEmpty()||bytes.size()>64*1024||digest(bytes)!=run.value("reportSha256").toString()||source.value("sha256")!=run.value("sourceSha256"))return rejected("report-integrity");
  seen.insert(id);QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);const auto report=doc.object().toVariantMap();
  if(error.error!=QJsonParseError::NoError||!doc.isObject()||report.value("schema")!="nexvary-authorized-import/v1"||report.value("sourceSha256")!=source.value("sha256"))return rejected("report-schema");
  if(report.value("status")=="rejected")continue;
  if(report.value("status")!="imported")return rejected("report-status");
  const auto findings=report.value("findings").toList();if(findings.size()>128)return rejected("findings-limit");
  if(report.value("findingsTruncated").toBool())++truncatedRuns;
  int index=0;
  for(const auto &f:findings){
   if(stopped())return rejected("cancelled");
   const auto finding=f.toMap();const auto raw=finding.value("value").toString(),kind=finding.value("kind").toString();const int ordinal=index++;
   if(kind!="logcat-threadtime-raw"&&!candidate.match(raw).hasMatch())continue;
   if(kind=="logcat-threadtime-raw"&&(raw.isEmpty()||raw.startsWith("---------")))continue;
   ++candidates;
   auto row=timeValue(raw,finding.value("valueTruncated").toBool(),kind);
   row.insert("id",id+":"+QString::number(ordinal));row.insert("analysisId",id);row.insert("caseId",record.value("id"));row.insert("evidenceId",eid);
   row.insert("deviceId",source.value("deviceId"));row.insert("sourceName",source.value("name"));row.insert("sourceSha256",source.value("sha256"));
   row.insert("reportSha256",run.value("reportSha256"));row.insert("reference",finding.value("reference"));row.insert("kind",kind);
   // Literal search only: no regular expression, token merging or inferred identities.
   if(!query.isEmpty()&&!QString::fromUtf8(QJsonDocument::fromVariant(row).toJson(QJsonDocument::Compact)).contains(query,Qt::CaseInsensitive))continue;
   if(events.size()>=timelineEventLimit){++omitted;continue;}
   if(row.value("timeState")=="normalized")++normalized;else ++unresolved;
   events.append(row);
  }
 }
 std::sort(events.begin(),events.end(),[](const QVariant &a,const QVariant &b){
  const auto x=a.toMap(),y=b.toMap();const bool xn=x.value("timeState")=="normalized",yn=y.value("timeState")=="normalized";
  if(xn!=yn)return xn;
  if(xn&&x.value("utc")!=y.value("utc"))return x.value("utc").toString()<y.value("utc").toString();
  return x.value("id").toString()<y.value("id").toString();
 });
 return {{"schema","nexvary-case-timeline/v1"},{"caseId",record.value("id")},{"caseRevision",record.value("revision")},
  {"synthetic",record.value("synthetic")},{"query",query},{"events",events},{"candidateCount",candidates},{"normalizedCount",normalized},{"unresolvedCount",unresolved},
  {"omittedMatchingCandidates",omitted},{"truncatedImportReports",truncatedRuns},{"eventLimit",timelineEventLimit},
  {"scope","Observed timestamp literals from retained authorized imports and recorded bookmark date-added metadata only. No timestamp authenticity, causal/person relationship, timezone inference, complete device activity or current source-byte verification is established."}};
}
}
