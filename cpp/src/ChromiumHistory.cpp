#include "ChromiumHistory.hpp"
#include "ChromiumHistoryReader.hpp"
#include "SqliteEvidenceFamily.hpp"
#include "SqliteReaderSupport.hpp"
#include <QDateTime>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSet>
#include <QTimeZone>

namespace mr::forensic {
namespace {
QVariantMap parameters(){return {{"parserVersion","nexvary-mr-chromium-history/1"},{"schemaPolicy","rowid urls+visits; existing ChromiumHistoryReader declared-column allowlist"},{"maxRowsPerTable",4096},{"maxFindings",128},{"timestampPolicy","Chromium microseconds since 1601 UTC; raw exact; display floor to milliseconds"},{"joinPolicy","visits.url=urls.id; orphan rejected; no urls-only fallback"},{"order","signed visit rowid ascending"}};}
QStringList warnings(){return {"Recorded visits are source-reported metadata, not proof that a person viewed a page. No decryption, deleted recovery or live-profile access.","Common acquisition/authenticity of operator-bound main/WAL/SHM is not independently established; missing WAL cannot be ruled out when not supplied.","Only supported rowid urls+visits declarations and visited URL values are interpreted. No database-wide integrity or browser/profile identity claim. UTC display floors microseconds to milliseconds; exact source microseconds are retained."};}
}
QString chromiumHistoryTime(const QString &raw){
 bool ok=false;const auto n=raw.toLongLong(&ok);if(!ok||n<=0||QString::number(n)!=raw)return {};
 const auto date=QDateTime(QDate(1601,1,1),QTime(0,0),QTimeZone(0)).addMSecs(n/1000);
 return date.isValid()&&date.date().year()>=1601&&date.date().year()<=9999?date.toUTC().toString(Qt::ISODateWithMs):QString{};
}
ForensicAdapterResult inspectChromiumHistory(const ReadOnlyEvidenceRequest &r,QVariantMap report){
 const auto reject=[](const QString &e){return ForensicAdapterResult{{},e};};
 const auto extracted=readProtectedChromiumHistory(r);if(!extracted.ok())return extracted;
 const auto &x=extracted.report;
 if(x.value("mode")!="visits-joined-to-urls")return reject("history-required-tables");
 if(x.value("unmatchedVisitCount").toInt()!=0)return reject("history-orphan-visit");
 const auto family=inspectSqliteEvidenceFamily(r,{});if(!family.analysis.ok())return family.analysis;
 QVariantList findings;
 for(const auto &v:x.value("observations").toList()){
  const auto o=v.toMap();const auto url=o.value("url").toString(),title=o.value("title").toString(),id=o.value("visitId").toString(),raw=o.value("visitTime").toMap().value("rawMicroseconds1601").toString();
  if(o.value("url").isNull()||url.isEmpty()||url.size()>2048||title.size()>2048||o.value("visitCount").isNull()||o.value("typedCount").isNull()||o.value("lastVisit").toMap().value("rawMicroseconds1601").isNull()||o.value("visitCount").toLongLong()<0||o.value("typedCount").toLongLong()<0||o.value("transition").isNull())return reject("history-value-limit");
  QVariantMap d{{"urlId",o.value("urlId")},{"title",title},{"visitTimeRaw",raw},{"timestampUtc",chromiumHistoryTime(raw)},{"lastVisitTimeRaw",o.value("lastVisit").toMap().value("rawMicroseconds1601")},{"visitCount",o.value("visitCount")},{"typedCount",o.value("typedCount")},{"transitionRaw",o.value("transition")},{"fromVisitRaw",o.value("fromVisit")},{"visitRecord",sqliteRecordReference(id,o.value("visitSourceExtents").toList())},{"urlRecord",sqliteRecordReference(o.value("urlId").toString(),o.value("urlSourceExtents").toList())}};
  findings.append(ForensicFinding{"browser-history-visit","sqlite/visits/rowid/"+id,url,"source-reported-unverified",d}.toMap());
 }
 report.insert("analysisParameters",parameters());report.insert("familyParameters",family.analysis.report.value("analysisParameters"));report.insert("sqliteFamily",family.analysis.report.value("sqliteFamily"));report.insert("historySchema",x.value("historySchema"));report.insert("urlRows",x.value("urlRows"));report.insert("visitRows",x.value("visitRows"));report.insert("warnings",warnings());report.insert("findings",findings);
 return {report,{}};
}
ForensicAdapterResult validateRetainedChromiumHistory(const QVariantMap &r,const QMap<QString,QVariantMap> &inventory){
 const auto bad=[] {return ForensicAdapterResult{{},"history-retained-provenance"};};
 if(r.value("analysisParameters").toMap()!=parameters()||r.value("warnings").toStringList()!=warnings())return bad();
 const auto summary=r.value("sqliteFamily").toMap();auto familyReport=r;
 familyReport.insert("analysisParameters",r.value("familyParameters"));
 familyReport.insert("findings",QVariantList{ForensicFinding{"browser-sqlite-family","sqlite/main+wal/last-commit-frame/"+summary.value("lastCommitFrame").toString(),summary.value("snapshotSha256").toString(),"validated-structure-only",summary}.toMap()});
 if(!validateRetainedSqliteFamily(familyReport,inventory).ok())return bad();
 const auto ps=summary.value("pageSize").toLongLong(),imageSize=summary.value("snapshotBytes").toLongLong();QMap<qint64,QVariantMap> origins;
 const auto canonicalInteger=[](const QVariant &v,bool positive=false){bool ok=false;const auto n=v.toString().toLongLong(&ok);return v.metaType().id()==QMetaType::QString&&ok&&QString::number(n)==v.toString()&&(!positive||n>0);};
 const auto record=[&](const QVariantMap &rec){return validateSqliteRecordReference(rec,summary,origins);};
 const auto schema=r.value("historySchema").toMap();if(schema.size()!=2||!schema.contains("urls")||!schema.contains("visits"))return bad();QSet<qint64> roots;
 for(auto it=schema.cbegin();it!=schema.cend();++it){
  const auto t=it.value().toMap();const auto root=t.value("rootPage").toLongLong();const auto cols=t.value("columns").toStringList();
  static const QRegularExpression hex("\\A[0-9a-f]{64}\\z");
  if(t.size()!=4||t.value("rootPage").toDouble()!=double(root)||root<=1||root>imageSize/ps||roots.contains(root)||cols.size()>64||QSet<QString>(cols.cbegin(),cols.cend()).size()!=cols.size()||!cols.contains("id")||!hex.match(t.value("declarationSha256").toString()).hasMatch()||!record(t.value("schemaRecord").toMap()))return bad();
  roots.insert(root);
  const auto required=it.key()=="urls"?QStringList{"url","title","visit_count","typed_count","last_visit_time"}:QStringList{"url","visit_time","from_visit","transition"};
  for(const auto &s:required)if(!cols.contains(s))return bad();
 }
 const auto rows=r.value("findings").toList();const auto urls=r.value("urlRows").toLongLong(),visits=r.value("visitRows").toLongLong();
 if(rows.size()>128||r.value("urlRows").toDouble()!=double(urls)||r.value("visitRows").toDouble()!=double(visits)||urls<0||urls>4096||visits!=rows.size())return bad();
 bool previousSet=false;qint64 previous=0;QMap<QString,QVariantMap> urlsSeen;
 for(const auto &v:rows){
  const auto f=v.toMap(),d=f.value("details").toMap();const auto vr=d.value("visitRecord").toMap(),ur=d.value("urlRecord").toMap();
  if(f.size()!=5||d.size()!=11||f.value("kind")!="browser-history-visit"||f.value("confidence")!="source-reported-unverified"||f.value("reference")!="sqlite/visits/rowid/"+vr.value("rowid").toString()||!record(vr)||!record(ur)||d.value("urlId")!=ur.value("rowid"))return bad();
  const auto id=vr.value("rowid").toString().toLongLong();if(previousSet&&id<=previous)return bad();previousSet=true;previous=id;
  for(const auto &key:QStringList{"urlId","visitTimeRaw","lastVisitTimeRaw","visitCount","typedCount","transitionRaw"})if(!canonicalInteger(d.value(key)))return bad();
  if(!d.value("fromVisitRaw").isNull()&&!canonicalInteger(d.value("fromVisitRaw")))return bad();
  if(d.value("timestampUtc").toString()!=chromiumHistoryTime(d.value("visitTimeRaw").toString())||d.value("visitCount").toLongLong()<0||d.value("typedCount").toLongLong()<0||f.value("value").metaType().id()!=QMetaType::QString||f.value("value").toString().isEmpty()||f.value("value").toString().size()>2048||d.value("title").metaType().id()!=QMetaType::QString||d.value("title").toString().size()>2048)return bad();
  const auto key=d.value("urlId").toString();const QVariantMap u{{"record",ur},{"url",f.value("value")},{"title",d.value("title")},{"visitCount",d.value("visitCount")},{"typedCount",d.value("typedCount")},{"lastVisitTimeRaw",d.value("lastVisitTimeRaw")}};
  if(urlsSeen.contains(key)&&urlsSeen.value(key)!=u)return bad();
  urlsSeen.insert(key,u);
 }
 if(urlsSeen.size()>urls)return bad();
 return {r,{}};
}
}
