#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include <QtTest>
#include <QCryptographicHash>
#include <QFile>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
namespace {
const QString password="Synthetic OSINT fixture 2026!";
struct Fixture {
 QTemporaryDir t;
 IdentityService identity{t.path()+"/identity",true};
 GovernmentOperationPolicy policy{&identity};
 LawCaseBridge law{t.filePath("cases.db"),nullptr,true,nexvary::law::Role::Investigator,nullptr,&identity};
 QString id,device,request;
 Fixture(){
  if(!identity.bootstrap("admin","Synthetic admin",password)||!identity.login("admin",password)||identity.createUser("investigator","Synthetic investigator","investigator",password).isEmpty()||identity.createUser("reviewer","Synthetic reviewer","reviewer",password).isEmpty()||!identity.login("investigator",password))qFatal("OSINT identity fixture failed");
  law.setOperationPolicy(&policy);
  id=law.createCase({{"caseNumber","Synthetic OSINT"},{"investigatingAgency","Synthetic agency"},{"investigatorName","Synthetic investigator"},{"seizedDeviceDescription","Synthetic device"},{"authorityReference","Synthetic authority"},{"issuingAuthority","Fixture owner"},{"scope","Synthetic phone numbers only"},{"jurisdiction","Test"}});
  device=law.addDevice({{"model","Synthetic fixture"},{"custodian","Synthetic custodian"}});
  if(id.isEmpty()||device.isEmpty())qFatal("OSINT case fixture failed");
 }
 bool approve(const QString &caseId,const QString &reason="Synthetic OSINT +12025550100"){
  request=policy.requestException("external-analysis",reason,caseId,60,true);
  return !request.isEmpty()&&identity.login("reviewer",password)&&policy.reviewException(request,"Independent synthetic review")&&policy.decideException(request,true,"Independent synthetic approval")&&identity.login("investigator",password)&&policy.activateException(request)&&law.openCase(id);
 }
 bool run(QStringList scanners={"local"}){
  const QString engine=QString::fromUtf8(MR_OSINT_MOCK_PATH);QFile f(engine);if(!f.open(QIODevice::ReadOnly))return false;
  const auto hash=QString::fromLatin1(QCryptographicHash::hash(f.readAll(),QCryptographicHash::Sha256).toHex());
  return law.runPhoneOsint("+12025550100",scanners,engine,hash,device,true);
 }
};
}
class PhoneOsintCaseTests:public QObject {
 Q_OBJECT
private slots:
 void directCaseEvidenceNeedsNoDeviceOrReview(){
  Fixture f;f.device.clear();QVERIFY2(f.run(),qPrintable(f.law.error()));
  QTRY_VERIFY_WITH_TIMEOUT(!f.law.osintBusy(),20000);QVERIFY2(!f.law.osintResult().isEmpty(),qPrintable(f.law.error()));
  const auto result=f.law.osintResult();QVERIFY(f.law.verifyEvidence(result.value("evidenceId").toString()));QVERIFY(f.law.verifyAudit());
  QCOMPARE(result.value("receipt").toMap().value("schema").toString(),QString("nexvary-investigator-operation/v1"));
  QCOMPARE(result.value("receipt").toMap().value("status").toString(),QString("completed"));
  QVERIFY(f.run());QTRY_VERIFY_WITH_TIMEOUT(!f.law.osintBusy(),20000);QVERIFY(!f.law.osintResult().isEmpty());
 }
 void directSearchWithoutCaseOrApplicationLogin(){
  Fixture f;f.law.closeWorkspace();QVERIFY(f.identity.startDirectWork());f.device.clear();
  QVERIFY2(f.run(),qPrintable(f.law.error()));QTRY_VERIFY_WITH_TIMEOUT(!f.law.osintBusy(),20000);
  const auto result=f.law.osintResult();QVERIFY2(!result.isEmpty(),qPrintable(f.law.error()));QVERIFY(result.value("evidenceId").toString().isEmpty());
  QCOMPARE(result.value("receipt").toMap().value("status").toString(),QString("completed"));
  LawCaseStore store(f.t.filePath("cases.db"),true);QVERIFY(store.get(f.id).value("evidence").toList().isEmpty());
 }
 void cancelWithoutCaseWrite_data(){QTest::addColumn<QString>("reason");for(const auto *r:{"cancel","logout","workspace"})QTest::newRow(r)<<QString::fromLatin1(r);}
 void cancelWithoutCaseWrite(){
  QFETCH(QString,reason);Fixture f;QVERIFY(f.approve(f.id));QVERIFY(f.run({"ovh"}));
  QTest::qWait(200);
  if(reason=="cancel")f.law.cancelOsint();else if(reason=="revoke")f.policy.revoke("external-analysis");else if(reason=="logout")f.identity.logout();else f.law.closeWorkspace();
  QTRY_VERIFY_WITH_TIMEOUT(!f.law.osintBusy(),15000);QVERIFY(f.law.osintResult().isEmpty());
  LawCaseStore store(f.t.filePath("cases.db"),true);QVERIFY(store.get(f.id).value("evidence").toList().isEmpty());
 }
};
QTEST_GUILESS_MAIN(PhoneOsintCaseTests)
#include "phone_osint_case_tests.moc"
