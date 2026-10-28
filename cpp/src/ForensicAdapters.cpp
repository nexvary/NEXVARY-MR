#include "ForensicAdapters.hpp"
#include "DcImagePartitions.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QTimeZone>
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
 const auto dt=QDateTime(QDate(1601,1,1),QTime(0,0),QTimeZone(0)).addMSecs(micros/1000).toUTC();
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
 QSet<QString> seen,declaredPaths;QVariantList findings;int issues=0,missing=0,tampered=0,verified=0;
 qint64 payloadBytes=0;QSet<QString> payloadNames;
 if(!r.verifyManifestPayloads&&!r.manifestPayloads.isEmpty())return {{},"unexpected-manifest-payloads"};
 if(r.manifestPayloads.size()>128)return {{},"manifest-payload-limit"};
 for(auto it=r.manifestPayloads.cbegin();it!=r.manifestPayloads.cend();++it){
  if(!safeManifestPath(it.key())||!uuid(it.value().evidenceId)||!hash(it.value().sha256))return {{},"manifest-payload-provenance"};
  if(payloadNames.contains(it.key().toCaseFolded()))return {{},"manifest-duplicate-payload"};
  payloadNames.insert(it.key().toCaseFolded());
  payloadBytes+=it.value().bytes.size();
  if(payloadBytes>8*1024*1024)return {{},"manifest-payload-limit"};
  if(stopped(r))return {{},"cancelled"};
  if(sha(it.value().bytes)!=it.value().sha256)return {{},"manifest-payload-hash-mismatch"};
 }
 for(int i=0;i<entries.size();++i){
  if(stopped(r))return {{},"cancelled"};
   if(!entries[i].isObject())return {{},"manifest-entry-schema"};
   const auto e=entries[i].toObject();
  if(r.verifyManifestPayloads&&e.contains("path")&&e.contains("source_path"))return {{},"manifest-ambiguous-path"};
  const auto p=e.value("path").isUndefined()?e.value("source_path"):e.value("path");
  if(!p.isString()||p.toString().isEmpty()||p.toString().size()>2048||!e.value("sha256").isString())return {{},"manifest-entry-schema"};
  const auto path=p.toString();QStringList problems;
  if(r.verifyManifestPayloads&&!safeManifestPath(path))return {{},"manifest-unsafe-path"};
  const auto pathKey=r.verifyManifestPayloads?path.toCaseFolded():path;
  if(seen.contains(pathKey)){
   if(r.verifyManifestPayloads)return {{},"manifest-duplicate-path"};
   problems<<"duplicate-path";
  }
  seen.insert(pathKey);declaredPaths.insert(path);
  if(!hash(e.value("sha256").toString().toLower()))problems<<"malformed-sha256";
  QVariantMap content;
  if(r.verifyManifestPayloads){
   if(!problems.isEmpty())return {{},"manifest-malformed-sha256"};
   const auto payload=r.manifestPayloads.constFind(path);
   if(payload==r.manifestPayloads.cend()){++missing;content.insert("verification","missing");}
   else{
    const bool matches=payload.value().sha256==e.value("sha256").toString().toLower();
    if(matches)++verified;else ++tampered;
    content={{"verification",matches?"verified":"hash-mismatch"},{"payloadEvidenceId",payload.value().evidenceId},
     {"originalSha256",payload.value().sha256},{"workingCopySha256",sha(payload.value().bytes)},{"payloadBytes",payload.value().bytes.size()}};
   }
  }
  if(!problems.isEmpty())++issues;
  ForensicFinding f{"export-manifest-entry","/entries/"+QString::number(i),path,"structural-observation",{{"declaredSha256",e.value("sha256").toString()},{"issues",problems}}};f.details.insert(content);findings.append(f.toMap());
 }
 if(r.verifyManifestPayloads){
  for(auto it=r.manifestPayloads.cbegin();it!=r.manifestPayloads.cend();++it)if(!declaredPaths.contains(it.key()))return {{},"manifest-unreferenced-payload"};
 }
 report.insert("findings",findings);report.insert("entriesWithIssues",issues);report.insert("structurallyValid",issues==0);
 if(r.verifyManifestPayloads){
  report.insert("payloadVerified",missing==0&&tampered==0);report.insert("verifiedFiles",verified);report.insert("missingFiles",missing);report.insert("tamperedFiles",tampered);
  report.insert("analysisParameters",QVariantMap{{"mode","protected-case-payloads"},{"verifierVersion","nexvary-mr-manifest-content/1"},{"pathPolicy","portable-relative-no-filesystem-resolution"},{"maxPayloadBytes",8*1024*1024}});
  report.insert("warnings",QStringList{"Verification covers only the explicitly mapped protected case evidence; it does not establish acquisition authenticity or inspect a live folder."});
  return {report,{}};
 }
 report.insert("payloadVerified",false);report.insert("warnings",QStringList{"Structure inspection only; declared hashes are not verified against payload files. Use MR working-copy verification for actual evidence bytes."});return {report,{}};
}
}
bool safeManifestPath(const QString &path){
 if(path.isEmpty()||path.size()>2048||path!=path.normalized(QString::NormalizationForm_C))return false;
 for(const auto c:path)if(c.unicode()<32||c.unicode()==127||QString("\\:<>\"|?*").contains(c)||c.unicode()>126)return false;
 const auto parts=path.split('/');
 static const QRegularExpression device("\\A(?:con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\\.|$)",QRegularExpression::CaseInsensitiveOption);
 for(const auto &part:parts)if(part.isEmpty()||part=="."||part==".."||part.endsWith('.')||part.endsWith(' ')||device.match(part).hasMatch())return false;
 return true;
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
 if(r.adapter!="workbench-export-manifest"&&(r.verifyManifestPayloads||!r.manifestPayloads.isEmpty()))return {{},"payload-mode-not-supported"};
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
 if(adapter=="workbench-export-manifest"&&!r.contains("analysisParameters")&&
    (o.value("payloadVerified")!=QJsonValue(false)||r.contains("verifiedFiles")||r.contains("missingFiles")||r.contains("tamperedFiles")))return {{},"manifest-retained-mode"};
 if(adapter=="workbench-export-manifest"&&r.contains("analysisParameters")){
  const QVariantMap parameters{{"mode","protected-case-payloads"},{"verifierVersion","nexvary-mr-manifest-content/1"},{"pathPolicy","portable-relative-no-filesystem-resolution"},{"maxPayloadBytes",8*1024*1024}};
  if(r.value("analysisParameters").toMap()!=parameters)return {{},"manifest-retained-parameters"};
  QSet<QString> paths;int verified=0,missing=0,tampered=0;qint64 total=0;
  for(const auto &entry:o.value("findings").toArray()){
   const auto f=entry.toObject();const auto path=f.value("value").toString();const auto d=f.value("details").toObject();
   if(!safeManifestPath(path)||paths.contains(path.toCaseFolded())||!hash(d.value("declaredSha256").toString().toLower()))return {{},"manifest-retained-path"};
   paths.insert(path.toCaseFolded());const auto status=d.value("verification").toString();
   if(status=="missing"){++missing;if(d.contains("payloadEvidenceId")||d.contains("workingCopySha256"))return {{},"manifest-retained-payload"};continue;}
   const auto original=d.value("originalSha256").toString();const auto working=d.value("workingCopySha256").toString();
   const auto size=d.value("payloadBytes").toDouble(-1);
   if(!uuid(d.value("payloadEvidenceId").toString())||!hash(original)||working!=original||size<0||size>8*1024*1024||size!=qint64(size))return {{},"manifest-retained-payload"};
   total+=qint64(size);if(total>8*1024*1024)return {{},"manifest-retained-payload"};
   const bool matches=working==d.value("declaredSha256").toString().toLower();
   if(status=="verified"&&matches)++verified;
   else if(status=="hash-mismatch"&&!matches)++tampered;
   else return {{},"manifest-retained-verification"};
  }
  if(paths.isEmpty()||r.value("verifiedFiles")!=verified||r.value("missingFiles")!=missing||r.value("tamperedFiles")!=tampered||r.value("payloadVerified")!=(missing==0&&tampered==0))return {{},"manifest-retained-summary"};
 }
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
