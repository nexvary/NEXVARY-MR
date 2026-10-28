#include "ForensicAdapters.hpp"
#include "DcImagePartitions.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QUuid>
#include <functional>

namespace mr::forensic {
namespace {
const QString permission="LicenseRef-NEXVARY-Owner-Authorized-Integration";
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
bool hash(const QString &s){static const QRegularExpression re("\\A[0-9a-f]{64}\\z");return re.match(s).hasMatch();}
bool uuid(const QString &s){return !QUuid(s).isNull()&&QUuid(s).toString(QUuid::WithoutBraces)==s;}
bool stopped(const ReadOnlyEvidenceRequest &r){return (r.cancelled&&r.cancelled->load())||r.deadline.hasExpired();}
// Reject duplicate JSON member names rather than accepting Qt's last-value-wins
// interpretation. Bound lexical nesting before entering the JSON parser.
bool unambiguousJson(const QByteArray &b){
 QList<QSet<QString>> keys;int start=-1,depth=0;bool string=false,escape=false;
 for(int i=0;i<b.size();++i){const char c=b[i];
  if(string){if(escape){escape=false;continue;}if(c=='\\'){escape=true;continue;}if(c!='"')continue;
   string=false;int j=i+1;while(j<b.size()&&(b[j]==' '||b[j]=='\n'||b[j]=='\r'||b[j]=='\t'))++j;
   if(j<b.size()&&b[j]==':'){
    if(keys.isEmpty())return false;
    const auto doc=QJsonDocument::fromJson("["+b.mid(start,i-start+1)+"]");
    if(!doc.isArray()||doc.array().size()!=1||!doc.array()[0].isString())return false;
    const auto key=doc.array()[0].toString();if(keys.last().contains(key))return false;keys.last().insert(key);
   }
  }else if(c=='"'){string=true;start=i;}
  else if(c=='{'||c=='['){if(++depth>40)return false;if(c=='{')keys.append(QSet<QString>{});}
  else if(c=='}'||c==']'){if(--depth<0)return false;if(c=='}'){if(keys.isEmpty())return false;keys.removeLast();}}
 }
 return !string&&depth==0&&keys.isEmpty();
}
QString chromeTime(const QString &raw){
 static const QRegularExpression number("\\A[0-9]{1,18}\\z");if(!number.match(raw).hasMatch())return {};
 bool ok=false;const auto micros=raw.toLongLong(&ok);if(!ok||micros<=0)return {};
 const auto dt=QDateTime(QDate(1601,1,1),QTime(0,0),Qt::UTC).addMSecs(micros/1000);
 if(!dt.isValid()||dt.date().year()<1||dt.date().year()>9999)return {};
 // Preserve submillisecond precision for the timeline's explicit unresolved gate.
 if(micros%1000==0)return dt.toString(Qt::ISODateWithMs);
 return dt.toString("yyyy-MM-dd'T'HH:mm:ss")+"."+QString::number(micros%1000000).rightJustified(6,'0')+"Z";
}
ForensicAdapterResult parseBookmarks(const ReadOnlyEvidenceRequest &r,QVariantMap report){
 if(!unambiguousJson(r.verifiedBytes))return {{},"ambiguous-or-deep-json"};
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(r.verifiedBytes,&error);
 if(error.error!=QJsonParseError::NoError||!doc.isObject()||!doc.object().value("roots").isObject()||doc.object().value("roots").toObject().isEmpty())return {{},"bookmarks-schema"};
 struct Node {QJsonObject object;QString folder,reference;int depth;};QList<Node> stack;
 const auto roots=doc.object().value("roots").toObject();
 for(auto it=roots.begin();it!=roots.end();++it){if(!it.value().isObject()||it.key().size()>512)return {{},"bookmarks-root"};stack.append({it.value().toObject(),it.key(),"/roots/"+it.key(),0});}
 QVariantList findings;int nodes=0;QStringList warnings{"Bookmark entries are recorded metadata, not proof of a page visit. No live profiles, SQLite, WAL, cookies or password decryption are processed."};
 while(!stack.isEmpty()){
  if(stopped(r))return {{},"cancelled"};
   const auto node=stack.takeLast();
  if(++nodes>4096||node.depth>32)return {{},"bookmark-node-or-depth-limit"};
  const auto o=node.object;const auto type=o.value("type").toString();
  if(type!="url"&&type!="folder")return {{},"bookmark-node-type"};
  if(!o.value("name").isString()||o.value("name").toString().size()>512||node.folder.size()>2048)return {{},"bookmark-text-limit"};
  if(type=="url"){
   if(!o.value("url").isString()||o.value("url").toString().size()>2048||o.contains("children"))return {{},"bookmark-url-schema"};
   if(findings.size()>=128)return {{},"findings-limit"};
   const auto date=o.value("date_added");if(!date.isUndefined()&&(!date.isString()||date.toString().size()>20))return {{},"bookmark-time-schema"};
   ForensicFinding f{"browser-bookmark",node.reference,o.value("url").toString(),"source-reported-unverified",
    {{"title",o.value("name").toString()},{"folder",node.folder},{"dateAddedRaw",date.toString()},{"timestampUtc",chromeTime(date.toString())}}};
   findings.append(f.toMap());
  }else{
   if(o.contains("children")&&!o.value("children").isArray())return {{},"bookmark-children-schema"};
   const auto children=o.value("children").toArray();if(children.size()+stack.size()+nodes>4096)return {{},"bookmark-node-or-depth-limit"};
   const auto folder=node.folder+"/"+o.value("name").toString();
   for(int i=children.size()-1;i>=0;--i){if(!children[i].isObject())return {{},"bookmark-child-schema"};stack.append({children[i].toObject(),folder,node.reference+"/children/"+QString::number(i),node.depth+1});}
  }
 }
 report.insert("findings",findings);report.insert("warnings",warnings);report.insert("nodesExamined",nodes);return {report,{}};
}
ForensicAdapterResult inspectManifest(const ReadOnlyEvidenceRequest &r,QVariantMap report){
 if(!unambiguousJson(r.verifiedBytes))return {{},"ambiguous-or-deep-json"};
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(r.verifiedBytes,&error);QJsonArray entries;
 if(error.error!=QJsonParseError::NoError)return {{},"manifest-json"};
 if(doc.isArray())entries=doc.array();
 else if(doc.isObject()){
  const auto o=doc.object();QString selected;
  for(const auto &key:{"entries","evidence","files"})if(o.contains(key)){if(!selected.isEmpty()||!o.value(key).isArray())return {{},"manifest-entries-schema"};selected=key;entries=o.value(key).toArray();}
  if(selected.isEmpty())return {{},"manifest-entries-schema"};
 }else return {{},"manifest-entries-schema"};
 if(entries.isEmpty()||entries.size()>128)return {{},"manifest-entry-limit"};
 QSet<QString> seen;QVariantList findings;int issues=0;
 for(int i=0;i<entries.size();++i){
  if(stopped(r))return {{},"cancelled"};
   if(!entries[i].isObject())return {{},"manifest-entry-schema"};
   const auto e=entries[i].toObject();
  const auto p=e.value("path").isUndefined()?e.value("source_path"):e.value("path");
  if(!p.isString()||p.toString().isEmpty()||p.toString().size()>2048||!e.value("sha256").isString())return {{},"manifest-entry-schema"};
  const auto path=p.toString();QStringList problems;
  if(seen.contains(path))problems<<"duplicate-path";
  seen.insert(path);
  if(!hash(e.value("sha256").toString().toLower()))problems<<"malformed-sha256";
  if(!problems.isEmpty())++issues;
  ForensicFinding f{"export-manifest-entry","/entries/"+QString::number(i),path,"structural-observation",{{"declaredSha256",e.value("sha256").toString()},{"issues",problems}}};findings.append(f.toMap());
 }
 report.insert("findings",findings);report.insert("entriesWithIssues",issues);report.insert("structurallyValid",issues==0);
 report.insert("payloadVerified",false);report.insert("warnings",QStringList{"Structure inspection only; declared hashes are not verified against payload files. Use MR working-copy verification for actual evidence bytes."});return {report,{}};
}
}
QVariantMap ForensicFinding::toMap() const{return {{"kind",kind},{"reference",reference},{"value",value},{"confidence",confidence},{"details",details}};}
QVariantMap ForensicSourceDescriptor::toMap() const{return {{"adapter",adapter},{"adapterVersion","nexvary-mr-forensic/1"},{"repository",repository},{"upstreamCommit",upstreamCommit},{"sourceModule",sourceModule},{"permissionBasis",permissionBasis},{"maxInputBytes",maxInputBytes},{"maxOutputBytes",maxOutputBytes},{"maxFindings",maxFindings}};}
QStringList forensicAdapters(){return {"workbench-export-manifest","chromium-bookmarks","dc-image-partitions"};}
ForensicSourceDescriptor forensicSource(const QString &adapter){
 if(adapter=="workbench-export-manifest")return {adapter,"nexvary/NEXVARY-Digital-Forensics-Workbench","8b35940b1c879e0ce5f5c13e0d6c89aeab023757","nexvary_lab/pro_tools_v1221.py:export_validate",permission};
 if(adapter=="chromium-bookmarks")return {adapter,"nexvary/NEXVARY-Browser-Forensics-Lab","d7754cd8f051d285406403465b6d7daa3bc4b060","nbf/parsers.py:ChromiumParser.bookmarks",permission};
 if(adapter=="dc-image-partitions")return {adapter,"nexvary/NEXVARY-DC","d8e2baf61b43d044b33a1dd76a444d093c80023c","src/core/Partitions.cpp:imageVolumes",permission};
 return {};
}
ForensicAdapterResult analyzeReadOnlyEvidence(const ReadOnlyEvidenceRequest &r){
 if(!forensicAdapters().contains(r.adapter))return {{},"adapter-not-allowlisted"};
 const auto descriptor=forensicSource(r.adapter);
 if(!uuid(r.caseId)||!uuid(r.evidenceId)||!hash(r.sourceSha256))return {{},"invalid-provenance"};
 if(stopped(r))return {{},r.deadline.hasExpired()?"worker-timeout":"cancelled"};
 if(r.verifiedBytes.isEmpty()||r.verifiedBytes.size()>descriptor.maxInputBytes)return {{},"input-limit"};
 if(sha(r.verifiedBytes)!=r.sourceSha256)return {{},"source-hash-mismatch"};
 QVariantMap report{{"schema","nexvary-forensic-adapter/v1"},{"adapter",r.adapter},{"caseId",r.caseId},{"evidenceId",r.evidenceId},{"sourceSha256",r.sourceSha256},{"sourceBytes",r.verifiedBytes.size()},{"provenance",descriptor.toMap()},{"status","analyzed"}};
 auto result=r.adapter=="chromium-bookmarks"?parseBookmarks(r,report):r.adapter=="workbench-export-manifest"?inspectManifest(r,report):inspectDcPartitions(r,report);
 if(stopped(r))return {{},r.deadline.hasExpired()?"worker-timeout":"cancelled"};
 if(result.ok()&&QJsonDocument::fromVariant(result.report).toJson(QJsonDocument::Compact).size()>descriptor.maxOutputBytes)return {{},"output-limit"};
 return result;
}
ForensicAdapterResult retainedForensicReport(const QVariantMap &run,const QVariantMap &source,const QString &caseId){
 const auto adapter=run.value("parser").toString();const auto bytes=run.value("reportJson").toString().toUtf8();
 if(!forensicAdapters().contains(adapter)||source.isEmpty()||bytes.isEmpty()||bytes.size()>65536||!unambiguousJson(bytes)||sha(bytes)!=run.value("reportSha256")||run.value("sourceSha256")!=source.value("sha256")||run.value("sourceSize")!=source.value("size"))return {{},"forensic-report-integrity"};
 QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);const auto o=doc.object();const auto r=o.toVariantMap();
 if(error.error!=QJsonParseError::NoError||!doc.isObject()||r.value("schema")!="nexvary-forensic-adapter/v1"||r.value("status")!="analyzed"||r.value("adapter")!=adapter||r.value("caseId")!=caseId||r.value("evidenceId")!=source.value("id")||r.value("evidenceId")!=run.value("evidenceId")||r.value("sourceSha256")!=source.value("sha256")||r.value("sourceBytes")!=source.value("size")||r.value("provenance").toMap()!=forensicSource(adapter).toMap()||!o.value("findings").isArray()||o.value("findings").toArray().size()>128)return {{},"forensic-report-provenance"};
 const auto kind=adapter=="chromium-bookmarks"?QString("browser-bookmark"):adapter=="dc-image-partitions"?QString("image-partition"):QString("export-manifest-entry");
 const auto confidence=adapter=="chromium-bookmarks"?QString("source-reported-unverified"):adapter=="dc-image-partitions"?QString("validated-structure-only"):QString("structural-observation");
 for(const auto &entry:o.value("findings").toArray()){
  const auto f=entry.toObject();if(!entry.isObject()||f.value("kind").toString()!=kind||f.value("confidence").toString()!=confidence||!f.value("reference").isString()||f.value("reference").toString().size()>3072||!f.value("value").isString()||f.value("value").toString().size()>2048||!f.value("details").isObject())return {{},"forensic-finding-schema"};
  if(adapter=="chromium-bookmarks"){
   const auto d=f.value("details").toObject();if(!d.value("dateAddedRaw").isString()||d.value("timestampUtc").toString()!=chromeTime(d.value("dateAddedRaw").toString()))return {{},"forensic-time-provenance"};
  }
 }
 return {r,{}};
}

}
