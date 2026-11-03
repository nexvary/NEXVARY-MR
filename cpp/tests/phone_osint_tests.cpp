#include "PhoneOsint.hpp"
#include <QtTest>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <thread>
#include <chrono>
using namespace mr::osint;
class PhoneOsintTests:public QObject {
 Q_OBJECT
private:
 QString engine()const{return QString::fromLocal8Bit(qgetenv("MR_OSINT_MOCK"));}
 Request input(QStringList scanners={"local"})const{QFile f(engine());if(!f.open(QIODevice::ReadOnly))return {};return {"+12025550100",engine(),QString::fromLatin1(QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex()),scanners};}
private slots:
 void normalization(){
  QCOMPARE(normalize("٠٠٢٠ ١٠١٢٣٤٥٦٧٨"),QString("+201012345678"));
  QCOMPARE(normalize("+1 (202) 555-0100"),QString("+12025550100"));
  for(const auto &bad:{"01012345678","+012345678","+12025550100,+12025550101","+12025550100\n","+12025550100;whoami","++12025550100","+1234567890123456"})QVERIFY(normalize(bad).isEmpty());
 }
 void classification(){
  QCOMPARE(classifyResponse("local",200,"{\"result\":{\"country\":\"example\"}}").value("classification").toString(),QString("numbering-plan-metadata"));
  QCOMPARE(classifyResponse("googlesearch",200,"{\"result\":{}}").value("classification").toString(),QString("generated-search-links"));
  QCOMPARE(classifyResponse("ovh",404,"{}").value("status").toString(),QString("unavailable"));
  for(const auto &body:{QByteArray("broken"),QByteArray("{\"success\":false,\"result\":{}}"),QByteArray("{\"result\":null}"),QByteArray("{}")})QCOMPARE(classifyResponse("local",200,body).value("status").toString(),QString("failed"));
 }
 void disabledCommercialProvider(){OsintIndustries p;QVERIFY(p.run({},std::make_shared<std::atomic_bool>(false)).contains("error"));}
 void rejectBeforeLaunch(){PhoneInfoga p;auto r=input();r.executableSha256=QString(64,'0');auto c=std::make_shared<std::atomic_bool>(false);QCOMPARE(p.run(r,c).value("error").toString(),QString("engine-digest-mismatch"));r=input({"local","local"});QVERIFY(p.run(r,c).contains("error"));c->store(true);QCOMPARE(p.run(input(),c).value("error").toString(),QString("cancelled"));}
 void mockLifecycle(){
  QVERIFY2(QFileInfo::exists(engine()),"MR_OSINT_MOCK fixture is required");PhoneInfoga p;
  const auto result=p.run(input({"local","googlesearch","googlecse","numverify"}),std::make_shared<std::atomic_bool>(false));
  QVERIFY2(!result.contains("error"),qPrintable(result.value("error").toString()));
  const auto rows=result.value("scanners").toList();QCOMPARE(rows.size(),4);
  QCOMPARE(rows[0].toMap().value("status").toString(),QString("completed"));QCOMPARE(rows[1].toMap().value("status").toString(),QString("unavailable"));QCOMPARE(rows[2].toMap().value("status").toString(),QString("failed"));QCOMPARE(rows[3].toMap().value("error").toString(),QString("response-too-large"));
  QVERIFY(result.value("processCleanup").toMap().value("cleanupComplete").toBool());
  QVERIFY(!rows[0].toMap().value("verifiedIdentity").toBool());QVERIFY(!rows[0].toMap().value("retrievedUtc").toString().isEmpty());
 }
 void cancellation(){PhoneInfoga p;auto cancel=std::make_shared<std::atomic_bool>(false);std::thread t([cancel]{std::this_thread::sleep_for(std::chrono::milliseconds(500));cancel->store(true);});const auto result=p.run(input({"ovh"}),cancel);t.join();QVERIFY(result.contains("error"));QVERIFY(result.value("processCleanup").toMap().value("cleanupComplete").toBool());}
};
QTEST_GUILESS_MAIN(PhoneOsintTests)
#include "phone_osint_tests.moc"
