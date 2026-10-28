#include "DcImagePartitions.hpp"
#include <QSet>
#include <QtEndian>

// Adapted from nexvary/NEXVARY-DC d8e2baf... src/core/Partitions.cpp.
// Owner-authorized integration grant; see docs/FORENSIC_SOURCE_PERMISSIONS.md.
// Deliberately excludes Operations, MediaRead, RescueEngine and filesystem output.
namespace mr::forensic {
namespace {
quint32 le32(const QByteArray &b,int p){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(b.constData()+p));}
quint64 le64(const QByteArray &b,int p){return qFromLittleEndian<quint64>(reinterpret_cast<const uchar*>(b.constData()+p));}
quint32 crc32(const QByteArray &b){quint32 c=~0u;for(uchar x:b){c^=x;for(int i=0;i<8;++i)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return ~c;}
bool extended(int t){return t==5||t==15||t==0x85;}
}
ForensicAdapterResult inspectDcPartitions(const ReadOnlyEvidenceRequest &r,QVariantMap report){
 const auto &image=r.verifiedBytes;constexpr quint64 sector=512;const auto sectors=quint64(image.size()/512);
 const auto cancelled=[&]{return (r.cancelled&&r.cancelled->load())||r.deadline.hasExpired();};
 if(image.size()<512||image.size()%512)return {{},"image-size-or-sector-alignment"};
 const auto read=[&](quint64 offset,quint64 count){if(offset>quint64(image.size())||count>quint64(image.size())-offset)return QByteArray();return image.mid(qsizetype(offset),qsizetype(count));};
 const auto m=read(0,512);if(uchar(m[510])!=85||uchar(m[511])!=170)return {{},"mbr-signature"};
 QVariantList findings;QStringList warnings{"512-byte logical sectors only. No filesystem analysis, repair, formatting, disk access or file recovery is performed."};
 QList<QPair<quint64,quint64>> extents;
 const auto warning=[&](const QString &s){if(warnings.size()<16)warnings<<s;};
 bool limit=false;
 const auto add=[&](quint64 lba,quint64 count,const QString &scheme,const QString &reference){
  if(!lba||!count||lba>=sectors||count>sectors-lba){warning("Partition outside image rejected");return;}
  for(const auto &v:extents)if(lba<v.first+v.second&&v.first<lba+count){warning("Overlapping partition rejected");return;}
  if(findings.size()>=128){limit=true;return;}extents.append({lba,count});
  ForensicFinding f{"image-partition",reference,scheme,"validated-structure-only",{{"startLba",qulonglong(lba)},{"sectorCount",qulonglong(count)},{"sectorBytes",512},{"offsetBytes",qulonglong(lba*sector)},{"lengthBytes",qulonglong(count*sector)}}};findings.append(f.toMap());
 };
 bool protective=false;for(int p=446;p<510;p+=16)protective|=uchar(m[p+4])==0xee;
 if(protective){
  bool accepted=false;
  for(quint64 headerLba:QList<quint64>{1,sectors?sectors-1:0}){
   if(cancelled())return {{},"cancelled"};
   auto h=read(headerLba*sector,sector);
   if(h.size()!=512||h.left(8)!="EFI PART"||le32(h,8)!=0x10000||le32(h,20)!=0)continue;
   const auto hs=le32(h,12),expected=le32(h,16);if(hs<92||hs>sector)continue;
   auto header=h.left(hs);for(int j=16;j<20;++j)header[j]=0;
   if(crc32(header)!=expected||le64(h,24)!=headerLba)continue;
   const auto alternate=le64(h,32),lba=le64(h,72),first=le64(h,40),last=le64(h,48);
   const auto n=le32(h,80),size=le32(h,84);const quint64 len=quint64(n)*size;
   if(alternate!=(headerLba==1?sectors-1:1)||first>last||first<2||last>=sectors-1||!n||n>4096||size<128||size>4096||size%128||len>2*1024*1024||lba>=sectors||len>quint64(image.size())-lba*sector)continue;
   const quint64 tableEnd=lba+(len+sector-1)/sector;
   if(lba==0||(lba<=headerLba&&headerLba<tableEnd)||(lba<=alternate&&alternate<tableEnd)||(lba<=last&&first<tableEnd))continue;
   const auto table=read(lba*sector,len);if(quint64(table.size())!=len||crc32(table)!=le32(h,88))continue;
   for(quint32 i=0;i<n;++i){
    if(cancelled())return {{},"cancelled"};
   const int p=int(quint64(i)*size);if(table.mid(p,16)==QByteArray(16,0))continue;
    const auto a=le64(table,p+32),b=le64(table,p+40);if(a<first||b>last||b<a){warning("Invalid GPT extent rejected");continue;}
    add(a,b-a+1,"GPT","GPT/entry/"+QString::number(i));
   }
   accepted=true;if(headerLba!=1)warning("Backup GPT accepted; primary invalid");break;
  }
  if(!accepted)return {{},"gpt-checksum-or-bounds"}; // Never reinterpret hybrid entries.
 }else{
  for(int p=446;p<510;p+=16){
   if(cancelled())return {{},"cancelled"};
   const int type=uchar(m[p+4]);const quint64 base=le32(m,p+8),count=le32(m,p+12);if(!type)continue;
   if(!extended(type)){add(base,count,"MBR","MBR/entry/"+QString::number((p-446)/16));continue;}
   if(!base||base>=sectors||!count||count>sectors-base){warning("Invalid extended container rejected");continue;}
   quint64 next=base;QSet<quint64> seen;
   while(next>=base&&next<base+count){
    if(cancelled())return {{},"cancelled"};
   if(seen.contains(next)||seen.size()>=4096){warning("EBR cycle or traversal limit");break;}seen.insert(next);
    const auto e=read(next*sector,512);if(e.size()!=512||uchar(e[510])!=85||uchar(e[511])!=170){warning("Invalid EBR");break;}
    const quint64 rel=le32(e,454),len=le32(e,458);
    if(uchar(e[450])&&!extended(uchar(e[450]))&&rel&&rel<base+count-next&&len&&len<=base+count-next-rel)add(next+rel,len,"MBR logical","EBR/LBA/"+QString::number(next));
    else if(uchar(e[450]))warning("Logical partition outside extended container rejected");
    if(!uchar(e[466]))break;
    if(!extended(uchar(e[466]))){warning("Invalid EBR link");break;}
    const quint64 link=le32(e,470);if(!link||link>=count){warning("EBR link outside container");break;}next=base+link;
   }
  }
 }
 if(limit)return {{},"findings-limit"};
 report.insert("findings",findings);report.insert("warnings",warnings);report.insert("sectorBytes",512);report.insert("partitionCount",findings.size());
 report.insert("hasRejectedStructures",warnings.size()>1);return {report,{}};
}
}
