#include "ExifSummary.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QSet>
#include <QtEndian>

namespace mr::forensic {
namespace {
class TiffReader {
public:
 explicit TiffReader(const QByteArray &bytes):bytes_(bytes) {}
 bool parse() {
  if(!range(0,8))return fail("truncated-tiff-header");
  little_=bytes_.startsWith("II");
  if(!little_&&!bytes_.startsWith("MM"))return fail("invalid-tiff-byte-order");
  if(u16(2)!=42)return fail("classic-tiff-required");
  if(!directory(u32(4),false))return false;
  if(exifOffset_&&!directory(exifOffset_,true))return false;
  return true;
 }
 QJsonObject fields;
 QString error;
 bool gps=false,thumbnail=false;
private:
 bool fail(const char *code){error=QString::fromLatin1(code);return false;}
 bool range(quint64 at,quint64 size)const{return at<=static_cast<quint64>(bytes_.size())&&size<=static_cast<quint64>(bytes_.size())-at;}
 quint16 u16(quint32 at)const{return little_?qFromLittleEndian<quint16>(bytes_.constData()+at):qFromBigEndian<quint16>(bytes_.constData()+at);}
 quint32 u32(quint32 at)const{return little_?qFromLittleEndian<quint32>(bytes_.constData()+at):qFromBigEndian<quint32>(bytes_.constData()+at);}
 bool directory(quint32 offset,bool exif) {
  if(offset<8||!range(offset,2)||visited_.contains(offset))return fail("invalid-or-repeated-ifd");
  visited_.insert(offset);
  const auto count=u16(offset);
  if(count>256||!range(offset,2ULL+12ULL*count+4ULL))return fail("ifd-entry-limit-or-truncation");
  QSet<quint16> tags;
  for(quint32 i=0;i<count;++i) {
   const auto at=offset+2+12*i;
   const auto tag=u16(at),type=u16(at+2);
   const auto n=u32(at+4);
   if(tags.contains(tag))return fail("duplicate-ifd-tag");
   tags.insert(tag);
   if(!exif&&(tag==0x8769||tag==0x8825)) {
    if(type!=4||n!=1)return fail("invalid-ifd-pointer");
    const auto pointer=u32(at+8);
    if(pointer<8||!range(pointer,2)||pointer==offset)return fail("invalid-ifd-pointer");
    if(tag==0x8769)exifOffset_=pointer;else gps=true;
    continue;
   }
   QString key;
   bool numeric=false;
   if(!exif) {
    switch(tag) {
     case 0x010f:key="make";break;
     case 0x0110:key="model";break;
     case 0x0131:key="software";break;
     case 0x0132:key="dateTimeRaw";break;
     case 0x0112:key="orientation";numeric=true;break;
     default:continue;
    }
   }else {
    switch(tag) {
     case 0x9003:key="dateTimeOriginalRaw";break;
     case 0x9004:key="dateTimeDigitizedRaw";break;
     case 0x9010:key="offsetTimeRaw";break;
     case 0x9011:key="offsetTimeOriginalRaw";break;
     case 0x9012:key="offsetTimeDigitizedRaw";break;
     case 0xa002:key="pixelXDimensionDeclared";numeric=true;break;
     case 0xa003:key="pixelYDimensionDeclared";numeric=true;break;
     default:continue;
    }
   }
   if(numeric) {
    if(n!=1||(type!=3&&type!=4)||(tag==0x0112&&type!=3))return fail("invalid-numeric-tag");
    const auto value=type==3?static_cast<quint32>(u16(at+8)):u32(at+8);
    if(tag==0x0112&&(value<1||value>8))return fail("invalid-orientation");
    fields.insert(key,static_cast<double>(value));
   }else {
    if(type!=2||n<1||n>256)return fail("invalid-ascii-tag");
    const auto start=n<=4?at+8:u32(at+8);
    if(start<8||!range(start,n))return fail("ascii-value-outside-app1");
    const auto raw=bytes_.mid(start,n);
    if(raw.back()!='\0')return fail("unterminated-ascii-tag");
    for(qsizetype c=0;c<raw.size()-1;++c) {
     const auto ch=static_cast<unsigned char>(raw[c]);
     if(ch<32||ch>126)return fail("unsupported-ascii-value");
    }
    fields.insert(key,QString::fromLatin1(raw.constData(),raw.size()-1));
   }
  }
  // Deliberately never follow next-IFD/thumbnail pointers.
  if(u32(offset+2+12*count)!=0)thumbnail=true;
  return true;
 }
 const QByteArray &bytes_;
 bool little_=false;
 quint32 exifOffset_=0;
 QSet<quint32> visited_;
};
}
ExifSummary summarizeExif(const QByteArray &bytes,const std::shared_ptr<std::atomic_bool> &cancelled) {
 const auto fail=[](const char *code){return ExifSummary{{},QString::fromLatin1(code)};};
 const auto stopped=[&]{return cancelled&&cancelled->load();};
 if(stopped())return fail("cancelled");
 if(bytes.size()<4||bytes.size()>maximumExifSourceBytes)return fail("jpeg-size-limit");
 if(!bytes.startsWith(QByteArray::fromHex("ffd8")))return fail("jpeg-required");
 QByteArray tiff;
 bool found=false,boundary=false;
 qsizetype cursor=2;
 unsigned segments=0;
 while(cursor<bytes.size()) {
  if(stopped())return fail("cancelled");
  if(++segments>4096)return fail("jpeg-segment-limit");
  if(static_cast<uchar>(bytes[cursor])!=0xff)return fail("invalid-jpeg-marker");
  while(cursor<bytes.size()&&static_cast<uchar>(bytes[cursor])==0xff)++cursor;
  if(cursor>=bytes.size())return fail("truncated-jpeg-marker");
  const auto marker=static_cast<uchar>(bytes[cursor++]);
  if(marker==0xd9){boundary=true;break;}
  if(marker==0x00||marker==0xd8||(marker>=0xd0&&marker<=0xd7))return fail("unexpected-jpeg-marker");
  if(marker==0x01)continue;
  if(bytes.size()-cursor<2)return fail("truncated-jpeg-length");
  const auto length=qFromBigEndian<quint16>(bytes.constData()+cursor);
  if(length<2||length>bytes.size()-cursor)return fail("jpeg-segment-outside-source");
  if(marker==0xe1&&bytes.mid(cursor+2,6)==QByteArray("Exif\0\0",6)) {
   if(found)return fail("multiple-exif-segments");
   if(length<8)return fail("truncated-exif-segment");
   found=true;tiff=bytes.mid(cursor+8,length-8);
  }
  cursor+=length;
  // Entropy-coded data and any later scans/metadata are outside this reader.
  if(marker==0xda){boundary=true;break;}
 }
 if(!boundary)return fail("jpeg-header-boundary-missing");
 TiffReader reader(tiff);
 if(found&&!reader.parse())return {{},reader.error};
 if(stopped())return fail("cancelled");
 QJsonObject report{{"schema","nexvary-exif-summary/v1"},{"capability","forensic.exif.summary.v1"},
  {"format","jpeg-app1-classic-tiff"},{"exifPresent",found},{"fields",reader.fields},
  {"gpsIfdDeclared",reader.gps},{"nextIfdOmitted",reader.thumbnail},{"gpsValuesIncluded",false},
  {"imagePixelsDecoded",false},{"metadataAuthenticated",false},{"timestampsConvertedToUtc",false},
  {"sourceSha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())},
  {"limits",QJsonArray{"Untrusted metadata declarations; no camera identity, capture time or image authenticity validation.",
   "JPEG headers before the first scan only; 8 MiB source, 4096 markers, 256 entries per supported IFD and 256-byte ASCII values.",
   "Allowlisted IFD0/Exif fields only. GPS values, serials, owner names, comments, MakerNotes, thumbnails and unknown tags are omitted.",
   "Dates and offsets remain raw strings, without timezone inference or insertion as capture events in the case timeline.",
   "No full JPEG validation, pixel decoding, standalone TIFF, BigTIFF, HEIC, XMP, extended Exif or complete Exif 3.1 conformance."}}};
 return {report,{}};
}
}
