#pragma once
#include <QFont>
#include <QGlyphRun>
#include <QTextLayout>
namespace mr::ui {
// Exercise Qt's actual shaping and font fallback, rather than accepting a PNG
// filled with missing-glyph boxes as visual validation.
inline bool textHasGlyphs(const QString &text,const QFont &font){
 if(text.isEmpty())return false;
 QTextLayout layout(text,font);layout.beginLayout();
 while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(2000);}
 layout.endLayout();bool any=false;
 for(const auto &run:layout.glyphRuns()){
  if(!run.rawFont().isValid())return false;
  for(const auto index:run.glyphIndexes()){if(index==0)return false;any=true;}
 }
 return any;
}
}
