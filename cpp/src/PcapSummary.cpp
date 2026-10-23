#include "PcapSummary.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QtEndian>
#include <algorithm>
#include <limits>

namespace mr::forensic {
PcapSummary summarizeCapture(const QByteArray &bytes,const std::shared_ptr<std::atomic_bool> &cancelled) {
 const auto fail=[](const char *code){return PcapSummary{{},QString::fromLatin1(code)};};
 const auto stopped=[&]{return cancelled&&cancelled->load();};
 if(stopped())return fail("cancelled");
 if(bytes.size()<24||bytes.size()>maximumCaptureBytes)return fail("capture-size-limit");
 const auto magic=bytes.left(4).toHex();
 const bool little=magic=="d4c3b2a1"||magic=="4d3cb2a1";
 const bool nano=magic=="a1b23c4d"||magic=="4d3cb2a1";
 if(!little&&magic!="a1b2c3d4"&&magic!="a1b23c4d")return fail("classic-pcap-required");
 const auto u16=[&](qsizetype p){return little?qFromLittleEndian<quint16>(bytes.constData()+p):qFromBigEndian<quint16>(bytes.constData()+p);};
 const auto u32=[&](qsizetype p){return little?qFromLittleEndian<quint32>(bytes.constData()+p):qFromBigEndian<quint32>(bytes.constData()+p);};
 if(u16(4)!=2||u16(6)!=4)return fail("unsupported-pcap-version");
 const quint32 snaplen=u32(16),link=u32(20);
 if(snaplen==0||snaplen>static_cast<quint32>(maximumCaptureBytes))return fail("snapshot-limit");
 quint64 count=0,captured=0,original=0,truncated=0,outOfOrder=0;
 quint64 first=std::numeric_limits<quint64>::max(),last=0,previous=0;
 quint64 ipv4=0,ipv6=0,tcp=0,udp=0,icmp=0,other=0,unparsed=0,incomplete=0;
 auto protocol=[&](unsigned value){if(value==6)++tcp;else if(value==17)++udp;else if(value==1||value==58)++icmp;else ++other;};
 qsizetype cursor=24;
 while(cursor<bytes.size()) {
  if(stopped())return fail("cancelled");
  if(count>=200000)return fail("record-count-limit");
  if(bytes.size()-cursor<16)return fail("truncated-record-header");
  const quint32 seconds=u32(cursor),fraction=u32(cursor+4),cap=u32(cursor+8),wire=u32(cursor+12);
  if(fraction>=(nano?1000000000U:1000000U))return fail("invalid-timestamp-fraction");
  // Deliberately stricter than permissive legacy PCAP readers; report the scope.
  if(cap>snaplen||cap>wire)return fail("inconsistent-record-length");
  cursor+=16;
  if(cap>static_cast<quint64>(bytes.size()-cursor))return fail("truncated-record-payload");
  const quint64 stamp=static_cast<quint64>(seconds)*1000000000ULL+static_cast<quint64>(fraction)*(nano?1ULL:1000ULL);
  if(count&&stamp<previous)++outOfOrder;
  previous=stamp;first=std::min(first,stamp);last=std::max(last,stamp);
  ++count;captured+=cap;original+=wire;if(cap<wire)++truncated;
  if(link==1) {
   // Ethernet II with up to two VLAN tags. Counts are header declarations,
   // never transport validation, reconstructed flows or application inference.
   const auto *packet=reinterpret_cast<const uchar *>(bytes.constData()+cursor);
   const auto be16=[&](quint32 at){return qFromBigEndian<quint16>(packet+at);};
   quint32 offset=14;
   if(cap<14){++unparsed;}
   else {
    quint16 type=be16(12);unsigned tags=0;
    while((type==0x8100||type==0x88a8)&&tags<2&&offset<=cap&&cap-offset>=4){type=be16(offset+2);offset+=4;++tags;}
    const quint32 available=cap-offset;
    if(type==0x0800) {
     if(available<20||(packet[offset]>>4)!=4){++unparsed;}
     else {
      const quint32 header=(packet[offset]&15U)*4U,total=be16(offset+2);
      if(header<20||header>available||total<header){++unparsed;}
      else {++ipv4;protocol(packet[offset+9]);if(total>available)++incomplete;}
     }
    }else if(type==0x86dd) {
     if(available<40||(packet[offset]>>4)!=6){++unparsed;}
     else {++ipv6;protocol(packet[offset+6]);if(static_cast<quint32>(be16(offset+4))+40>available)++incomplete;}
    }else {++unparsed;}
   }
  }
  cursor+=static_cast<qsizetype>(cap);
 }
 if(stopped())return fail("cancelled");
 QJsonObject report{{"schema","nexvary-pcap-summary/v1"},{"capability","forensic.pcap.summary.v1"},{"format","classic-pcap-2.4"},
  {"byteOrder",little?"little":"big"},{"timestampResolution",nano?"nanoseconds":"microseconds"},{"linkLayerInfo",static_cast<double>(link)},
  {"snapshotLength",static_cast<double>(snaplen)},{"packetCount",QString::number(count)},{"capturedBytes",QString::number(captured)},
  {"declaredOriginalBytes",QString::number(original)},{"truncatedPackets",QString::number(truncated)},{"outOfOrderTimestamps",QString::number(outOfOrder)},
  {"minimumTimestampNs",count?QJsonValue(QString::number(first)):QJsonValue(QJsonValue::Null)},
  {"maximumTimestampNs",count?QJsonValue(QString::number(last)):QJsonValue(QJsonValue::Null)},
  {"ethernetHeaderStatisticsAvailable",link==1},{"packetPayloadIncluded",false},{"addressesIncluded",false},{"decryptionPerformed",false},
  {"headerCounts",QJsonObject{{"ipv4",QString::number(ipv4)},{"ipv6",QString::number(ipv6)},{"tcpProtocol",QString::number(tcp)},
   {"udpProtocol",QString::number(udp)},{"icmpProtocol",QString::number(icmp)},{"otherIpProtocol",QString::number(other)},
   {"unparsedEthernetPackets",QString::number(unparsed)},{"incompleteIpPackets",QString::number(incomplete)}}},
  {"limits",QJsonArray{"Untrusted capture declarations; timestamps and original lengths are not authenticated.",
   "Classic PCAP only; 8 MiB and 200000 records; strict captured<=original length policy.",
   "Ethernet II and up to two VLAN tags only; other link encodings have container statistics only.",
   "IP protocol fields only; no checksum, transport, extension-chain, flow or application validation.",
   "No PCAPNG, live capture, endpoint addresses, payload extraction or TLS decryption."}}};
 report.insert("sourceSha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex()));
 if(stopped())return fail("cancelled");
 return {report,{}};
}
}
