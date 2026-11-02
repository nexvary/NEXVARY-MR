#include "ChromiumHistoryReader.hpp"
#include "SqliteEvidenceFamily.hpp"
#include "SqliteTableReader.hpp"
#include "SqliteReaderSupport.hpp"
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSet>

namespace mr::forensic {
namespace {
struct Table { quint32 root=0; QStringList columns; SqliteTableRow schemaRow; };
QStringList columns(const QString &sql,const QString &name){
 const QSet<QString> ints=name=="urls"?QSet<QString>{"visit_count","typed_count","last_visit_time","hidden"}:QSet<QString>{"url","visit_time","from_visit","transition","segment_id","is_indexed","visit_duration","incremented_omnibox_typed_score","opener_visit","originator_visit_id","originator_from_visit","originator_opener_visit","is_known_to_sync","consider_for_ntp_most_visited","visited_link_id"};
 const QSet<QString> texts=name=="urls"?QSet<QString>{"url","title"}:QSet<QString>{"external_referrer_url","originator_cache_guid","app_id"};
 const QStringList required=name=="urls"?QStringList{"id","url","title","visit_count","typed_count","last_visit_time","hidden"}:QStringList{"id","url","visit_time","from_visit","transition"};
 return sqliteDeclaredColumns(sql,name,{ints,texts,{},required,false,false});
}
QString stopped(const ReadOnlyEvidenceRequest &r){return r.cancelled&&r.cancelled->load()?"cancelled":r.deadline.hasExpired()?"worker-timeout":QString{};}
}
ForensicAdapterResult readProtectedChromiumHistory(const ReadOnlyEvidenceRequest &r){
 const auto bad=[](const QString &e){return ForensicAdapterResult{{},e};};
 const auto family=inspectSqliteEvidenceFamily(r,{});if(!family.analysis.ok())return family.analysis;
 const auto schema=readProtectedSqliteTable(r,1);if(!schema.ok())return bad(schema.errorCode);
 QMap<QString,Table> tables;
 for(const auto &row:schema.rows){
  const auto stop=stopped(r);if(!stop.isEmpty())return bad(stop);
  if(row.fields.size()!=5||row.serialTypes.size()!=5)return bad("history-schema-record");
  const auto name=row.fields[1].toString().toLower();if(name!="urls"&&name!="visits")continue;
  // An index/trigger with the same target table is irrelevant; an object with the
  // exact target NAME must itself be the expected ordinary table.
  if(!sqliteTextField(row,0)||row.fields[0].toString()!="table"||!sqliteTextField(row,1)||!sqliteTextField(row,2)||row.fields[2].toString().toLower()!=name||!sqliteIntegerField(row,3)||!sqliteTextField(row,4)||tables.contains(name))return bad("history-schema-object");
  bool ok=false;const auto root=row.fields[3].toString().toUInt(&ok);const auto cols=columns(row.fields[4].toString(),name);
  if(!ok||root<2||cols.isEmpty())return bad("history-schema-unsupported");
  tables.insert(name,{root,cols,row});
 }
 if(!tables.contains("urls"))return bad("history-urls-missing");
 if(tables.contains("visits")&&tables["visits"].root==tables["urls"].root)return bad("history-schema-shared-root");
 QMap<QString,SqliteTableRow> urls;
 const auto urlTable=tables.value("urls");const auto urlRows=readProtectedSqliteTable(r,urlTable.root);if(!urlRows.ok())return bad(urlRows.errorCode);
 QSet<quint32> occupied=schema.visitedPages;
 for(const auto page:urlRows.visitedPages)if(occupied.contains(page))return bad("history-aliased-table-page");
 occupied.unite(urlRows.visitedPages);
 auto valid=[&](const SqliteTableRow &row,const Table &table){
  if(row.fields.size()!=table.columns.size()||row.serialTypes.size()!=row.fields.size())return false;
  const auto id=table.columns.indexOf("id");if(row.serialTypes[id]!=0)return false;
  // Preserve exact storage classes; text '123' must never become integer 123.
  for(int i=0;i<table.columns.size();++i){
   if(i==id)continue;
   const auto key=table.columns[i];const bool isText=(table.root==urlTable.root&&(key=="url"||key=="title"))||key=="external_referrer_url"||key=="originator_cache_guid"||key=="app_id";
   if(row.serialTypes[i]!=0&&!(isText?sqliteTextField(row,i):sqliteIntegerField(row,i)))return false;
  }
  return true;
 };
 for(const auto &row:urlRows.rows){if(!valid(row,urlTable))return bad("history-url-record");urls.insert(row.rowid,row);}
 QVariantList observations;int unmatched=0,visitRows=0;
 const auto add=[&](const SqliteTableRow &u,const SqliteTableRow *v,const Table *visits)->bool{
  const auto ui=[&](const QString &key){return u.fields[urlTable.columns.indexOf(key)];};
  const auto urlRefs=sqliteSourceExtents(u,family),schemaRefs=sqliteSourceExtents(urlTable.schemaRow,family);if(urlRefs.isEmpty()||schemaRefs.isEmpty())return false;
  QVariantMap item{{"kind",v?"chromium-visit":"chromium-url-metadata"},{"confidence","source-reported-unverified"},{"urlId",u.rowid},{"url",ui("url")},{"title",ui("title")},{"visitCount",ui("visit_count")},{"typedCount",ui("typed_count")},{"lastVisit",sqliteChromiumTime(ui("last_visit_time"))},{"urlSourceExtents",urlRefs},{"urlSchemaExtents",schemaRefs}};
  if(v){
   const auto vi=[&](const QString &key){return v->fields[visits->columns.indexOf(key)];};
   const auto refs=sqliteSourceExtents(*v,family),sr=sqliteSourceExtents(visits->schemaRow,family);if(refs.isEmpty()||sr.isEmpty())return false;
   item.insert("visitId",v->rowid);item.insert("visitTime",sqliteChromiumTime(vi("visit_time")));item.insert("fromVisit",vi("from_visit"));item.insert("transition",vi("transition"));item.insert("visitSourceExtents",refs);item.insert("visitSchemaExtents",sr);
  }else item.insert("limitation","urls-only metadata; not a visit event");
  observations.append(item);return true;
 };
 if(tables.contains("visits")){
  const auto table=tables.value("visits");const auto visits=readProtectedSqliteTable(r,table.root);if(!visits.ok())return bad(visits.errorCode);
  visitRows=int(visits.rows.size());
  for(const auto page:visits.visitedPages)if(occupied.contains(page))return bad("history-aliased-table-page");
  for(const auto &v:visits.rows){
   const auto stop=stopped(r);if(!stop.isEmpty())return bad(stop);
   if(!valid(v,table)||!sqliteIntegerField(v,table.columns.indexOf("url"))||!sqliteIntegerField(v,table.columns.indexOf("visit_time")))return bad("history-visit-record");
   const auto id=v.fields[table.columns.indexOf("url")].toString();if(!urls.contains(id)){++unmatched;continue;}
   if(observations.size()>=128)return bad("history-result-limit");
   if(!add(urls[id],&v,&table))return bad("history-source-reference");
  }
 }else for(const auto &u:urlRows.rows){
  const auto stop=stopped(r);if(!stop.isEmpty())return bad(stop);
  if(observations.size()>=128)return bad("history-result-limit");
  if(!add(u,nullptr,nullptr))return bad("history-source-reference");
 }
 const auto stop=stopped(r);if(!stop.isEmpty())return bad(stop);
 QVariantMap report{{"schema","nexvary-mr-chromium-history-observation/1"},{"caseId",r.caseId},{"evidenceId",r.evidenceId},{"sourceSha256",r.sourceSha256},{"sqliteFamily",family.analysis.report.value("sqliteFamily")},{"mode",tables.contains("visits")?"visits-joined-to-urls":"urls-only-metadata"},{"unmatchedVisitCount",unmatched},{"observations",observations},{"limitations",QStringList{"Source-reported timestamps/URLs do not establish operator identity, local browsing, authenticity or common acquisition.","Bounded internal extractor; no case retention, SQL execution, live files, Downloads, deleted records or complete database-integrity claim."}}};
 QVariantMap descriptors;
 for(auto it=tables.cbegin();it!=tables.cend();++it){
  QVariantList image,raw;
  for(const auto &v:sqliteSourceExtents(it->schemaRow,family)){auto e=v.toMap();image.append(QVariantMap{{"imageByteOffset",e.take("imageByteOffset")},{"lengthBytes",e.value("lengthBytes")}});raw.append(e);}
  const auto digest=QString::fromLatin1(QCryptographicHash::hash(it->schemaRow.fields[4].toString().toUtf8(),QCryptographicHash::Sha256).toHex());
  descriptors.insert(it.key(),QVariantMap{{"rootPage",it->root},{"columns",it->columns},{"declarationSha256",digest},{"schemaRecord",QVariantMap{{"rowid",it->schemaRow.rowid},{"imageExtents",image},{"rawExtents",raw}}}});
 }
 report.insert("historySchema",descriptors);report.insert("urlRows",urlRows.rows.size());report.insert("visitRows",visitRows);
 if(QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact).size()>64*1024)return bad("history-output-limit");
 return {report,{}};
}
}
