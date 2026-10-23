#include "PcapSummary.hpp"
#include <QRandomGenerator>
#include <QtEndian>
#include <cstdio>
int main(){
 QRandomGenerator random(0x50434150U);QByteArray seed(24+16+64,'\0');
 qToLittleEndian<quint32>(0xa1b2c3d4U,seed.data());qToLittleEndian<quint16>(2,seed.data()+4);qToLittleEndian<quint16>(4,seed.data()+6);
 qToLittleEndian<quint32>(65535,seed.data()+16);qToLittleEndian<quint32>(1,seed.data()+20);qToLittleEndian<quint32>(64,seed.data()+32);qToLittleEndian<quint32>(64,seed.data()+36);
 if(!mr::forensic::summarizeCapture(seed).error.isEmpty())return 1;
 for(int i=0;i<30000;++i){auto b=seed;for(unsigned j=0,n=1+random.bounded(8U);j<n;++j)b[static_cast<qsizetype>(random.bounded(104U))]=static_cast<char>(random.bounded(256U));
  if(i%3==0)b.truncate(static_cast<qsizetype>(random.bounded(105U)));
  const auto r=mr::forensic::summarizeCapture(b);
  if(!r.error.isEmpty()){if(!r.report.isEmpty())return 2;}
  else for(const auto *field:{"packetPayloadIncluded","addressesIncluded","decryptionPerformed"})if(!r.report[field].isBool()||r.report[field].toBool(true))return 3;
 }
 std::puts("30000 deterministic PCAP mutations passed; no partial reports or payload/address output.");return 0;
}
