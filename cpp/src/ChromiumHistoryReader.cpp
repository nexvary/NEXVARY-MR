#include "ChromiumHistoryReader.hpp"
#include "SqliteEvidenceFamily.hpp"
#include "SqliteTableReader.hpp"
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSet>

namespace mr::forensic {
namespace {
struct Table { quint32 root=0; QStringList columns; SqliteTableRow schemaRow; };
bool integerType(quint64 t){return (t>=1&&t<=6)||t==8||t==9;}
bool textType(quint64 t){return t>=13&&(t&1);}
bool text(const SqliteTableRow &r,int i){return i>=0&&i<r.serialTypes.size()&&textType(r.serialTypes[i]);}
bool integer(const SqliteTableRow &r,int i,bool nullable=false){return i>=0&&i<r.serialTypes.size()&&(integerType(r.serialTypes[i])||(nullable&&r.serialTypes[i]==0));}
// Deliberately small CREATE grammar: no comments, quoted identifiers, expressions,
// table constraints, generated columns, virtual tables or WITHOUT ROWID.
QStringList columns(const QString &sql,const QString &name){
 if(sql.size()>8192)return {};
 for(const auto c:sql){const auto u=c.unicode();if(u>126||(u<32&&u!=9&&u!=10&&u!=12&&u!=13))return {};}
 static const QRegularExpression create("\\ACREATE\\s+TABLE\\s+(urls|visits)\\s*\\(([^()]*)\\)\\s*\\z",QRegularExpression::CaseInsensitiveOption);
 static const QRegularExpression column("\\A([a-z_][a-z_0-9]*)\\s+(INTEGER|BOOLEAN|TEXT|LONGVARCHAR)((?:\\s+(?:PRIMARY KEY(?: AUTOINCREMENT)?|NOT NULL|DEFAULT (?:-?[0-9]+|FALSE|TRUE)))*)\\z",QRegularExpression::CaseInsensitiveOption);
 const auto m=create.match(sql.trimmed());if(!m.hasMatch()||m.captured(1).toLower()!=name)return {};
 const auto definitions=m.captured(2).split(',');if(definitions.size()>32)return {};
 const QSet<QString> ints=name=="urls"?QSet<QString>{"visit_count","typed_count","last_visit_time","hidden"}:QSet<QString>{"url","visit_time","from_visit","transition","segment_id","is_indexed","visit_duration","incremented_omnibox_typed_score","opener_visit","originator_visit_id","originator_from_visit","originator_opener_visit","is_known_to_sync","consider_for_ntp_most_visited","visited_link_id"};
 const QSet<QString> texts=name=="urls"?QSet<QString>{"url","title"}:QSet<QString>{"external_referrer_url","originator_cache_guid","app_id"};
 QStringList out;
 for(const auto &def:definitions){
  const auto c=column.match(def.simplified());if(!c.hasMatch())return {};
  const auto key=c.captured(1).toLower(),type=c.captured(2).toUpper(),tail=c.captured(3).toUpper();
  if(out.contains(key))return {};
  if(key=="id"){
   if(type!="INTEGER"||(tail!=" PRIMARY KEY"&&tail!=" PRIMARY KEY AUTOINCREMENT"))return {};
  }else{
   if(tail.contains("PRIMARY"))return {};
   if(ints.contains(key)){if(type!="INTEGER"&&type!="BOOLEAN")return {};}
   else if(texts.contains(key)){if(type!="TEXT"&&type!="LONGVARCHAR")return {};}
   else return {};
  }
  out<<key;
 }
 const QStringList required=name=="urls"?QStringList{"id","url","title","visit_count","typed_count","last_visit_time","hidden"}:QStringList{"id","url","visit_time","from_visit","transition"};
 for(const auto &key:required)if(!out.contains(key))return {};
 return out;
}
QVariantList sourceExtents(const SqliteTableRow &row,const SqliteFamilySnapshot &family){
 QVariantList result;const auto ps=family.analysis.report.value("sqliteFamily").toMap().value("pageSize").toLongLong();
 auto extents=row.imageExtents;extents.prepend(row.imageCellExtent);
 for(const auto &value:extents){
  const auto e=value.toMap();const auto off=e.value("imageByteOffset").toLongLong(),len=e.value("lengthBytes").toLongLong();
  const auto page=quint32(off/ps+1);auto origin=family.pageOrigins.value(page);
  if(origin.isEmpty()||len<0||off%ps+len>ps)return {};
  origin.insert("byteOffset",origin.value("byteOffset").toLongLong()+off%ps);origin.insert("lengthBytes",len);origin.insert("imageByteOffset",off);result.append(origin);
 }
 return result;
}
QVariantMap timeValue(const QVariant &value){
 if(value.isNull())return {{"rawMicroseconds1601",QVariant{}},{"status","null"}};
 const auto raw=value.toString();bool ok=false;const auto n=raw.toLongLong(&ok);
 QVariantMap result{{"rawMicroseconds1601",raw},{"status",!ok||n<0?"unsupported-negative":n==0?"unset-zero":"converted"}};
 // Subtract only nonnegative signed values: no overflow or floating point.
 if(ok&&n>0)result.insert("unixMicroseconds",QString::number(n-Q_INT64_C(11644473600000000)));
 return result;
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
  if(!text(row,0)||row.fields[0].toString()!="table"||!text(row,1)||!text(row,2)||row.fields[2].toString().toLower()!=name||!integer(row,3)||!text(row,4)||tables.contains(name))return bad("history-schema-object");
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
   if(row.serialTypes[i]!=0&&!(isText?text(row,i):integer(row,i)))return false;
  }
  return true;
 };
 for(const auto &row:urlRows.rows){if(!valid(row,urlTable))return bad("history-url-record");urls.insert(row.rowid,row);}
 QVariantList observations;int unmatched=0,visitRows=0;
 const auto add=[&](const SqliteTableRow &u,const SqliteTableRow *v,const Table *visits)->bool{
  const auto ui=[&](const QString &key){return u.fields[urlTable.columns.indexOf(key)];};
  const auto urlRefs=sourceExtents(u,family),schemaRefs=sourceExtents(urlTable.schemaRow,family);if(urlRefs.isEmpty()||schemaRefs.isEmpty())return false;
  QVariantMap item{{"kind",v?"chromium-visit":"chromium-url-metadata"},{"confidence","source-reported-unverified"},{"urlId",u.rowid},{"url",ui("url")},{"title",ui("title")},{"visitCount",ui("visit_count")},{"typedCount",ui("typed_count")},{"lastVisit",timeValue(ui("last_visit_time"))},{"urlSourceExtents",urlRefs},{"urlSchemaExtents",schemaRefs}};
  if(v){
   const auto vi=[&](const QString &key){return v->fields[visits->columns.indexOf(key)];};
   const auto refs=sourceExtents(*v,family),sr=sourceExtents(visits->schemaRow,family);if(refs.isEmpty()||sr.isEmpty())return false;
   item.insert("visitId",v->rowid);item.insert("visitTime",timeValue(vi("visit_time")));item.insert("fromVisit",vi("from_visit"));item.insert("transition",vi("transition"));item.insert("visitSourceExtents",refs);item.insert("visitSchemaExtents",sr);
  }else item.insert("limitation","urls-only metadata; not a visit event");
  observations.append(item);return true;
 };
 if(tables.contains("visits")){
  const auto table=tables.value("visits");const auto visits=readProtectedSqliteTable(r,table.root);if(!visits.ok())return bad(visits.errorCode);
  visitRows=int(visits.rows.size());
  for(const auto page:visits.visitedPages)if(occupied.contains(page))return bad("history-aliased-table-page");
  for(const auto &v:visits.rows){
   const auto stop=stopped(r);if(!stop.isEmpty())return bad(stop);
   if(!valid(v,table)||!integer(v,table.columns.indexOf("url"))||!integer(v,table.columns.indexOf("visit_time")))return bad("history-visit-record");
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
  for(const auto &v:sourceExtents(it->schemaRow,family)){auto e=v.toMap();image.append(QVariantMap{{"imageByteOffset",e.take("imageByteOffset")},{"lengthBytes",e.value("lengthBytes")}});raw.append(e);}
  const auto digest=QString::fromLatin1(QCryptographicHash::hash(it->schemaRow.fields[4].toString().toUtf8(),QCryptographicHash::Sha256).toHex());
  descriptors.insert(it.key(),QVariantMap{{"rootPage",it->root},{"columns",it->columns},{"declarationSha256",digest},{"schemaRecord",QVariantMap{{"rowid",it->schemaRow.rowid},{"imageExtents",image},{"rawExtents",raw}}}});
 }
 report.insert("historySchema",descriptors);report.insert("urlRows",urlRows.rows.size());report.insert("visitRows",visitRows);
 if(QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact).size()>64*1024)return bad("history-output-limit");
 return {report,{}};
}
}
