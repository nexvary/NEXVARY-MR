#include "OwnedProcess.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdlib>
#ifdef Q_OS_LINUX
#include <signal.h>
#include <unistd.h>
#endif
namespace {
bool write(const QString &p,const QByteArray &bytes){QFile f(p);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();}
bool waitFile(const QString &p,int ms=5000){QElapsedTimer t;t.start();while(!QFile::exists(p)&&t.elapsed()<ms)QThread::msleep(10);return QFile::exists(p);}
int fixture(const QStringList &args){
 const auto root=args.value(1);
 if(args.value(0)=="--leaf"){
  if(!write(root+"/ready","synthetic marker-only detached child"))return 80;
  if(waitFile(root+"/release",10000))return write(root+"/survived","synthetic child responded after cleanup")?0:81;
  return 82;
 }
 if(args.value(0)=="--child"){
  if(!QProcess::startDetached(QCoreApplication::applicationFilePath(),{"--leaf",root}))return 83;
  if(!waitFile(root+"/ready"))return 84;
#ifdef Q_OS_LINUX
  if(args.value(2)=="kill-supervisor"){kill(getppid(),SIGKILL);return 0;}
#endif
  if(args.value(2)=="exit")return 0;
  QThread::msleep(12000);return 0;
 }
 if(args.value(0)=="--owner"){
  OwnedProcess worker;worker.start(QCoreApplication::applicationFilePath(),{"--child",root,"hang"});
  if(!worker.waitForStarted(3000)||!waitFile(root+"/ready"))return 85;
  write(root+"/owner-ready","safe to crash synthetic owner");
  QThread::msleep(12000);return 0;
 }
 return 86;
}
}
class OwnedProcessTests final:public QObject {
 Q_OBJECT
private slots:
 void directKillNegativeIsRetained(){
  QTemporaryDir dir;QVERIFY(dir.isValid());QProcess raw;
  raw.start(QCoreApplication::applicationFilePath(),{"--child",dir.path(),"hang"});QVERIFY(raw.waitForStarted(3000));QVERIFY(waitFile(dir.filePath("ready")));
  raw.kill();QVERIFY(raw.waitForFinished(3000));QVERIFY(write(dir.filePath("release"),"raw parent is gone"));
  QVERIFY2(waitFile(dir.filePath("survived")),"Uncontained negative must still demonstrate the original detached escape.");
 }
 void shutdown_data(){
  QTest::addColumn<QString>("reason");QTest::addColumn<bool>("rootExits");
  for(const auto &reason:{"cancel","logout","session-expiry","permission-revocation","worker-timeout"})QTest::newRow(reason)<<QString::fromLatin1(reason)<<false;
  QTest::newRow("root-exit-with-detached-child")<<QString("worker-exit")<<true;
 }
 void shutdown(){
  QFETCH(QString,reason);QFETCH(bool,rootExits);QTemporaryDir dir;QVERIFY(dir.isValid());OwnedProcess owned;
  owned.start(QCoreApplication::applicationFilePath(),{"--child",dir.path(),rootExits?"exit":"hang"});QVERIFY(owned.waitForStarted(3000));QVERIFY(waitFile(dir.filePath("ready")));
  QElapsedTimer elapsed;elapsed.start();if(!rootExits)owned.stop(reason);
  QVERIFY(owned.waitForFinished(3500));QVERIFY(elapsed.elapsed()<3500);
  QVERIFY2(owned.cleanupComplete(),QJsonDocument::fromVariant(owned.cancellationReceipt()).toJson().constData());
  if(!rootExits)QCOMPARE(owned.cancellationReceipt().value("reason").toString(),reason);
  QVERIFY(write(dir.filePath("release"),"owned worker returned"));QTest::qWait(150);QVERIFY(!QFile::exists(dir.filePath("survived")));
 }
 void parentCrash(){
  QTemporaryDir dir;QVERIFY(dir.isValid());QProcess owner;
  owner.start(QCoreApplication::applicationFilePath(),{"--owner",dir.path()});QVERIFY(owner.waitForStarted(3000));QVERIFY(waitFile(dir.filePath("owner-ready")));
  owner.kill();QVERIFY(owner.waitForFinished(3000));
  // No fresh in-product receipt after SIGKILL; use an independent marker.
  QTest::qWait(2500);QVERIFY(write(dir.filePath("release"),"product parent crashed"));QTest::qWait(200);QVERIFY(!QFile::exists(dir.filePath("survived")));
 }
 void hostileSameUserSupervisorKillRemainsUnknown(){
#ifdef Q_OS_LINUX
  QTemporaryDir dir;OwnedProcess owned;
  owned.start(QCoreApplication::applicationFilePath(),{"--child",dir.path(),"kill-supervisor"});QVERIFY(owned.waitForStarted(3000));QVERIFY(owned.waitForFinished(3500));
  QVERIFY(!owned.cleanupComplete());
  QVERIFY(write(dir.filePath("release"),"supervisor was killed by same-user worker"));
  QVERIFY(waitFile(dir.filePath("survived"))); // Explicit retained escape limit.
#else
  QSKIP("Linux hostile supervisor-kill boundary; Windows Job lifecycle is separate.");
#endif
 }
 void repeatedUseAndLaunchFailure(){
  OwnedProcess owned;
  for(int n=0;n<3;++n){owned.start(QCoreApplication::applicationFilePath(),{"--child",QDir::tempPath()+"/absent-nexvary-fixture","exit"});QVERIFY(owned.waitForStarted(3000));QVERIFY(owned.waitForFinished(6000));QVERIFY(owned.cleanupComplete());QVERIFY(owned.exitCode()!=0);}
  owned.start(QDir::tempPath()+"/nonexistent-nexvary-program",{});
  if(owned.waitForStarted(3000)){QVERIFY(owned.waitForFinished(3500));QVERIFY(owned.exitCode()!=0);}else QVERIFY(!owned.cleanupComplete());
 }
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);const auto args=app.arguments().mid(1);if(args.value(0).startsWith("--"))return fixture(args);OwnedProcessTests tests;return QTest::qExec(&tests,argc,argv);}
#include "owned_process_tests.moc"
