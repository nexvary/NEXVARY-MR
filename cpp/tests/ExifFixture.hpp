#pragma once
#include <QByteArray>
#include <QtEndian>
inline QByteArray syntheticExifJpeg(bool little=true) {
 QByteArray tiff(128,'\0');tiff.replace(0,2,little?"II":"MM");
 const auto p16=[&](qsizetype at,quint16 n){if(little)qToLittleEndian(n,tiff.data()+at);else qToBigEndian(n,tiff.data()+at);};
 const auto p32=[&](qsizetype at,quint32 n){if(little)qToLittleEndian(n,tiff.data()+at);else qToBigEndian(n,tiff.data()+at);};
 p16(2,42);p32(4,8);p16(8,3);
 // IFD0: manufacturer, orientation, Exif pointer. Endian-specific inline SHORT.
 p16(10,0x010f);p16(12,2);p32(14,10);p32(18,50);
 p16(22,0x0112);p16(24,3);p32(26,1);p16(30,6);
 p16(34,0x8769);p16(36,4);p32(38,1);p32(42,64);
 tiff.replace(50,10,QByteArray("Synthetic\0",10));
 p16(64,1);p16(66,0x9003);p16(68,2);p32(70,20);p32(74,90);
 tiff.replace(90,20,QByteArray("2026:10:23 12:34:56\0",20));
 QByteArray segment("Exif\0\0",6);segment+=tiff;
 QByteArray jpeg=QByteArray::fromHex("ffd8ffe1");QByteArray size(2,'\0');qToBigEndian<quint16>(static_cast<quint16>(segment.size()+2),size.data());
 return jpeg+size+segment+QByteArray::fromHex("ffd9");
}
