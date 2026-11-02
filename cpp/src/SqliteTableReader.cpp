#include "SqliteTableReader.hpp"
#include "SqliteEvidenceFamily.hpp"
#include <QStringDecoder>
#include <QSet>
#include <QtEndian>
#include <bit>
#include <limits>

namespace mr::forensic {
namespace {
constexpr int maxRows=4096,maxPayload=64*1024,maxFields=256,maxDepth=32;
class Reader {
 const QByteArray &image;
 const ReadOnlyEvidenceRequest &request;
 int pageSize=0,usable=0;
 QSet<quint32> seen;
 qsizetype decoded=0;
 QString error;
 QList<SqliteTableRow> rows;
 bool fail(const QString &code){if(error.isEmpty())error=code;return false;}
 bool active(){return (request.cancelled&&request.cancelled->load())?fail("cancelled"):request.deadline.hasExpired()?fail("worker-timeout"):true;}
 quint32 u32(qsizetype p) const {return qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(image.constData()+p));}
 int u16(qsizetype p) const {return qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(image.constData()+p));}
 bool claim(quint32 page){
  if(!active())return false;
  if(!page||page>quint32(image.size()/pageSize)||seen.contains(page))return fail("sqlite-table-page-reference");
  seen.insert(page);return true;
 }
 bool varint(const QByteArray &b,qsizetype &p,qsizetype end,quint64 &v){
  v=0;
  for(int i=0;i<9;++i){
   if(p>=end)return fail("sqlite-table-varint");
   const auto c=uchar(b[p++]);
   if(i==8){v=(v<<8)|c;return true;}
   v=(v<<7)|(c&127);if(!(c&128))return true;
  }
  return false;
 }
 bool record(const QByteArray &payload,QVariantList &values,QList<quint64> &types){
  qsizetype p=0;quint64 header=0;
  if(!varint(payload,p,payload.size(),header)||header<quint64(p)||header>quint64(payload.size()))return fail("sqlite-table-record-header");
  while(p<qsizetype(header)){
   quint64 type=0;if(types.size()>=maxFields||!varint(payload,p,qsizetype(header),type))return fail("sqlite-table-record-header");
   types.append(type);
  }
  p=qsizetype(header);
  for(const auto type:types){
   if(!active())return false;
   if(type==10||type==11)return fail("sqlite-table-reserved-serial-type");
   // Reject floating-point rather than silently coercing forensic integer fields.
   if(type==7)return fail("sqlite-table-float-unsupported");
   const quint64 length=type>=12?(type-12)/2:type==6?8:type==5?6:type==4?4:type==3?3:type==2?2:type==1?1:0;
   if(length>quint64(payload.size()-p))return fail("sqlite-table-record-length");
   if(type==0)values.append(QVariant{});
   else if(type==8||type==9)values.append(QString::number(type-8));
   else if(type<=6){
    quint64 bits=0;for(quint64 i=0;i<length;++i)bits=(bits<<8)|uchar(payload[p+qsizetype(i)]);
    if(length<8&&(bits&(quint64(1)<<(length*8-1))))bits|=(~quint64(0))<<(length*8);
    values.append(QString::number(std::bit_cast<qint64>(bits)));
   }else if(type&1){
    QStringDecoder decoder(QStringDecoder::Utf8,QStringConverter::Flag::Stateless);
    const QString text=decoder(payload.mid(p,qsizetype(length)));
    if(decoder.hasError())return fail("sqlite-table-invalid-utf8");
    values.append(text);
   }else values.append(payload.mid(p,qsizetype(length)));
   p+=qsizetype(length);
  }
  return p==payload.size()||fail("sqlite-table-record-trailing-bytes");
 }
 QVariantMap extent(qsizetype offset,qsizetype length) const {return {{"imageByteOffset",offset},{"lengthBytes",length}};}
 bool walk(quint32 page,int depth,qint64 lower,bool hasLower,qint64 upper,bool hasUpper){
  if(depth>maxDepth)return fail("sqlite-table-depth-limit");
  if(!claim(page))return false;
  const qsizetype base=qsizetype(page-1)*pageSize,end=base+usable,h=base+(page==1?100:0);
  if(h+8>end)return fail("sqlite-table-header");
  const int kind=uchar(image[h]),headerSize=kind==5?12:8;
  if(kind!=5&&kind!=13)return fail("sqlite-table-page-type");
  const int count=u16(h+3);const qsizetype pointersEnd=h+headerSize+2*count;
  if(pointersEnd>end)return fail("sqlite-table-cell-pointers");
  const int content=u16(h+5)?u16(h+5):65536;
  if(base+content<pointersEnd||content>usable||uchar(image[h+7])>60)return fail("sqlite-table-cell-content");
  QList<QPair<qsizetype,qsizetype>> ranges;
  qint64 previous=lower;bool hasPrevious=hasLower;
  for(int cell=0;cell<count;++cell){
   if(!active())return false;
   qsizetype p=base+u16(h+headerSize+2*cell),start=p;
   if(p<base+content||p>=end)return fail("sqlite-table-cell-offset");
   quint32 child=0;if(kind==5){if(p+4>end)return fail("sqlite-table-cell-length");child=u32(p);p+=4;}
   quint64 size=0,key=0;
   if(kind==13&&!varint(image,p,end,size))return false;
   if(!varint(image,p,end,key))return false;
   const qint64 rowid=std::bit_cast<qint64>(key);
   if((hasPrevious&&rowid<=previous)||(hasUpper&&rowid>upper))return fail("sqlite-table-key-order");
   if(kind==5){if(!walk(child,depth+1,previous,hasPrevious,rowid,true))return false;}
   else {
    if(rows.size()>=maxRows||size>maxPayload||decoded+qsizetype(size)>8*1024*1024)return fail("sqlite-table-resource-limit");
    decoded+=qsizetype(size);
    const quint64 minimum=quint64((usable-12)*32/255-23),maximum=quint64(usable-35);
    quint64 local=size;
    if(size>maximum){local=minimum+(size-minimum)%quint64(usable-4);if(local>maximum)local=minimum;}
    if(local>quint64(end-p)||(size>local&&p+qsizetype(local)+4>end))return fail("sqlite-table-cell-length");
    QByteArray payload=image.mid(p,qsizetype(local));QVariantList extents{extent(p,qsizetype(local))};p+=qsizetype(local);
    quint32 overflow=0;if(size>local){overflow=u32(p);p+=4;}
    while(quint64(payload.size())<size){
     if(!claim(overflow))return false;
     const qsizetype off=qsizetype(overflow-1)*pageSize;
     const auto take=qMin(qsizetype(size)-payload.size(),qsizetype(usable-4));
     payload+=image.mid(off+4,take);extents.append(extent(off+4,take));overflow=u32(off);
    }
    if(overflow)return fail("sqlite-table-overflow-tail");
    QVariantList values;QList<quint64> types;if(!record(payload,values,types))return false;
    rows.append({QString::number(rowid),values,extents,types,extent(start,p-start)});
   }
   for(const auto &range:ranges)if(start<range.second&&p>range.first)return fail("sqlite-table-overlapping-cells");
   ranges.append({start,p});previous=rowid;hasPrevious=true;
  }
  return kind!=5||walk(u32(h+8),depth+1,previous,hasPrevious,upper,hasUpper);
 }
public:
 Reader(const QByteArray &bytes,const ReadOnlyEvidenceRequest &r):image(bytes),request(r){}
 SqliteTableRead read(quint32 root){
  // Caller supplied only the committed image returned by the family verifier.
  pageSize=u16(16);if(pageSize==1)pageSize=65536;usable=pageSize-uchar(image[20]);
  if(u32(56)!=1)return {{},"sqlite-table-encoding-unsupported"};
  if(!walk(root,0,0,false,0,false))return {{},error};
  if(!active())return {{},error};
  return {rows,{},seen};
 }
};
}
SqliteTableRead readProtectedSqliteTable(const ReadOnlyEvidenceRequest &request,quint32 rootPage){
 const auto family=inspectSqliteEvidenceFamily(request,{});
 if(!family.analysis.ok())return {{},family.analysis.errorCode};
 return Reader(family.committedPages,request).read(rootPage);
}
}
