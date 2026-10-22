#pragma once
#include <QFile>
#include <QtEndian>
#include <QSet>

namespace mr::service {
// Structural ZIP/APK validation only: does not certify signatures, payload CRC,
// firmware compatibility, or recovery acceptance. No archive extraction occurs.
inline bool validPackageStructure(QFile &file, bool apk) {
    auto u16=[](const QByteArray &b,qsizetype p){return qFromLittleEndian<quint16>(b.constData()+p);};
    auto u32=[](const QByteArray &b,qsizetype p){return qFromLittleEndian<quint32>(b.constData()+p);};
    if(file.size()<22||!file.seek(qMax<qint64>(0,file.size()-65557)))return false;
    const auto tail=file.read(65557);qsizetype end=-1;
    for(qsizetype p=tail.size()-22;p>=0;--p)
        if(tail.mid(p,4)==QByteArray("PK\005\006",4)&&p+22+u16(tail,p+20)==tail.size()){end=p;break;}
    if(end<0||u16(tail,end+4)!=0||u16(tail,end+6)!=0)return false;
    const auto entries=u16(tail,end+10);
    if(entries==0||entries==65535||u16(tail,end+8)!=entries)return false;
    const auto bytes=u32(tail,end+12),offset=u32(tail,end+16);
    const qint64 endOffset=file.size()-tail.size()+end;
    if(bytes>8*1024*1024||qint64(offset)+bytes!=endOffset||!file.seek(offset))return false;
    const auto directory=file.read(bytes);if(directory.size()!=bytes)return false;
    qsizetype pos=0;QSet<QByteArray> names;bool manifest=false;
    for(quint16 i=0;i<entries;++i) {
        if(pos+46>directory.size()||directory.mid(pos,4)!=QByteArray("PK\001\002",4))return false;
        const auto flags=u16(directory,pos+8),method=u16(directory,pos+10);
        const auto compressed=u32(directory,pos+20),uncompressed=u32(directory,pos+24),local=u32(directory,pos+42);
        const auto length=u16(directory,pos+28),extra=u16(directory,pos+30),comment=u16(directory,pos+32);
        if((flags&1)||method>8||(method!=0&&method!=8)||!length||length>4096||
            uncompressed==0xffffffffu||compressed==0xffffffffu||u16(directory,pos+34)!=0||
            pos+46+length+extra+comment>directory.size()||qint64(local)+30>offset)return false;
        const auto name=directory.mid(pos+46,length);
        if(name.contains('\0')||name.contains('\\')||name.startsWith('/')||names.contains(name)||name.split('/').contains(".."))return false;
        names.insert(name);manifest=manifest||(name=="AndroidManifest.xml"&&uncompressed>0);
        if(!file.seek(local))return false;
        const auto header=file.read(30);
        if(header.size()!=30||header.left(4)!=QByteArray("PK\003\004",4)||u16(header,6)!=flags||
            u16(header,8)!=method||u16(header,26)!=length||
            qint64(local)+30+length+u16(header,28)+compressed>offset||file.read(length)!=name)return false;
        if(!(flags&8)&&(u32(header,14)!=u32(directory,pos+16)||u32(header,18)!=compressed||u32(header,22)!=uncompressed))return false;
        pos+=46+length+extra+comment;
    }
    return pos==directory.size()&&(!apk||manifest)&&file.seek(0);
}
}
