#include "ChromiumDownloads.hpp"
#include "ChromiumDownloadsReader.hpp"
#include "ChromiumHistory.hpp"
#include <QRegularExpression>

namespace mr::forensic {
namespace {
QVariantMap parameters(){return {{"parserVersion","nexvary-mr-chromium-downloads/1"},{"maxRows",128},{"maxChainRows",512},{"maxChainLength",64},{"pathPolicy","source strings only; never resolved or opened"},{"chainPolicy","optional table; zero-based contiguous unique indices; orphan refused"},{"timestampPolicy","raw Chromium microseconds; UTC display floors to milliseconds"},{"order","signed download rowid ascending"}};}
QStringList warnings(){return {"Recorded download metadata does not establish file presence, content, completion, safety or operator identity. Paths are strings and are never opened.","Declared file hashes are source-reported and are not verified against downloaded content. No decryption, live profiles or recovery.","Common acquisition/authenticity of protected main/WAL/opaque-SHM members is not independently established; a missing WAL cannot be ruled out when not supplied. Exact timestamp microseconds are retained; UTC display floors to milliseconds."};}
bool integer(const QVariant &v){bool ok=false;const auto n=v.toString().toLongLong(&ok);return v.metaType().id()==QMetaType::QString&&ok&&QString::number(n)==v.toString();}
bool count(const QVariant &v,qint64 low,qint64 high){const auto t=v.metaType().id();if(t!=QMetaType::Int&&t!=QMetaType::UInt&&t!=QMetaType::LongLong&&t!=QMetaType::ULongLong&&t!=QMetaType::Double)return false;bool ok=false;const auto n=v.toLongLong(&ok);return ok&&v.toDouble()==double(n)&&n>=low&&n<=high;}
}
ForensicAdapterResult inspectChromiumDownloads(const ReadOnlyEvidenceRequest &r,QVariantMap report){
 const auto parsed=readProtectedChromiumDownloads(r);if(!parsed.ok())return parsed;
 const auto family=inspectSqliteEvidenceFamily(r,{});if(!family.analysis.ok())return family.analysis;
 const auto &x=parsed.report;QVariantMap schema;
 const auto descriptors=x.value("downloadsSchema").toMap();
 for(auto it=descriptors.cbegin();it!=descriptors.cend();++it){const auto d=it.value().toMap();schema.insert(it.key(),QVariantMap{{"rootPage",d.value("rootPage")},{"columns",d.value("columns")},{"declarationSha256",d.value("declarationSha256")},{"schemaRecord",sqliteRecordReference(d.value("schemaRowid").toString(),d.value("schemaSourceExtents").toList())}});}
 QVariantList findings;
 for(const auto &v:x.value("observations").toList()){
  if((r.cancelled&&r.cancelled->load())||r.deadline.hasExpired())return {{},"cancelled"};
  const auto o=v.toMap();const auto fields=o.value("fields").toMap();QVariantList chain;
  for(const auto &c:o.value("urlChain").toList()){const auto link=c.toMap();chain.append(QVariantMap{{"chainIndex",link.value("chainIndex")},{"url",link.value("url")},{"record",sqliteRecordReference(link.value("rowid").toString(),link.value("sourceExtents").toList())}});}
  const QVariantMap details{{"fields",fields},{"times",o.value("times")},{"timestampUtc",chromiumHistoryTime(fields.value("start_time").toString())},{"downloadRecord",sqliteRecordReference(o.value("downloadId").toString(),o.value("sourceExtents").toList())},{"urlChain",chain},{"chainStatus",o.value("chainStatus")}};
  findings.append(ForensicFinding{"browser-download","sqlite/downloads/rowid/"+o.value("downloadId").toString(),fields.value("target_path").toString(),"source-reported-unverified",details}.toMap());
 }
 report.insert("analysisParameters",parameters());report.insert("familyParameters",family.analysis.report.value("analysisParameters"));report.insert("sqliteFamily",family.analysis.report.value("sqliteFamily"));report.insert("downloadsSchema",schema);report.insert("downloadRows",x.value("downloadRows"));report.insert("chainRows",x.value("chainRows"));report.insert("findings",findings);report.insert("warnings",warnings());return {report,{}};
}
ForensicAdapterResult validateRetainedChromiumDownloads(const QVariantMap &r,const QMap<QString,QVariantMap> &inventory){
 const auto bad=[] {return ForensicAdapterResult{{},"downloads-retained-provenance"};};
 if(r.value("analysisParameters").toMap()!=parameters()||r.value("warnings").toStringList()!=warnings())return bad();
 const auto summary=r.value("sqliteFamily").toMap();auto family=r;family.insert("analysisParameters",r.value("familyParameters"));family.insert("findings",QVariantList{ForensicFinding{"browser-sqlite-family","sqlite/main+wal/last-commit-frame/"+summary.value("lastCommitFrame").toString(),summary.value("snapshotSha256").toString(),"validated-structure-only",summary}.toMap()});
 if(!validateRetainedSqliteFamily(family,inventory).ok())return bad();
 const auto schema=r.value("downloadsSchema").toMap();const bool hasChain=schema.contains("downloads_url_chains");
 if(!schema.contains("downloads")||schema.size()!=(hasChain?2:1))return bad();
 QMap<qint64,QVariantMap> origins;QSet<qint64> roots;QSet<QString> schemaIds;
 static const QRegularExpression hash("\\A[0-9a-f]{64}\\z"),blob("\\A(?:[0-9a-f]{2}){0,64}\\z");
 for(auto it=schema.cbegin();it!=schema.cend();++it){const auto d=it.value().toMap();const auto root=d.value("rootPage").toLongLong();const auto cols=d.value("columns").toStringList();const auto p=chromiumDownloadsColumnPolicy(it.key());
  if(d.size()!=4||!count(d.value("rootPage"),2,summary.value("snapshotPages").toLongLong())||roots.contains(root)||cols.size()>32||QSet<QString>(cols.cbegin(),cols.cend()).size()!=cols.size()||!hash.match(d.value("declarationSha256").toString()).hasMatch()||!validateSqliteRecordReference(d.value("schemaRecord").toMap(),summary,origins))return bad();
  for(const auto &key:cols)if(key!="id"&&!p.integers.contains(key)&&!p.texts.contains(key)&&!p.blobs.contains(key))return bad();
  for(const auto &key:p.required)if(!cols.contains(key))return bad();
  const auto id=d.value("schemaRecord").toMap().value("rowid").toString();if(schemaIds.contains(id))return bad();schemaIds.insert(id);roots.insert(root);
 }
 const auto rows=r.value("findings").toList();if(!count(r.value("downloadRows"),0,128)||r.value("downloadRows").toInt()!=rows.size()||!count(r.value("chainRows"),0,512))return bad();
 const auto columns=schema.value("downloads").toMap().value("columns").toStringList();const auto p=chromiumDownloadsColumnPolicy("downloads");
 qint64 previous=0;bool hasPrevious=false;int links=0;QSet<QString> chainIds;
 for(const auto &v:rows){const auto f=v.toMap(),d=f.value("details").toMap(),fields=d.value("fields").toMap(),rec=d.value("downloadRecord").toMap();
  if(f.size()!=5||d.keys()!=QStringList{"chainStatus","downloadRecord","fields","times","timestampUtc","urlChain"}||d.value("timestampUtc").metaType().id()!=QMetaType::QString||d.value("chainStatus").metaType().id()!=QMetaType::QString||d.value("urlChain").metaType().id()!=QMetaType::QVariantList||f.value("kind")!="browser-download"||f.value("confidence")!="source-reported-unverified"||!validateSqliteRecordReference(rec,summary,origins)||fields.value("id")!=rec.value("rowid")||f.value("reference")!="sqlite/downloads/rowid/"+rec.value("rowid").toString()||fields.size()!=columns.size())return bad();
  const auto id=rec.value("rowid").toString().toLongLong();if(hasPrevious&&id<=previous)return bad();hasPrevious=true;previous=id;
  QVariantMap times;
  for(const auto &col:columns){const auto key=col=="hash"?QString("declaredHashHex"):col;if(!fields.contains(key))return bad();const auto value=fields.value(key);
   if(value.isNull()){if(p.required.contains(col))return bad();}
   else if(col=="id"||p.integers.contains(col)){if(!integer(value))return bad();}
   else if(p.texts.contains(col)){if(value.metaType().id()!=QMetaType::QString||value.toString().size()>2048)return bad();}
   else if(col=="hash"){if(value.metaType().id()!=QMetaType::QString||!blob.match(value.toString()).hasMatch())return bad();}
   else return bad();
   if(col=="start_time"||col=="end_time"||col=="last_access_time")times.insert(col,sqliteChromiumTime(value));
  }
  if(d.value("times").toMap()!=times||d.value("timestampUtc").toString()!=chromiumHistoryTime(fields.value("start_time").toString())||f.value("value").metaType().id()!=QMetaType::QString||f.value("value")!=fields.value("target_path"))return bad();
  const auto chain=d.value("urlChain").toList();if(chain.size()>64||(!hasChain&&!chain.isEmpty()))return bad();int index=0;
  for(const auto &c:chain){const auto link=c.toMap(),record=link.value("record").toMap();const auto rowid=record.value("rowid").toString();if(link.size()!=3||!integer(link.value("chainIndex"))||link.value("chainIndex").toString()!=QString::number(index++)||link.value("url").metaType().id()!=QMetaType::QString||link.value("url").toString().isEmpty()||link.value("url").toString().size()>2048||chainIds.contains(rowid)||!validateSqliteRecordReference(record,summary,origins))return bad();chainIds.insert(rowid);++links;}
  if(d.value("chainStatus")!=(!hasChain?"table-not-supplied":chain.isEmpty()?"no-rows-recorded":"contiguous-recorded-chain"))return bad();
 }
 if(links!=r.value("chainRows").toInt())return bad();
 return {r,{}};
}
}
