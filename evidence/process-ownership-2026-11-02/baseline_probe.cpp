#include "TskImage.hpp"
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QElapsedTimer>
#include <cstdio>
#include <thread>
static bool write(const QString &p,const QByteArray &s){QFile f(p);return f.open(QIODevice::WriteOnly)&&f.write(s)==s.size();}
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);const auto args=app.arguments();
 if(args.value(1)=="--detached"){
  const auto root=args.value(2);write(root+"/ready","marker-only synthetic fixture");QElapsedTimer t;t.start();
  while(!QFile::exists(root+"/cancelled")&&t.elapsed()<10000)QThread::msleep(20);
  return write(root+"/survived","detached child ran after analyzer returned")?0:2;
 }
 if(args.value(1)=="-V"){
  if(!QProcess::startDetached(app.applicationFilePath(),{"--detached",qEnvironmentVariable("MR_PROBE_ROOT")}))return 3;
  QThread::msleep(10000);return 0;
 }
 QTemporaryDir dir;if(!dir.isValid()||!write(dir.filePath("fixture.img"),QByteArray(4096,'x')))return 4;
 qputenv("MR_PROBE_ROOT",dir.path().toUtf8());auto token=std::make_shared<std::atomic_bool>(false);
 std::jthread cancel([&]{QElapsedTimer t;t.start();while(!QFile::exists(dir.filePath("ready"))&&t.elapsed()<5000)QThread::msleep(20);token->store(true);});
 const auto result=mr::forensic::analyzePartitions(dir.filePath("fixture.img"),app.applicationFilePath(),token);
 write(dir.filePath("cancelled"),"worker returned");QElapsedTimer t;t.start();while(!QFile::exists(dir.filePath("survived"))&&t.elapsed()<2000)QThread::msleep(20);
 const bool survived=QFile::exists(dir.filePath("survived"));std::printf("cancelled=%d error=%s detachedSurvived=%d\n",result.cancelled,qPrintable(result.error),survived);
 return survived?0:1;
}
