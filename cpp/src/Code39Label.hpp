/*
 * Code 39 encoding table derived from ZXing-C++ v2.3.0 ODCode39Writer.cpp.
 * Copyright 2016 Huy Cuong Nguyen; Copyright 2016 ZXing authors.
 * Qt renderer and NEXVARY label restriction: Copyright 2026 NEXVARY.
 * SPDX-License-Identifier: Apache-2.0
 * Full license: packaging/licenses/ZXING-CODE39-APACHE-2.0.txt
 */
#pragma once
#include <QImage>
#include <QPainter>
#include <QRegularExpression>
namespace mr::workshop {
inline bool validBarcode(const QString &code){return QRegularExpression("\\ANMR-[0-9A-F]{16}\\z").match(code).hasMatch();}
inline QImage barcodeImage(const QString &code){
 if(!validBarcode(code))return {};
 const QString alphabet=QStringLiteral("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%");
 constexpr int patterns[]{0x034,0x121,0x061,0x160,0x031,0x130,0x070,0x025,0x124,0x064,
  0x109,0x049,0x148,0x019,0x118,0x058,0x00D,0x10C,0x04C,0x01C,
  0x103,0x043,0x142,0x013,0x112,0x052,0x007,0x106,0x046,0x016,
  0x181,0x0C1,0x1C0,0x091,0x190,0x0D0,0x085,0x184,0x0C4,0x094,0x0A8,0x0A2,0x08A,0x02A};
 const auto encoded=QStringLiteral("*")+code+QStringLiteral("*");
 constexpr int scale=3,quiet=12;
 const int width=(2*quiet+static_cast<int>(encoded.size())*15+static_cast<int>(encoded.size())-1)*scale;
 QImage image(width,120,QImage::Format_RGB32);image.fill(Qt::white);
 QPainter painter(&image);painter.setRenderHint(QPainter::Antialiasing,false);int x=quiet*scale;
 for(qsizetype n=0;n<encoded.size();++n){const int index=static_cast<int>(alphabet.indexOf(encoded[n]));if(index<0||index>=44)return {};
  for(int element=0;element<9;++element){const int pixels=((patterns[index]&(1<<(8-element)))?3:1)*scale;if(element%2==0)painter.fillRect(x,8,pixels,104,Qt::black);x+=pixels;}
  if(n+1<encoded.size())x+=scale;
 }
 return image;
}
}
