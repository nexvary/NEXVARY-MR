#include "NtfsMetadata.hpp"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QSet>
#include <QStringDecoder>
#include <QtEndian>
#include <bit>
#include <limits>

namespace mr::forensic {
namespace {
constexpr qint64 limit=8*1024*1024;
quint16 u16(const QByteArray &b,qsizetype p){return qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(b.constData()+p));}
quint32 u32(const QByteArray &b,qsizetype p){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(b.constData()+p));}
quint64 u64(const QByteArray &b,qsizetype p){return qFromLittleEndian<quint64>(reinterpret_cast<const uchar*>(b.constData()+p));}
QString sha(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
struct Run {qint64 physical=0,length=0;};
struct Stream {QByteArray resident;QList<Run> runs;qint64 size=0,initialized=0;bool valid=false,isResident=false;};
class Parser {
 const ReadOnlyEvidenceRequest &r;const QByteArray &image;QString error;
 qint64 volume=0,cluster=0,recordSize=0;Stream mft;QByteArray bitmap;int empty=0,extensions=0,named=0;
 bool fail(const QString &code){if(error.isEmpty())error=code;return false;}
 bool active(){return r.cancelled&&r.cancelled->load()?fail("cancelled"):r.deadline.hasExpired()?fail("worker-timeout"):true;}
 QByteArray read(qint64 p,qint64 n){if(!active()||p<0||n<0||p>volume||n>volume-p){fail("ntfs-image-range");return {};}return image.mid(p,n);}
 Stream stream(const QByteArray &a){
  Stream s;if(a.size()<24){fail("ntfs-attribute-short");return s;}
  if(a[8]==0){const auto n=u32(a,16);const auto p=u16(a,20);if(p<24||quint64(p)+n>quint64(a.size())||u16(a,12)){fail("ntfs-resident-range");return s;}s.resident=a.mid(p,n);s.size=s.initialized=n;s.valid=s.isResident=true;return s;}
  if(a[8]!=1||a.size()<64||u64(a,16)!=0||(u16(a,12)&~quint16(0x8000))||a[34]!=0){fail("ntfs-stream-unsupported");return s;}
  const auto size=u64(a,48),initialized=u64(a,56),highest=u64(a,24);int p=u16(a,32);
  if(size>quint64(volume)||initialized>size||p<(u16(a,12)&0x8000?72:64)||p>=a.size()){fail("ntfs-stream-range");return s;}
  qint64 lcn=0,total=0;bool ended=false;
  while(p<a.size()){
   if(!active())return {};
   const int h=uchar(a[p++]);if(!h){ended=true;break;}
   const int ns=h&15,os=h>>4;if(ns<1||ns>8||os>8||p+ns+os>a.size()||s.runs.size()>=128){fail("ntfs-runlist-bound");return {};}
   quint64 count=0,bits=0;for(int j=0;j<ns;++j)count|=quint64(uchar(a[p++]))<<(8*j);
   if(!count||count>quint64((volume-total)/cluster)){fail("ntfs-runlist-length");return {};}
   for(int j=0;j<os;++j)bits|=quint64(uchar(a[p++]))<<(8*j);
   if(os){
    if(os<8&&(bits&(quint64(1)<<(os*8-1))))bits|=(~quint64(0))<<(os*8);
    const auto delta=std::bit_cast<qint64>(bits);
    if((delta>0&&lcn>std::numeric_limits<qint64>::max()-delta)||(delta<0&&delta < -lcn)){fail("ntfs-runlist-delta");return {};}
    lcn+=delta;
    if(lcn<0||lcn>volume/cluster||count>quint64(volume/cluster-lcn)){fail("ntfs-runlist-physical");return {};}
   }
   if(!os&&!(u16(a,12)&0x8000)){fail("ntfs-sparse-flag");return {};}
   const auto n=qint64(count)*cluster,off=os?lcn*cluster:qint64(-1);
   if(off>=0)for(const auto &old:s.runs)if(old.physical>=0&&off<old.physical+old.length&&old.physical<off+n){fail("ntfs-runlist-overlap");return {};}
   s.runs.append({off,n});total+=n;
  }
  if(!ended||!total||total<qint64(size)||highest!=quint64(total/cluster)-1){fail("ntfs-runlist-incomplete");return {};}
  s.size=qint64(size);s.initialized=qint64(initialized);s.valid=true;return s;
 }
 QByteArray streamRead(const Stream &s,qint64 pos,qint64 n,QVariantList *refs=nullptr){
  if(!s.valid||pos<0||n<0||pos>s.size||n>s.size-pos||pos+n>s.initialized){fail("ntfs-stream-read");return {};}
  if(s.isResident)return s.resident.mid(pos,n);
  QByteArray out;qint64 logical=0;
  for(const auto &run:s.runs){
   if(!active())return {};
   if(pos<logical+run.length&&out.size()<n){
    const auto within=pos-logical,take=qMin(n-out.size(),run.length-within);
    if(run.physical<0){fail("ntfs-metadata-sparse");return {};}
    const auto b=read(run.physical+within,take);if(b.size()!=take)return {};
    if(refs)refs->append(QVariantMap{{"byteOffset",run.physical+within},{"lengthBytes",take},{"recordByteOffset",out.size()}});
    out+=b;pos+=take;
   }
   logical+=run.length;if(out.size()==n)break;
  }
  if(out.size()!=n){fail("ntfs-stream-read");return {};}
  return out;
 }
 bool fixup(QByteArray &b){
  if(b.size()!=recordSize||!b.startsWith("FILE"))return fail("ntfs-record-signature");
  const int p=u16(b,4),count=u16(b,6),first=u16(b,20);const auto used=u32(b,24);
  // Full array includes the leading USN. Never let it overlap its first trailer.
  if(count!=recordSize/512+1||p<48||p%2||p+2*count>510||p+2*count>first||first<48||first%8||used>quint32(b.size())||used<quint32(first+4)||u32(b,28)!=quint32(recordSize))return fail("ntfs-usa-or-record-bound");
  const auto usn=b.mid(p,2);if(usn==QByteArray(2,0))return fail("ntfs-usa-zero");
  for(int i=1;i<count;++i){const int tail=i*512-2;if(b.mid(tail,2)!=usn)return fail("ntfs-torn-record");b.replace(tail,2,b.mid(p+2*i,2));}
  return true;
 }
 QList<QByteArray> attributes(const QByteArray &b){
  QList<QByteArray> result;const auto used=u32(b,24);quint32 p=u16(b,20);QSet<quint16> ids;
  while(p+4<=used){
   if(u32(b,p)==0xffffffffu)return result;
   if(!active()||p+24>used||result.size()>=64){fail("ntfs-attributes-bound");return {};}
   const auto n=u32(b,p+4);if(n<24||n%8||n>used-p){fail("ntfs-attribute-length");return {};}
   const auto a=b.mid(p,n);const auto names=uchar(a[9]);const auto nameOffset=u16(a,10),id=u16(a,14);
   if(ids.contains(id)||(names&&(nameOffset<24||quint32(nameOffset)+2u*names>n))){fail("ntfs-attribute-name-or-id");return {};}
   ids.insert(id);if(u32(a,0)==0x20){fail("ntfs-attribute-list-unsupported");return {};}
   result.append(a);p+=n;
  }
  fail("ntfs-attribute-terminator");return {};
 }
 Stream data(const QList<QByteArray> &attrs){
  Stream result;bool found=false;
  for(const auto &a:attrs)if(u32(a,0)==0x80){
   if(a[9]!=0){++named;continue;}
   if(found){fail("ntfs-multiple-data-extents-unsupported");return {};}
   found=true;result=stream(a);
  }
  return result;
 }
 QVariantList names(const QList<QByteArray> &attrs){
  QVariantList result;
  for(const auto &a:attrs)if(u32(a,0)==0x30){
   if(a[8]!=0){fail("ntfs-name-nonresident");return {};}
   const auto s=stream(a);const auto &b=s.resident;
   if(!s.valid||b.size()<66||!uchar(b[64])||66+2*uchar(b[64])>b.size()||uchar(b[65])>3||result.size()>=8){fail("ntfs-name-bound");return {};}
   QStringDecoder decoder(QStringDecoder::Utf16LE,QStringConverter::Flag::Stateless);const QString name=decoder(b.mid(66,2*uchar(b[64])));
   if(decoder.hasError()||name.contains(QChar(0))){fail("ntfs-name-encoding");return {};}
   for(qsizetype i=0;i<name.size();++i){
    if(name[i].isHighSurrogate()){if(i+1>=name.size()||!name[i+1].isLowSurrogate()){fail("ntfs-name-encoding");return {};}++i;}
    else if(name[i].isLowSurrogate()){fail("ntfs-name-encoding");return {};}
   }
   result.append(QVariantMap{{"name",name},{"namespace",uchar(b[65])},{"parentReferenceRaw",QString::number(u64(b,0))},{"createdFiletimeRaw",QString::number(u64(b,8))},{"modifiedFiletimeRaw",QString::number(u64(b,16))},{"recordChangedFiletimeRaw",QString::number(u64(b,24))},{"accessedFiletimeRaw",QString::number(u64(b,32))}});
  }
  return result;
 }
 QVariantMap describe(const Stream &s,bool deleted){
  if(!s.valid)return {{"kind","not-present"},{"recovery","not-performed"}};
  QVariantMap d{{"kind",s.isResident?"resident":"nonresident"},{"logicalBytes",s.size},{"initializedBytes",s.initialized},{"recovery","not-performed"}};
  if(s.isResident){d.insert("reconstructedResidentSha256",sha(s.resident));return d;}
  QVariantList extents;qint64 logical=0,free=0,allocated=0,sparse=0;
  for(const auto &run:s.runs){
   const auto relevant=qMax(qint64(0),qMin(run.length,s.size-logical));
   extents.append(QVariantMap{{"logicalByteOffset",logical},{"byteOffset",run.physical},{"lengthBytes",run.length},{"kind",run.physical<0?"sparse-logical-zero":"recorded-data-run"}});
   if(run.physical<0)sparse+=relevant;
   else if(!bitmap.isEmpty())for(qint64 j=0;j<relevant;j+=cluster){
    if(!active())return {};
    const auto c=run.physical/cluster+j/cluster;const auto n=qMin(cluster,relevant-j);
    if(uchar(bitmap[c/8])&(1<<(c%8)))allocated+=n;else free+=n;
   }
   logical+=run.length;
  }
  d.insert("extents",extents);d.insert("recordedSparseLogicalBytes",sparse);
  d.insert("allocationObservation",bitmap.isEmpty()?"bitmap-unavailable":deleted?"deleted-record-bitmap-observation":"live-record-bitmap-observation");
  if(!bitmap.isEmpty()){d.insert("recordedUnallocatedBytes",free);d.insert("recordedAllocatedBytes",allocated);}
  return d;
 }
public:
 explicit Parser(const ReadOnlyEvidenceRequest &request):r(request),image(request.verifiedBytes){}
 ForensicAdapterResult parse(){
  const auto reject=[&]{return ForensicAdapterResult{{},error};};
  if(!active())return reject();
  if(image.size()<512||image.size()>limit||sha(image)!=r.sourceSha256){fail("ntfs-input-or-hash");return reject();}
  if(image.mid(3,8)!="NTFS    "||image.mid(510,2)!=QByteArray::fromHex("55aa")||u16(image,11)!=512){fail("ntfs-volume-unsupported");return reject();}
  const auto spc=uchar(image[13]);const auto sectors=u64(image,40),mftCluster=u64(image,48);const auto code=std::bit_cast<qint8>(uchar(image[64]));
  if(!spc||spc>128||(spc&(spc-1))||!sectors||sectors>quint64(image.size()/512)){fail("ntfs-geometry");return reject();}
  volume=qint64(sectors)*512;cluster=qint64(spc)*512;recordSize=code<0?(code>=-16?qint64(1)<<(-code):0):qint64(code)*cluster;
  if(recordSize<512||recordSize>65536||recordSize%512||recordSize>cluster||mftCluster>=quint64(volume/cluster)){fail("ntfs-record-geometry");return reject();}
  const auto first=read(qint64(mftCluster)*cluster,recordSize);auto fixed=first;if(!fixup(fixed))return reject();if(u64(fixed,32)!=0||(u16(fixed,22)&3)!=1){fail("ntfs-mft-base-reference");return reject();}
  const auto attrs=attributes(fixed);if(!error.isEmpty())return reject();mft=data(attrs);
  if(!error.isEmpty())return reject();
  if(!mft.valid||mft.isResident||mft.initialized!=mft.size||mft.size%recordSize||mft.size/recordSize<16||mft.size/recordSize>128){fail("ntfs-mft-bound");return reject();}
  if(streamRead(mft,0,recordSize)!=first){fail("ntfs-mft-bootstrap-map");return reject();}
  auto vol=streamRead(mft,3*recordSize,recordSize);if(!fixup(vol))return reject();if(u64(vol,32)||!(u16(vol,22)&1)){fail("ntfs-volume-base-reference");return reject();}const auto va=attributes(vol);if(!error.isEmpty())return reject();bool version=false,versionSeen=false;
  for(const auto &a:va)if(u32(a,0)==0x70){if(versionSeen){fail("ntfs-volume-version-duplicate");return reject();}versionSeen=true;const auto s=stream(a);if(s.valid&&s.isResident&&s.size>=12&&s.resident[8]==3&&(s.resident[9]==0||s.resident[9]==1))version=true;}
  if(!version){fail("ntfs-version-unsupported");return reject();}
  auto br=streamRead(mft,6*recordSize,recordSize);
  if(!br.isEmpty()&&br!=QByteArray(br.size(),0)){
   if(!fixup(br))return reject();
   if(u64(br,32)||!(u16(br,22)&1)){fail("ntfs-bitmap-base-reference");return reject();}
   const auto bs=data(attributes(br));
   if(!error.isEmpty())return reject();
   const auto needed=(volume/cluster+7)/8;
   if(bs.valid&&bs.size>=needed&&bs.size<=65536&&bs.initialized==bs.size)bitmap=streamRead(bs,0,bs.size);
  }
  QVariantList findings;
  for(qint64 i=0;i<mft.size/recordSize;++i){
   QVariantList raw;const auto b=streamRead(mft,i*recordSize,recordSize,&raw);if(!error.isEmpty())return reject();
   if(b==QByteArray(b.size(),0)){++empty;continue;}
   auto fixedRecord=b;if(!fixup(fixedRecord))return reject();
   if(u64(fixedRecord,32)){++extensions;continue;}
   const auto as=attributes(fixedRecord);if(!error.isEmpty())return reject();
   if(i<16)continue;
   const auto ns=names(as);const auto ds=data(as);if(!error.isEmpty())return reject();
   if(ns.isEmpty()){if(!(u16(fixedRecord,22)&1)&&!ds.valid){++empty;continue;}fail("ntfs-user-name-missing");return reject();}
   const bool deleted=!(u16(fixedRecord,22)&1);const auto detail=describe(ds,deleted);if(!error.isEmpty())return reject();
   QVariantMap d{{"recordIndex",i},{"sequence",u16(fixedRecord,16)},{"deleted",deleted},{"directory",bool(u16(fixedRecord,22)&2)},{"names",ns},{"dataStream",detail},{"rawRecordExtents",raw},{"rawRecordSha256",sha(b)},{"usaVerified",true}};
   findings.append(QVariantMap{{"kind","ntfs-file-record"},{"reference","ntfs/mft/record/"+QString::number(i)},{"value",ns[0].toMap().value("name")},{"confidence","recorded-metadata-only"},{"details",d}});
  }
  if(!active())return reject();
  QVariantMap report{{"schema","nexvary-mr-ntfs-metadata/1"},{"sourceSha256",r.sourceSha256},{"sourceBytes",image.size()},{"sourceRepository","nexvary/NEXVARY-DC"},{"sourceCommit","d8e2baf61b43d044b33a1dd76a444d093c80023c"},{"sourceModule","src/core/FilesystemRecovery.cpp:fixup/attributes/ntStream/ntfs; bounded metadata-only derivative"},{"parserVersion","nexvary-mr-ntfs-metadata/1"},{"geometry",QVariantMap{{"volumeBytes",volume},{"clusterBytes",cluster},{"recordBytes",recordSize},{"sectorBytes",512},{"trailingImageBytes",image.size()-volume}}},{"mftRecords",mft.size/recordSize},{"emptySlots",empty},{"extensionRecordsExcluded",extensions},{"namedDataStreamsExcluded",named},{"bitmapAvailable",!bitmap.isEmpty()},{"findings",findings},{"limitations",QStringList{"Internal metadata prerequisite only; not an enabled case adapter or recovered file.","512-byte-sector NTFS3.0/3.1 volume-image prefix only; at most128 MFT records and128 runs/stream. No attribute-list, compression/encryption or named-stream interpretation.","Raw record extents reference pre-fixup protected bytes; resident hashes refer to USA-reconstructed data. Internal consistency does not establish authenticity or absence of overwrite.","Bitmap bits/counters are recorded observations, not recovery success. No file bytes are saved, no target paths are resolved, and no source/device/filesystem writes occur."}}};
  if(QJsonDocument::fromVariant(report).toJson(QJsonDocument::Compact).size()>65536){fail("ntfs-output-limit");return reject();}
  return {report,{}};
 }
};
}
ForensicAdapterResult inspectNtfsMetadata(const ReadOnlyEvidenceRequest &r){return Parser(r).parse();}
}
