#include "ChromiumDownloadsReader.hpp"
#include "SqliteReaderSupport.hpp"
#include <QCryptographicHash>
#include <QJsonDocument>

namespace mr::forensic {
SqliteColumnPolicy chromiumDownloadsColumnPolicy(const QString &name){
 if(name=="downloads_url_chains")return {{"id","chain_index"},{"url"},{},{"id","chain_index","url"},true,true};
 return {{"start_time","end_time","last_access_time","received_bytes","total_bytes","state","danger_type","interrupt_reason","opened","transient"},
 {"guid","current_path","target_path","referrer","site_url","embedder_download_data","tab_url","tab_referrer_url","http_method","by_ext_id","by_ext_name","by_web_app_id","etag","last_modified","mime_type","original_mime_type"},{"hash"},
 {"id","current_path","target_path","start_time","end_time","received_bytes","total_bytes","state"},false,true};
}
namespace {
struct Table {quint32 root=0;QStringList columns;SqliteTableRow schema;};
QString stopped(const ReadOnlyEvidenceRequest &r){return r.cancelled&&r.cancelled->load()?"cancelled":r.deadline.hasExpired()?"worker-timeout":QString{};}
bool valid(const SqliteTableRow &row,const Table &table,const SqliteColumnPolicy &p){
 if(row.fields.size()!=table.columns.size()||row.serialTypes.size()!=row.fields.size())return false;
 for(int i=0;i<table.columns.size();++i){
  const auto key=table.columns[i];const auto type=row.serialTypes[i];
  if(key=="id"&&!p.downloadChainKey){if(type!=0)return false;continue;}
  if(type==0){if(p.required.contains(key))return false;continue;}
  if(p.integers.contains(key)){if(!sqliteIntegerField(row,i))return false;}
  else if(p.texts.contains(key)){if(!sqliteTextField(row,i)||row.fields[i].toString().size()>2048)return false;}
  else if(p.blobs.contains(key)){if(type<12||(type&1)||row.fields[i].toByteArray().size()>64)return false;}
  else return false;
 }
 return true;
}
QVariantMap descriptor(const Table &t,const SqliteFamilySnapshot &family){
 return {{"rootPage",t.root},{"columns",t.columns},{"declarationSha256",QString::fromLatin1(QCryptographicHash::hash(t.schema.fields[4].toString().toUtf8(),QCryptographicHash::Sha256).toHex())},{"schemaRowid",t.schema.rowid},{"schemaSourceExtents",sqliteSourceExtents(t.schema,family)}};
}
}
ForensicAdapterResult readProtectedChromiumDownloads(const ReadOnlyEvidenceRequest &r){
 const auto reject=[](const QString &code){return ForensicAdapterResult{{},code};};
 const auto family=inspectSqliteEvidenceFamily(r,{});if(!family.analysis.ok())return family.analysis;
 const auto schema=readProtectedSqliteTable(r,1);if(!schema.ok())return reject(schema.errorCode);
 QMap<QString,Table> tables;
 for(const auto &row:schema.rows){
  const auto stop=stopped(r);if(!stop.isEmpty())return reject(stop);
  if(row.fields.size()!=5||row.serialTypes.size()!=5)return reject("downloads-schema-record");
  const auto name=row.fields[1].toString().toLower();if(name!="downloads"&&name!="downloads_url_chains")continue;
  if(!sqliteTextField(row,0)||row.fields[0].toString()!="table"||!sqliteTextField(row,1)||!sqliteTextField(row,2)||row.fields[2].toString().toLower()!=name||!sqliteIntegerField(row,3)||!sqliteTextField(row,4)||tables.contains(name))return reject("downloads-schema-object");
  bool ok=false;const auto root=row.fields[3].toString().toUInt(&ok);const auto cols=sqliteDeclaredColumns(row.fields[4].toString(),name,chromiumDownloadsColumnPolicy(name));
  if(!ok||root<=1||cols.isEmpty())return reject("downloads-schema-unsupported");
  tables.insert(name,{root,cols,row});
 }
 if(!tables.contains("downloads"))return reject("downloads-table-missing");
 const auto downloads=tables.value("downloads"),chains=tables.value("downloads_url_chains");
 if(chains.root&&chains.root==downloads.root)return reject("downloads-schema-shared-root");
 auto occupied=schema.visitedPages;
 const auto data=readProtectedSqliteTable(r,downloads.root);if(!data.ok())return reject(data.errorCode);
 for(const auto page:data.visitedPages)if(occupied.contains(page))return reject("downloads-aliased-table-page");
 occupied.unite(data.visitedPages);
 if(data.rows.size()>128)return reject("downloads-result-limit");
 QMap<QString,SqliteTableRow> byId;
 for(const auto &row:data.rows){if(!valid(row,downloads,chromiumDownloadsColumnPolicy("downloads")))return reject("downloads-record-type-or-limit");byId.insert(row.rowid,row);}
 QMap<QString,QMap<qint64,QVariantMap>> chainRows;
 int chainCount=0;
 if(chains.root){
  const auto rows=readProtectedSqliteTable(r,chains.root);if(!rows.ok())return reject(rows.errorCode);
  for(const auto page:rows.visitedPages)if(occupied.contains(page))return reject("downloads-aliased-table-page");
  if(rows.rows.size()>512)return reject("downloads-chain-result-limit");
  for(const auto &row:rows.rows){
   const auto stop=stopped(r);if(!stop.isEmpty())return reject(stop);
   if(!valid(row,chains,chromiumDownloadsColumnPolicy("downloads_url_chains")))return reject("downloads-chain-record-type-or-limit");
   const auto id=row.fields[chains.columns.indexOf("id")].toString();const auto index=row.fields[chains.columns.indexOf("chain_index")].toString().toLongLong();const auto url=row.fields[chains.columns.indexOf("url")].toString();
   if(!byId.contains(id))return reject("downloads-orphan-chain");
   if(index<0||index>=64||url.isEmpty()||chainRows[id].contains(index))return reject("downloads-chain-index-or-url");
   const auto refs=sqliteSourceExtents(row,family);if(refs.isEmpty())return reject("downloads-source-reference");
   chainRows[id].insert(index,{{"rowid",row.rowid},{"chainIndex",QString::number(index)},{"url",url},{"sourceExtents",refs}});++chainCount;
  }
 }
 QVariantList observations;
 for(const auto &row:data.rows){
  const auto stop=stopped(r);if(!stop.isEmpty())return reject(stop);
  QVariantMap fields,times;
  for(int i=0;i<downloads.columns.size();++i){
   const auto key=downloads.columns[i];const auto value=row.fields[i];
   if(key=="id")fields.insert(key,row.rowid);
   else if(key=="hash")fields.insert("declaredHashHex",value.isNull()?QVariant{}:QVariant(QString::fromLatin1(value.toByteArray().toHex())));
   else fields.insert(key,value);
   if(key=="start_time"||key=="end_time"||key=="last_access_time")times.insert(key,sqliteChromiumTime(value));
  }
  QVariantList chain;const auto ordered=chainRows.value(row.rowid);qint64 expected=0;
  for(auto it=ordered.cbegin();it!=ordered.cend();++it){if(it.key()!=expected++)return reject("downloads-chain-gap");chain.append(it.value());}
  const auto refs=sqliteSourceExtents(row,family);if(refs.isEmpty())return reject("downloads-source-reference");
  observations.append(QVariantMap{{"kind","chromium-download-metadata"},{"confidence","source-reported-unverified"},{"downloadId",row.rowid},{"fields",fields},{"times",times},{"urlChain",chain},{"chainStatus",!chains.root?"table-not-supplied":chain.isEmpty()?"no-rows-recorded":"contiguous-recorded-chain"},{"sourceExtents",refs}});
 }
 QVariantMap descriptors;for(auto it=tables.cbegin();it!=tables.cend();++it)descriptors.insert(it.key(),descriptor(it.value(),family));
 QVariantMap report{{"schema","nexvary-mr-chromium-downloads-observation/1"},{"caseId",r.caseId},{"evidenceId",r.evidenceId},{"sourceSha256",r.sourceSha256},{"sqliteFamily",family.analysis.report.value("sqliteFamily")},{"downloadsSchema",descriptors},{"observations",observations},{"downloadRows",data.rows.size()},{"chainRows",chainCount},{"limitations",QStringList{"Recorded metadata only; paths are strings and are never opened. A recorded download is not proof of file presence, content, completion, safety or operator identity.","The stored hash is a source-reported value, not a verification of downloaded content. No file recovery, live profile, SQL execution, case retention or timeline projection.","WAL consistency and member hashes do not establish common acquisition or authenticity. Unsupported schemas or bounds reject without partial observations."}}};
 const auto stop=stopped(r);if(!stop.isEmpty())return reject(stop);
 if(QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact).size()>64*1024)return reject("downloads-output-limit");
 return {report,{}};
}
}
