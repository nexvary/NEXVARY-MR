// Manual synthetic product-service measurements. No timing assertions or device I/O.
#include "LawCaseBridge.hpp"
#include "IdentityService.hpp"
#include "GovernmentOperationPolicy.hpp"
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <cstdio>
#ifdef Q_OS_LINUX
#include <sys/resource.h>
#endif
namespace {
qint64 peakRssKiB(){
#ifdef Q_OS_LINUX
 rusage usage{};return getrusage(RUSAGE_SELF,&usage)==0?usage.ru_maxrss:-1;
#else
 return -1;
#endif
}
QVariantMap metadata(int n){return {{"caseNumber",QString("SYNTHETIC-BENCH-%1").arg(n)},{"investigatingAgency","Synthetic lab"},{"investigatorName","Synthetic operator"},{"seizedDeviceDescription","Synthetic JSON"},{"authorityReference","Synthetic benchmark only"},{"issuingAuthority","Fixture owner"},{"scope","Synthetic local measurements"}};}
template<class Start,class Busy> qint64 asynchronous(Start start,Busy busy){
 QElapsedTimer timer;timer.start();if(!start())return -1;
 while(busy()&&timer.elapsed()<15000){QCoreApplication::processEvents(QEventLoop::AllEvents,5);QThread::msleep(1);}
 return busy()?-1:timer.nsecsElapsed()/1000;
}
}
int main(int argc,char **argv){
 QGuiApplication app(argc,argv);QVariantList observations;
 const QString password="Synthetic workload benchmark 2026!";
 for(int objects:{4,16,32})for(int repeat=0;repeat<3;++repeat){
  QTemporaryDir dir;if(!dir.isValid())return 1;
  IdentityService identity(dir.filePath("identity"),true);
  if(!identity.bootstrap("admin","Synthetic administrator",password)||!identity.login("admin",password)||identity.createUser("owner","Synthetic investigator","investigator",password).isEmpty()||!identity.login("owner",password))return 2;
  GovernmentOperationPolicy policy(&identity);
  const auto database=dir.filePath("cases.db");
  auto bridge=std::make_unique<LawCaseBridge>(database,nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);
  bridge->setOperationPolicy(&policy);
  QElapsedTimer tick;tick.start();qint64 maxGapUs=0;int heartbeatTicks=0;
  QTimer heartbeat;heartbeat.setInterval(10);
  QObject::connect(&heartbeat,&QTimer::timeout,&app,[&]{maxGapUs=qMax(maxGapUs,tick.nsecsElapsed()/1000);tick.restart();++heartbeatTicks;});heartbeat.start();
  const auto id=bridge->createCase(metadata(objects));if(id.isEmpty())return 3;
  const auto device=bridge->addDevice({{"model","Synthetic JSON"},{"custodian","Synthetic custodian"}});if(device.isEmpty())return 4;
  QVariantList records;for(int n=0;n<objects;++n)records.append(QVariantMap{{"name",QString("اصطناعي SYNTHETIC %1").arg(n)},{"time","2026-11-02T12:00:00Z"},{"payload",QString(1024,'x')}});
  const auto source=QJsonDocument::fromVariant(records).toJson(QJsonDocument::Compact);
  QFile file(dir.filePath("source.json"));if(!file.open(QIODevice::WriteOnly)||file.write(source)!=source.size())return 5;file.close();
  QElapsedTimer timer;timer.start();const auto evidence=bridge->addEvidence(device,file.fileName(),"Synthetic JSON workload","Synthetic custodian",true);const auto registrationUs=timer.nsecsElapsed()/1000;if(evidence.isEmpty())return 6;
  const auto importUs=asynchronous([&]{return bridge->importEvidence(evidence,"json","Synthetic workload",true);},[&]{return bridge->importBusy();});
  if(importUs<0||bridge->importResult().isEmpty())return 7;
  const auto report=QJsonDocument::fromJson(bridge->importResult().value("reportJson").toString().toUtf8()).object().toVariantMap();
  if(report.value("status")!="imported")return 8;
  timer.restart();if(!bridge->openCase(id))return 9;const auto loadingUs=timer.nsecsElapsed()/1000;
  const auto indexUs=asynchronous([&]{return bridge->buildArtifacts();},[&]{return bridge->artifactsBusy();});if(indexUs<0||bridge->artifactsResult().isEmpty())return 10;
  const auto indexRows=bridge->artifactsResult().value("rows").toList().size();
  const auto timelineUs=asynchronous([&]{return bridge->buildTimeline("2026-11-02");},[&]{return bridge->timelineBusy();});if(timelineUs<0||bridge->timelineResult().isEmpty())return 11;
  const auto timelineRows=bridge->timelineResult().value("events").toList().size();
  timer.restart();if(!bridge->exportCasePdf(dir.filePath("report.pdf"),repeat%2==0,true))return 12;const auto pdfUs=timer.nsecsElapsed()/1000;
  // Cancellation includes original native launch and callback refusal; this is
  // an immediate-cancel workload, not worst-case latency on a large corpus.
  const auto cancelUs=asynchronous([&]{if(!bridge->importEvidence(evidence,"json","Synthetic immediate cancellation",true))return false;bridge->cancelImport();return true;},[&]{return bridge->importBusy();});
  if(cancelUs<0)return 13;
  const auto cancelReport=QJsonDocument::fromJson(bridge->importResult().value("reportJson").toString().toUtf8()).object().toVariantMap();
  if(cancelReport.value("status")=="imported")return 14;
  QCoreApplication::processEvents();heartbeat.stop();
  bridge.reset();timer.restart();
  bridge=std::make_unique<LawCaseBridge>(database,nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity);bridge->setOperationPolicy(&policy);
  if(!bridge->ready()||!bridge->openCase(id))return 15;
  const auto reopenUs=timer.nsecsElapsed()/1000;
  observations.append(QVariantMap{{"objects",objects},{"sourceBytes",source.size()},{"repeat",repeat},{"cases",1},{"evidence",1},{"importedFindings",report.value("findings").toList().size()},{"indexRows",indexRows},{"timelineRows",timelineRows},{"protectedEvidenceRegistrationUs",registrationUs},{"authorizedImportUs",importUs},{"warmOpenCaseUs",loadingUs},{"findingIndexUs",indexUs},{"timelineLiteralSearchUs",timelineUs},{"pdfUs",pdfUs},{"pdfBytes",QFileInfo(dir.filePath("report.pdf")).size()},{"immediateImportCancellationUs",cancelUs},{"cleanReopenRecoveryUs",reopenUs},{"eventLoopMaxGapUs",maxGapUs},{"heartbeatTicks",heartbeatTicks},{"processLifetimePeakRssKiB",peakRssKiB()}});
 }
 const QVariantMap output{{"schema","nexvary-case-workload-benchmark/v1"},{"os",QSysInfo::prettyProductName()},{"architecture",QSysInfo::currentCpuArchitecture()},{"qt",qVersion()},{"logicalCpuCount",QThread::idealThreadCount()},{"repetitionsPerSize",3},{"asyncDeadlineMs",15000},{"cache","Fresh temporary case/identity per repeat; warm OS cache, no disk-cache flush"},{"scope","Synthetic protected services with real local identity/policy checks. GUI-thread heartbeat is not rendered-frame latency. Clean reopen is not crash/power-loss recovery. Peak RSS is cumulative parent process high-water, excludes child workers; no memory cap or large-case support claimed."},{"observations",observations}};
 std::puts(QJsonDocument::fromVariant(output).toJson(QJsonDocument::Compact).constData());return 0;
}
