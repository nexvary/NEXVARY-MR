#include "ExifSummary.hpp"
#include "ExifFixture.hpp"
#include <QCoreApplication>
#include <QJsonDocument>
#include <cstdio>
int main(int argc,char **argv) {
 QCoreApplication app(argc,argv);
 const auto original=syntheticExifJpeg();
 quint32 random=0x34ab9281;
 for(int iteration=0;iteration<12000;++iteration) {
  auto bytes=original;
  const auto next=[&]{random^=random<<13;random^=random>>17;random^=random<<5;return random;};
  for(quint32 change=0,limit=1+next()%8;change<limit;++change)bytes[static_cast<qsizetype>(next()%static_cast<quint32>(bytes.size()))]=static_cast<char>(next()&255);
  if(iteration%3==0)bytes.truncate(static_cast<qsizetype>(next()%static_cast<quint32>(bytes.size())));
  const auto result=mr::forensic::summarizeExif(bytes);
  if((!result.error.isEmpty()&&!result.report.isEmpty())||QJsonDocument(result.report).toJson().size()>8192) {
   std::fputs("Invalid mutation result contract\n",stderr);return 1;
  }
 }
 std::puts("12000 deterministic bounded Exif mutations completed");return 0;
}
