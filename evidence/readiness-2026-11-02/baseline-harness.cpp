#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "LawCaseStore.hpp"
#include "LawCaseBridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <cstdio>
int main(int argc,char**argv){QCoreApplication app(argc,argv);const QString password="Synthetic bounded benchmark 2026!";QVariantList observations;
 for(const int size:{32,1024*1024,8*1024*1024}){
  QTemporaryDir dir;IdentityService identity(dir.filePath("identity"),true);
  if(!identity.bootstrap("admin","Synthetic admin",password)||!identity.login("admin",password)||identity.createUser("owner","Synthetic investigator","investigator",password).isEmpty()||!identity.login("owner",password))return 1;
  GovernmentOperationPolicy policy(&identity);LawCaseStore store(dir.filePath("cases.db"),true);
  const auto id=store.create({{"caseNumber","Synthetic benchmark"}},identity.actor(),identity.userId());const auto device=store.addDevice(id,1,{{"model","Synthetic"},{"custodian","Synthetic"}},identity.actor());
  QByteArray bytes(size,'x');QFile f(dir.filePath("source"));if(!f.open(QIODevice::WriteOnly)||f.write(bytes)!=bytes.size())return 2;f.close();const auto evidence=store.addEvidence(id,2,device,f.fileName(),"Synthetic","Synthetic",identity.actor());
  const QVariantMap report{{"sourceSha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())},{"fixture",true}};
  auto intent=store.prepareAnalysis(id,3,evidence,"classic-pcap",report,identity.userId(),"investigator");if(!policy.recordCaseOperation(intent)||store.recordAnalysis(id,3,evidence,"classic-pcap",report,identity.actor(),intent).isEmpty())return 3;intent.insert("recoveryState","completed");if(!policy.recordCaseOperation(intent))return 4;
  LawCaseBridge bridge(dir.filePath("cases.db"),nullptr,true);bridge.setOperationPolicy(&policy);QElapsedTimer timer;timer.start();const int iterations=100;
  for(int i=0;i<iterations;++i)if(!bridge.ready())return 5;
  observations.append(QVariantMap{{"sourceBytes",size},{"cases",1},{"analyses",1},{"readinessCalls",iterations},{"elapsedMs",timer.elapsed()}});
 }
 std::puts(QJsonDocument::fromVariant(observations).toJson(QJsonDocument::Compact).constData());}
