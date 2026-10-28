#include "SqliteEvidenceFamily.hpp"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSet>
#include <QUuid>
#include <QtEndian>

namespace mr::forensic {
namespace {
constexpr int byteLimit=8*1024*1024,frameLimit=4096,shmLimit=128*1024;
const QString acquisition="operator-bound-offline-members; common acquisition not independently established";
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
bool hash(const QString &s){static const QRegularExpression re("\\A[0-9a-f]{64}\\z");return re.match(s).hasMatch();}
bool uuid(const QString &s){return !QUuid(s).isNull()&&QUuid(s).toString(QUuid::WithoutBraces)==s;}
bool stopped(const ReadOnlyEvidenceRequest &r){return (r.cancelled&&r.cancelled->load())||r.deadline.hasExpired();}
quint32 be32(const QByteArray &b,qsizetype p){return qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(b.constData()+p));}
quint32 word(const QByteArray &b,qsizetype p,bool big){const auto raw=reinterpret_cast<const uchar*>(b.constData()+p);return big?qFromBigEndian<quint32>(raw):qFromLittleEndian<quint32>(raw);}
void checksum(const QByteArray &b,qsizetype off,qsizetype size,bool big,quint32 &s0,quint32 &s1){
 for(qsizetype i=off;i<off+size;i+=8){s0+=word(b,i,big)+s1;s1+=word(b,i+4,big)+s0;}
}
QVariantMap parameters(){return {{"verifierVersion","nexvary-mr-sqlite-family/1"},{"maxTotalBytes",byteLimit},{"maxSnapshotBytes",byteLimit},{"maxFrames",frameLimit},{"maxShmBytes",shmLimit},{"walPolicy","all-frames-valid; last-commit-only; incomplete-or-stale-tail-rejected"},{"shmPolicy","opaque-hash-only; never used for replay"},{"snapshotPolicy","page-image-only; no header normalization or SQL execution"},{"frameNumbering","1-based; 0 means no committed WAL frame"}};}
QVariantMap member(const QString &role,const QString &id,const QString &digest,qsizetype size){
 return {{"role",role},{"evidenceId",id},{"originalSha256",digest},{"workingCopySha256",digest},{"bytes",size}};
}
QString familyHash(const QVariantList &members){
 QVariantList material;for(const auto &v:members){const auto m=v.toMap();material.append(QVariantMap{{"role",m.value("role")},{"sha256",m.value("originalSha256")},{"bytes",m.value("bytes")}});}
 return sha(QJsonDocument::fromVariant(material).toJson(QJsonDocument::Compact));
}
QString reference(int frame){return "sqlite/main+wal/last-commit-frame/"+QString::number(frame);}
SqliteFamilySnapshot reject(const QString &error){return {{{},error},{}};}
}
SqliteFamilySnapshot inspectSqliteEvidenceFamily(const ReadOnlyEvidenceRequest &r,QVariantMap report){
 if(stopped(r))return reject(r.deadline.hasExpired()?"worker-timeout":"cancelled");
 const auto &main=r.verifiedBytes;
 if(!uuid(r.evidenceId)||!hash(r.sourceSha256)||sha(main)!=r.sourceSha256)return reject("sqlite-family-main-provenance");
 if(main.size()<100||main.size()>byteLimit||!main.startsWith(QByteArray("SQLite format 3\0",16)))return reject("sqlite-family-main-header");
 const auto encoded=qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(main.constData()+16));
 const int pageSize=encoded==1?65536:int(encoded);
 if(pageSize<512||pageSize>65536||(pageSize&(pageSize-1))||main.size()%pageSize||uchar(main[20])>pageSize-480||main[21]!=64||main[22]!=32||main[23]!=32)return reject("sqlite-family-page-size-or-header");
 const int writeVersion=uchar(main[18]),readVersion=uchar(main[19]);
 if((writeVersion!=1&&writeVersion!=2)||writeVersion!=readVersion)return reject("sqlite-family-journal-version");
 const auto physicalPages=quint32(main.size()/pageSize),declaredPages=be32(main,28);
 const bool validCount=declaredPages>0&&be32(main,24)==be32(main,92);
 if(validCount&&declaredPages>physicalPages)return reject("sqlite-family-missing-main-pages");
 quint32 pages=validCount?declaredPages:physicalPages;
 QVariantList members{member("main",r.evidenceId,r.sourceSha256,main.size())};QSet<QString> ids{r.evidenceId};qint64 total=main.size();
 if(r.sqliteFamilyMembers.size()>2)return reject("sqlite-family-member-count");
 for(const auto &role:QStringList{"wal","shm"}){
  if(!r.sqliteFamilyMembers.contains(role))continue;
  const auto &m=r.sqliteFamilyMembers[role];
  if(!uuid(m.evidenceId)||ids.contains(m.evidenceId)||!hash(m.sha256)||m.bytes.isEmpty())return reject("sqlite-family-member-provenance");
  ids.insert(m.evidenceId);total+=m.bytes.size();
  if(total>byteLimit||(role=="shm"&&m.bytes.size()>shmLimit))return reject("sqlite-family-input-limit");
  if(stopped(r))return reject("cancelled");
  if(sha(m.bytes)!=m.sha256)return reject("sqlite-family-member-hash");
  members.append(member(role,m.evidenceId,m.sha256,m.bytes.size()));
 }
 for(auto it=r.sqliteFamilyMembers.cbegin();it!=r.sqliteFamilyMembers.cend();++it)if(it.key()!="wal"&&it.key()!="shm")return reject("sqlite-family-member-role");
 if(r.sqliteFamilyMembers.contains("shm")&&!r.sqliteFamilyMembers.contains("wal"))return reject("sqlite-family-orphan-shm");
 const auto wal=r.sqliteFamilyMembers.value("wal").bytes;
 QMap<quint32,qsizetype> committed,pending;int frames=0,commits=0,lastCommit=0;
 if(!wal.isEmpty()){
  if(writeVersion!=2||wal.size()<32)return reject("sqlite-family-wal-header");
  const auto magic=be32(wal,0);const bool big=magic==0x377f0683u;
  if((magic!=0x377f0682u&&!big)||be32(wal,4)!=3007000u||be32(wal,8)!=quint32(pageSize))return reject("sqlite-family-wal-header");
  quint32 s0=0,s1=0;checksum(wal,0,24,big,s0,s1);
  if(s0!=be32(wal,24)||s1!=be32(wal,28))return reject("sqlite-family-wal-header-checksum");
  const qsizetype stride=pageSize+24;
  if((wal.size()-32)%stride)return reject("sqlite-family-incomplete-wal");
  if((wal.size()-32)/stride>frameLimit)return reject("sqlite-family-frame-limit");
  const auto maximumPage=quint32(byteLimit/pageSize);
  for(qsizetype offset=32;offset<wal.size();offset+=stride){
   if(stopped(r))return reject("cancelled");
   const auto page=be32(wal,offset),size=be32(wal,offset+4);++frames;
   if(!page||page>maximumPage||size>maximumPage)return reject("sqlite-family-page-bound");
   if(be32(wal,offset+8)!=be32(wal,16)||be32(wal,offset+12)!=be32(wal,20))return reject("sqlite-family-frame-salt");
   checksum(wal,offset,8,big,s0,s1);checksum(wal,offset+24,pageSize,big,s0,s1);
   if(s0!=be32(wal,offset+16)||s1!=be32(wal,offset+20))return reject("sqlite-family-frame-checksum");
   pending.insert(page,offset+24);
   if(size){
    // Growing a previously truncated image must not resurrect stale main/WAL pages.
    for(quint32 p=pages+1;p<=size;++p){if(stopped(r))return reject("cancelled");if(!pending.contains(p))return reject("sqlite-family-missing-committed-page");}
    for(auto it=committed.begin();it!=committed.end();)if(it.key()>size)it=committed.erase(it);else ++it;
    for(auto it=pending.cbegin();it!=pending.cend();++it)if(it.key()<=size)committed.insert(it.key(),it.value());
    pages=size;pending.clear();++commits;lastCommit=frames;
   }
  }
 }
 const auto imageBytes=qsizetype(pages)*pageSize;
 QByteArray image=main.left(imageBytes);if(image.size()<imageBytes)image+=QByteArray(imageBytes-image.size(),0);
 for(auto it=committed.cbegin();it!=committed.cend();++it){if(stopped(r))return reject("cancelled");image.replace(qsizetype(it.key()-1)*pageSize,pageSize,wal.mid(it.value(),pageSize));}
 if(!image.startsWith(QByteArray("SQLite format 3\0",16)))return reject("sqlite-family-derived-header");
 const auto derivedPageSize=qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(image.constData()+16));
 if((derivedPageSize==1?65536:int(derivedPageSize))!=pageSize)return reject("sqlite-family-derived-page-size");
 if((uchar(image[18])!=1&&uchar(image[18])!=2)||image[18]!=image[19]||uchar(image[20])>pageSize-480||image[21]!=64||image[22]!=32||image[23]!=32)return reject("sqlite-family-derived-header");
 if(be32(image,28)&&be32(image,24)==be32(image,92)&&be32(image,28)!=pages)return reject("sqlite-family-derived-page-count");
 const auto digest=sha(image);
 if(stopped(r))return reject(r.deadline.hasExpired()?"worker-timeout":"cancelled");
 const auto rawRole=lastCommit?QString("wal"):QString("main");const auto rawId=lastCommit?r.sqliteFamilyMembers.value("wal").evidenceId:r.evidenceId;
 const auto rawHash=lastCommit?r.sqliteFamilyMembers.value("wal").sha256:r.sourceSha256;
 const qint64 rawOffset=lastCommit?32+qint64(lastCommit-1)*(pageSize+24):0;
 const QVariantMap raw{{"role",rawRole},{"evidenceId",rawId},{"sourceSha256",rawHash},{"byteOffset",rawOffset},{"lengthBytes",lastCommit?pageSize+24:100}};
 QVariantMap summary{{"rawArtifact",raw},{"members",members},{"familySha256",familyHash(members)},{"pageSize",pageSize},{"mainPhysicalPages",physicalPages},{"snapshotPages",pages},{"snapshotBytes",image.size()},{"snapshotSha256",digest},{"walFrames",frames},{"commitFrames",commits},{"lastCommitFrame",lastCommit},{"uncommittedFrames",frames-lastCommit},{"shmUsedForReplay",false},{"acquisitionProvenance",acquisition}};
 QStringList warnings{"Checksums validate WAL structure, not authenticity or common acquisition of these operator-bound evidence members.","Page-image observation only: no SQL/B-tree integrity, History, Downloads, session reconstruction or live-profile access. SHM is retained as opaque hashed provenance and never trusted for replay."};
 if(wal.isEmpty())warnings<<"No WAL supplied; a missing sidecar or incomplete acquisition cannot be ruled out.";
 if(frames>lastCommit)warnings<<"Valid uncommitted trailing frames were excluded from the derived page image.";
 report.insert("analysisParameters",parameters());report.insert("sqliteFamily",summary);report.insert("warnings",warnings);
 report.insert("findings",QVariantList{ForensicFinding{"browser-sqlite-family",reference(lastCommit),digest,"validated-structure-only",summary}.toMap()});
 return {{report,{}},image};
}
ForensicAdapterResult validateRetainedSqliteFamily(const QVariantMap &report,const QMap<QString,QVariantMap> &inventory){
 const auto bad=[] {return ForensicAdapterResult{{},"sqlite-family-retained-provenance"};};
 if(report.value("analysisParameters").toMap()!=parameters())return bad();
 const auto s=report.value("sqliteFamily").toMap();const auto members=s.value("members").toList();
 if(members.isEmpty()||members.size()>3||s.value("shmUsedForReplay")!=false||s.value("acquisitionProvenance")!=acquisition)return bad();
 QSet<QString> ids,roles;qint64 total=0;QStringList order;
 for(const auto &value:members){
  const auto m=value.toMap();const auto role=m.value("role").toString(),id=m.value("evidenceId").toString(),digest=m.value("originalSha256").toString();const auto size=m.value("bytes").toLongLong();
  if(!QStringList{"main","wal","shm"}.contains(role)||roles.contains(role)||!uuid(id)||ids.contains(id)||!hash(digest)||m.value("workingCopySha256")!=digest||size<=0||size>byteLimit||(role=="shm"&&size>shmLimit))return bad();
  const auto source=inventory.value(id);if(source.isEmpty()||source.value("sha256")!=digest||source.value("size").toLongLong()!=size)return bad();
  if(role=="main"&&(id!=report.value("evidenceId")||digest!=report.value("sourceSha256")))return bad();
  roles.insert(role);ids.insert(id);order<<role;total+=size;
 }
 const QStringList expected=roles.contains("shm")?QStringList{"main","wal","shm"}:roles.contains("wal")?QStringList{"main","wal"}:QStringList{"main"};
 if(order!=expected||total>byteLimit||s.value("familySha256")!=familyHash(members))return bad();
 const auto integer=[&s](const QString &key,qint64 lo,qint64 hi){const auto value=s.value(key);bool ok=false;const auto n=value.toLongLong(&ok);return ok&&value.toDouble()==double(n)&&n>=lo&&n<=hi;};
 if(!integer("pageSize",512,65536)||!integer("snapshotPages",1,byteLimit/512)||!integer("snapshotBytes",512,byteLimit)||!integer("mainPhysicalPages",1,byteLimit/512)||!integer("walFrames",0,frameLimit)||!integer("commitFrames",0,frameLimit)||!integer("lastCommitFrame",0,frameLimit)||!integer("uncommittedFrames",0,frameLimit)||!hash(s.value("snapshotSha256").toString()))return bad();
 const auto ps=s.value("pageSize").toInt(),frames=s.value("walFrames").toInt(),commits=s.value("commitFrames").toInt(),last=s.value("lastCommitFrame").toInt();
 if((ps&(ps-1))||s.value("snapshotPages").toLongLong()*ps!=s.value("snapshotBytes").toLongLong()||s.value("mainPhysicalPages").toLongLong()*ps!=members[0].toMap().value("bytes").toLongLong()||last>frames||commits>last||(commits==0)!=(last==0)||s.value("uncommittedFrames").toInt()!=frames-last)return bad();
 if(roles.contains("wal")){const auto size=members[1].toMap().value("bytes").toLongLong();if(size!=32+qint64(frames)*(ps+24))return bad();}else if(frames||commits||last)return bad();
 const auto rawSource=members[last?1:0].toMap();const QVariantMap raw{{"role",last?"wal":"main"},{"evidenceId",rawSource.value("evidenceId")},{"sourceSha256",rawSource.value("originalSha256")},{"byteOffset",last?32+qint64(last-1)*(ps+24):qint64(0)},{"lengthBytes",last?ps+24:100}};
 if(s.value("rawArtifact").toMap()!=raw)return bad();
 const auto rows=report.value("findings").toList();if(rows.size()!=1)return bad();const auto f=rows[0].toMap();
 if(f.value("kind")!="browser-sqlite-family"||f.value("confidence")!="validated-structure-only"||f.value("reference")!=reference(last)||f.value("value")!=s.value("snapshotSha256")||f.value("details").toMap()!=s)return bad();
 return {report,{}};
}
}
