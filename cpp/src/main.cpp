#include <QTemporaryDir>
#include <QSaveFile>
#include "SyntheticPolicyExercise.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QCommandLineParser>
#include <QImage>
#include <QIcon>
#include <QFileInfo>
#include <QFile>
#include <QUuid>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QtEndian>
#include <memory>
#include <cstdio>
#include <functional>
#include "ServiceOperationPolicy.hpp"
#include "FontReadability.hpp"
#include "ToolBridge.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "RepairBridge.hpp"
#include "WorkshopBridge.hpp"
#include "TrackingPortal.hpp"
#include "CloudTracking.hpp"
#include "SecurityBridge.hpp"
#include "ServiceCatalog.hpp"
#include <QJsonDocument>
#include "BoardBridge.hpp"
#include "FirmwareBridge.hpp"
#include "RecoveryBridge.hpp"
#include "LawCaseBridge.hpp"
#include "IdentityService.hpp"
#include "ResearchBridge.hpp"
namespace {
// Diagnostics are emitted only by explicit synthetic smoke paths. Stage names
// are fixed literals; never print credentials, case contents or audit history.
bool smokeCheck(bool passed,const char *stage){
 if(!passed){std::fprintf(stderr,"Synthetic GUI smoke check failed: stage=%s\n",stage);std::fflush(stderr);}
 return passed;
}
int smokeFailure(const char *stage,int code){
 std::fprintf(stderr,"Synthetic GUI smoke exit: stage=%s code=%d\n",stage,code);std::fflush(stderr);return code;
}
// Actual deployed EXE receipt. This exports public synthetic history only;
// the retained anchor is independently verified by the packaging Python tool.
bool deployedPolicyAudit(GovernmentOperationPolicy &policy,const QString &screenshot){
 if(screenshot.isEmpty())return true;
 const QFileInfo file(screenshot);const auto base=QDir(file.absolutePath()).filePath(file.completeBaseName()+"-government-policy");
 auto anchor=policy.auditStatus();anchor.remove("healthy");anchor.remove("verification");
 const auto retained=QJsonDocument::fromVariant(anchor).toJson(QJsonDocument::Compact);
 QTemporaryDir temporary(QDir(file.absolutePath()).filePath("policy-smoke-XXXXXX"));
 if(!smokeCheck(temporary.isValid(),"policy-audit-temporary-directory"))return false;
 const auto exported=temporary.filePath("audit.json");
 if(!smokeCheck(policy.exportAudit(exported,true),"policy-audit-export")||!smokeCheck(policy.verifyAudit(exported,QString::fromUtf8(retained)),"policy-audit-retained-anchor-verification"))return false;
 QFile input(exported);if(!smokeCheck(input.open(QIODevice::ReadOnly),"policy-audit-export-read"))return false;
 const auto publicHistory=input.readAll();
 const auto artifact=[](const QString &path,const QByteArray &bytes){
  QSaveFile out(path);out.setDirectWriteFallback(false);
  return out.open(QIODevice::WriteOnly)&&out.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)&&out.write(bytes)==bytes.size()&&out.commit();
 };
 // Only explicit --smoke-test screenshot artifacts are replaceable; the normal
 // export API remains new-file-only. Repeat CI never reuses an older receipt.
 return smokeCheck(artifact(base+"-audit.json",publicHistory),"policy-audit-history-receipt-write")&&smokeCheck(artifact(base+"-anchor.json",retained),"policy-audit-anchor-receipt-write");
}
// Called only inside the existing explicit synthetic --smoke-test profile.
void governmentPolicySmoke(QQuickWindow *window,GovernmentOperationPolicy &policy,const QString &screenshot,const std::function<void(bool)> &finish){
 if(!smokeCheck(policy.offline(),"policy-ui-offline-precondition")){finish(false);return;}
 window->setProperty("governmentMode",true);window->setProperty("currentPage",12);window->setProperty("ar",true);window->setProperty("darkTheme",true);window->resize(1536,980);
 if(auto *workspace=window->findChild<QObject *>("lawEnforcementWorkspace"))workspace->setProperty("section",10);
 window->setProperty("activeSection",10);
 if(auto *panel=window->findChild<QObject *>("identityPanel"))if(auto *content=panel->property("contentItem").value<QObject *>())content->setProperty("contentY",0);
 QTimer::singleShot(250,window,[window,&policy,screenshot,finish]{
  if(auto *scroll=window->findChild<QObject *>("identityPanel")){
   auto *panel=scroll->findChild<QQuickItem *>("governmentNetworkPanel");
   auto *content=scroll->property("contentItem").value<QObject *>();
   if(panel&&content)content->setProperty("contentY",panel->y());
  }
  QTimer::singleShot(250,window,[window,&policy,screenshot,finish]{
  const QFileInfo file(screenshot);const auto destination=[&file](const QString &suffix){return QDir(file.absolutePath()).filePath(file.completeBaseName()+suffix);};
  const bool ar=screenshot.isEmpty()||window->grabWindow().save(destination("-government-offline-ar.png"));window->setProperty("ar",false);
  QTimer::singleShot(250,window,[window,&policy,screenshot,finish,ar]{
   const QFileInfo file(screenshot);const auto destination=[&file](const QString &suffix){return QDir(file.absolutePath()).filePath(file.completeBaseName()+suffix);};
   bool ok=ar&&policy.offline()&&(screenshot.isEmpty()||window->grabWindow().save(destination("-government-offline-en.png")));
   if(!screenshot.isEmpty()){
    const QVariantMap receipt{{"schema","nexvary-deployed-operation-policy/v1"},{"synthetic",true},{"defaultDeniedCategories",6},{"offlineAfterRevoke",policy.offline()},
     {"startupBoundaryDenials",true},{"explicitSyntheticPortalGrant",true},{"reviewedExceptionProtocol",true},{"signedPolicyAudit",policy.auditStatus().value("healthy")},{"policyVerification",policy.auditStatus().value("verification")},{"portalStoppedAfterRevoke",true},{"permissions",policy.permissions()},
     {"scope","Actual deployed application gate, synthetic loopback grant/revoke and local import/index smoke; not OS sandbox or field acceptance"}};
    QFile out(destination("-government-offline.json"));const auto data=QJsonDocument::fromVariant(receipt).toJson(QJsonDocument::Indented);ok=out.open(QIODevice::WriteOnly|QIODevice::Truncate)&&out.write(data)==data.size()&&ok;
   }
   finish(ok);
  });
  });
 });
}
void governmentArtifactSmoke(QQuickWindow *window, LawCaseBridge &law, const QString &directory,
 const QString &screenshot, const std::function<void(bool)> &finish) {
 const auto id=law.createCase({{"caseNumber","SYNTHETIC-DEPLOYED-INDEX"},{"investigatingAgency","Synthetic lab"},{"investigatorName","Synthetic operator"},
  {"seizedDeviceDescription","Synthetic JSON file"},{"authorityReference","Synthetic smoke authorization"},{"issuingAuthority","Lab owner"},{"scope","Synthetic provenance only"}});
 const auto device=law.addDevice({{"model","Synthetic source"},{"custodian","Synthetic custodian"}});
 const auto path=QDir(directory).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces)+"-synthetic-findings.json");
 QFile source(path);const auto bytes=QStringLiteral("{\"name\":\"أحمد\",\"note\":\"SYNTHETIC deployed index only\"}").toUtf8();
 if(!smokeCheck(!id.isEmpty()&&!device.isEmpty()&&source.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&source.write(bytes)==bytes.size(),"artifact-synthetic-source-setup")){finish(false);return;}source.close();
 const auto evidence=law.addEvidence(device,path,"Synthetic deployed index","Synthetic custodian",true);
 if(!smokeCheck(!evidence.isEmpty(),"artifact-protected-evidence-registration")){finish(false);return;}
 QObject::connect(&law,&LawCaseBridge::artifactsCompleted,window,[window,&law,screenshot,finish](bool ok){
  const auto result=law.artifactsResult();const auto rows=result.value("rows").toList();
  if(!smokeCheck(ok&&rows.size()==1&&rows[0].toMap().value("value")==QStringLiteral("أحمد")&&rows[0].toMap().value("provenancePath").toList().size()==5,"artifact-search-result-and-provenance")){finish(false);return;}
  window->setProperty("governmentMode",true);window->setProperty("currentPage",14);window->setProperty("ar",true);window->setProperty("darkTheme",true);window->resize(1536,980);
  QTimer::singleShot(250,window,[window,result,screenshot,finish]{
   auto *page=window->findChild<QObject *>("governmentAnalysisWorkspace");auto *panel=window->findChild<QQuickItem *>("governmentArtifactPanel");
   auto *scroll=page?page->property("contentItem").value<QObject *>():nullptr;
   if(!smokeCheck(panel&&scroll,"artifact-ui-panel-and-scroll")){finish(false);return;}
   if(auto *query=window->findChild<QObject *>("governmentArtifactQuery"))query->setProperty("text",QStringLiteral("أحمد"));
   scroll->setProperty("contentY",panel->y());
   QTimer::singleShot(250,window,[window,result,screenshot,finish]{
    const QFileInfo file(screenshot);const auto destination=[&file](const QString &suffix){return QDir(file.absolutePath()).filePath(file.completeBaseName()+suffix);};
    const bool arabic=screenshot.isEmpty()||window->grabWindow().save(destination("-government-artifacts-ar.png"));
    window->setProperty("ar",false);
    QTimer::singleShot(250,window,[window,result,screenshot,finish,arabic]{
     const QFileInfo file(screenshot);const auto destination=[&file](const QString &suffix){return QDir(file.absolutePath()).filePath(file.completeBaseName()+suffix);};
     bool valid=arabic&&(screenshot.isEmpty()||window->grabWindow().save(destination("-government-artifacts-en.png")));
     if(!screenshot.isEmpty()){
      const auto row=result.value("rows").toList().first().toMap();
      const QVariantMap receipt{{"schema","nexvary-deployed-artifact-index/v1"},{"synthetic",true},{"findingCount",1},{"provenancePathNodes",5},
       {"sourceSha256",row.value("sourceSha256")},{"reportSha256",row.value("reportSha256")},{"query",result.value("query")},
       {"nodes",result.value("nodes").toList().size()},{"edges",result.value("edges").toList().size()},{"scope","Actual deployed synthetic Government UI and native index; no field/institutional verification"}};
      QFile output(destination("-government-artifacts.json"));const auto data=QJsonDocument::fromVariant(receipt).toJson(QJsonDocument::Indented);
      valid=output.open(QIODevice::WriteOnly|QIODevice::Truncate)&&output.write(data)==data.size()&&valid;
     }
     finish(valid);
    });
   });
  });
 },Qt::SingleShotConnection);
 QObject::connect(&law,&LawCaseBridge::importCompleted,window,[&law,finish](bool ok){if(!ok||!law.buildArtifacts(QStringLiteral("أحمد")))finish(false);},Qt::SingleShotConnection);
 if(!law.importEvidence(evidence,"json","Synthetic deployed provenance smoke",true))finish(false);
}
}
int main(int argc,char *argv[]) {
 QGuiApplication app(argc,argv);
 QGuiApplication::setApplicationName(mr::service::laboratoryBuild()?QStringLiteral("NEXVARY-MR-Laboratory"):QStringLiteral("NEXVARY-MR"));
 QGuiApplication::setOrganizationName(QStringLiteral("NEXVARY"));
 QGuiApplication::setApplicationVersion(QStringLiteral("0.4.0"));
 QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/nexvary-mr.png")));
 QQuickStyle::setStyle(QStringLiteral("Fusion"));
 QCommandLineParser cli;
 cli.addOption({QStringLiteral("build-profile"),QStringLiteral("Print the compiled device-operation profile and exit")});
 cli.addOption({QStringLiteral("screen-tools-probe"),QStringLiteral("Print screen-tool resolution without starting tools or opening cases")});
 cli.addHelpOption();
 cli.addVersionOption();
 cli.addOption({QStringLiteral("data-dir"),QStringLiteral("Local data directory"),QStringLiteral("path")});
 cli.addOption({QStringLiteral("smoke-test"),QStringLiteral("Exercise all pages in an isolated test profile, capture and close")});
 cli.addOption({QStringLiteral("screenshot"),QStringLiteral("Runtime PNG output in smoke test"),QStringLiteral("path")});
 cli.process(app);
 if(cli.isSet(QStringLiteral("build-profile"))) {
  std::puts(mr::service::laboratoryBuild()?"laboratory-experimental":"production-read-only");return 0;
 }
 if(cli.isSet(QStringLiteral("screen-tools-probe"))){
  ToolBridge probe;
  const auto json=QJsonDocument::fromVariant(QVariantMap{{"screenViewAvailable",probe.screenViewAvailable()},{"tools",probe.tools()}}).toJson(QJsonDocument::Compact);
  std::fputs(json.constData(),stdout);return 0;
 }
 const bool smoke=cli.isSet(QStringLiteral("smoke-test"));
 if(smoke&&!cli.isSet(QStringLiteral("data-dir"))) return smokeFailure("missing-synthetic-data-directory",2);
 const QString dir=cli.isSet(QStringLiteral("data-dir"))?cli.value(QStringLiteral("data-dir")):QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
 if(!QDir().mkpath(dir)) return smoke?smokeFailure("synthetic-data-directory-create",3):3;
 IdentityService identity(QDir(dir).filePath(QStringLiteral("local-identities.dat")),smoke);
 GovernmentOperationPolicy operationPolicy(&identity);
 RecoveryBridge recovery(QDir(dir).filePath(QStringLiteral("recovery-cases")),smoke);
 ToolBridge tools;
 RepairBridge repair;
 WorkshopBridge workshop(QDir(dir).filePath(QStringLiteral("repairs.db")));
 TrackingPortal portal(&workshop);
 CloudTracking cloud(&workshop,QDir(dir).filePath(QStringLiteral("remote-tracking.dat")));
 SecurityBridge security(QDir(dir).filePath(QStringLiteral("security-tools.json")));
 ServiceCatalog catalog(QDir(dir).filePath(QStringLiteral("model-guides.json")));
 BoardBridge board;
 FirmwareBridge firmware;
 tools.setOperationPolicy(&operationPolicy);cloud.setOperationPolicy(&operationPolicy);portal.setOperationPolicy(&operationPolicy);security.setOperationPolicy(&operationPolicy);recovery.setOperationPolicy(&operationPolicy);
 LawCaseBridge law(QDir(dir).filePath(QStringLiteral("law-cases.db")),&tools,smoke,nexvary::law::Role::Investigator,nullptr,smoke?nullptr:&identity);
 ResearchBridge research(QDir(dir).filePath(QStringLiteral("public-research.json")),&tools,&firmware,nullptr,smoke?nullptr:&identity);
 tools.loadToolConfiguration(QDir(dir).filePath(QStringLiteral("tools.json")));
 QObject::connect(&tools,&ToolBridge::diagnosticCompleted,&workshop,&WorkshopBridge::recordDiagnostic);
 QQmlApplicationEngine engine;
 engine.rootContext()->setContextProperty(QStringLiteral("operationPolicy"),&operationPolicy);
 engine.rootContext()->setContextProperty(QStringLiteral("toolBridge"),&tools);
 engine.rootContext()->setContextProperty(QStringLiteral("lawCaseBridge"),&law);
 engine.rootContext()->setContextProperty(QStringLiteral("identityBridge"),&identity);
 engine.rootContext()->setContextProperty(QStringLiteral("researchBridge"),&research);
 engine.rootContext()->setContextProperty(QStringLiteral("repairBridge"),&repair);
 engine.rootContext()->setContextProperty(QStringLiteral("workshopBridge"),&workshop);
 engine.rootContext()->setContextProperty(QStringLiteral("boardBridge"),&board);
 engine.rootContext()->setContextProperty(QStringLiteral("firmwareBridge"),&firmware);
 engine.rootContext()->setContextProperty(QStringLiteral("recoveryBridge"),&recovery);
 engine.rootContext()->setContextProperty(QStringLiteral("trackingPortal"),&portal);
 engine.rootContext()->setContextProperty(QStringLiteral("securityBridge"),&security);
 engine.rootContext()->setContextProperty(QStringLiteral("serviceCatalog"),&catalog);
 engine.rootContext()->setContextProperty(QStringLiteral("cloudTracking"),&cloud);
 bool qmlWarning=false;
 QObject::connect(&engine,&QQmlEngine::warnings,&app,[&qmlWarning,smoke](const QList<QQmlError> &warnings){if(!warnings.isEmpty())qmlWarning=true;if(smoke)for(const auto &warning:warnings)std::fprintf(stderr,"Synthetic GUI smoke QML warning: %s\n",qPrintable(warning.toString()));});
 engine.load(QUrl(QStringLiteral("qrc:/NexvaryMR/qml/Main.qml")));
 if(engine.rootObjects().isEmpty()) return smoke?smokeFailure("qml-root-load",1):1;
 if(smoke) {
  // Explicit synthetic smoke only: prove default denial, then temporarily allow
  // the loopback portal test through the same request/review/decision/activation APIs as the UI.
  if(!smokeCheck(operationPolicy.offline(),"policy-startup-offline")||!smokeCheck(!portal.start("127.0.0.1",0,true),"policy-startup-portal-denied")||!smokeCheck(!cloud.syncNow(),"policy-startup-cloud-denied")||!smokeCheck(!security.updateDatabase(true),"policy-startup-update-denied")||!smokeCheck(!security.scan("synthetic-not-read",true),"policy-startup-scan-denied")||!smokeCheck(!recovery.analyzePartitions("synthetic-not-read","synthetic-not-started",true),"policy-startup-recovery-denied")||!smokeCheck(!operationPolicy.openExternal("https://example.invalid/"),"policy-startup-browser-denied"))return smokeFailure("policy-startup-boundaries",15);
  tools.scanAdb();if(!smokeCheck(!tools.busy(),"policy-startup-adb-denied"))return smokeFailure("policy-startup-device-boundary",15);
  const QString smokePassword="Synthetic-only deployed policy 2026!";
  if((identity.needsSetup()&&!smokeCheck(identity.bootstrap("smoke-admin","Synthetic smoke administrator",smokePassword),"synthetic-identity-bootstrap"))||!smokeCheck(identity.login("smoke-admin",smokePassword),"synthetic-identity-login")||!smokeCheck(performSyntheticReviewedException(identity,operationPolicy,"tracking-portal","Synthetic deployed loopback portal test only",smokePassword),"synthetic-reviewed-portal-exception"))return smokeFailure("policy-reviewed-setup",15);
  // Packaging smoke must fail if HTTPS cannot load outside the Qt SDK.
  if(!QSslSocket::supportsSsl())return smokeFailure("deployed-tls-runtime",12);
  if(!mr::ui::textHasGlyphs(QString::fromUtf8("NEXVARYالعربية"),QFont(QStringLiteral("Segoe UI"))))return smokeFailure("arabic-latin-font-glyphs",14);
  if(!workshop.ready()||workshop.checkIntegrity()!=QStringLiteral("ok"))return smokeFailure("workshop-database-integrity",4);
  auto *window=qobject_cast<QQuickWindow *>(engine.rootObjects().first());
 if(!window)return smokeFailure("qml-window-type",5);
  if(QImage(QStringLiteral(":/assets/codec-check.jpg")).size()!=QSize(8,8)) return smokeFailure("jpeg-codec-resource",8);
  if(QImage(QStringLiteral(":/assets/nexvary-mr.png")).isNull()||QImage(QStringLiteral(":/assets/icons/home.svg")).isNull()||QImage(QStringLiteral(":/assets/icons/check_mark.svg")).isNull()) return smokeFailure("png-svg-codec-resources",8);
  if(!board.importBoard(QStringLiteral(":/samples/training-board.json"),true)) return smokeFailure("synthetic-board-import",9);
  // Original synthetic raw-image fixture exercises deployed recovery decoding,
  // source verification and Windows DPAPI persistence without a phone.
  QFile png(QStringLiteral(":/assets/nexvary-mr.png")),jpeg(QStringLiteral(":/assets/codec-check.jpg"));
  if(!png.open(QIODevice::ReadOnly)||!jpeg.open(QIODevice::ReadOnly))return smokeFailure("synthetic-recovery-resources",10);
  const auto fixturePath=QDir(dir).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces)+QStringLiteral("-synthetic-smoke.dd"));
  QFile fixture(fixturePath);
 if(!fixture.open(QIODevice::WriteOnly))return smokeFailure("synthetic-image-create",10);
  const QByteArray fixtureBytes=QByteArray(65536,'\0')+png.readAll()+QByteArray(65536,'\0')+jpeg.readAll();
  if(fixture.write(fixtureBytes)!=fixtureBytes.size())return smokeFailure("synthetic-image-write",10);
 fixture.close();
  if(!recovery.createCase(QStringLiteral("SYNTHETIC smoke case"),QStringLiteral("automated fixture consent"),QStringLiteral("Synthetic"),QStringLiteral("No phone"),QString(),QStringLiteral("deleted"),QStringLiteral("none"),true)||!recovery.recordOperator(QStringLiteral("Synthetic CI operator"),QStringLiteral("Automated fixture authorization; no customer data"),true)||!recovery.scanImage(fixturePath,true))return smokeFailure("synthetic-recovery-start",10);
  // Local synthetic AVB declarations; no phone or OEM authenticity implied.
  const auto avbFixturePath=QDir(dir).filePath(QStringLiteral("synthetic-vbmeta.img"));
  QByteArray avbFixture(256,'\0');avbFixture.replace(0,4,"AVB0");
  qToBigEndian<quint32>(1,avbFixture.data()+4);qToBigEndian<quint64>(42,avbFixture.data()+112);
  QFile avbFile(avbFixturePath);if(!avbFile.open(QIODevice::WriteOnly)||avbFile.write(avbFixture)!=avbFixture.size())return smokeFailure("synthetic-avb-fixture",13);avbFile.close();
  if(auto *input=window->findChild<QObject *>(QStringLiteral("avbPath")))input->setProperty("text",QUrl::fromLocalFile(avbFixturePath).toString());
  firmware.inspectAvb(avbFixturePath);
  const int order=workshop.createOrder(QStringLiteral("SYNTHETIC workshop customer"),QStringLiteral("TEST-ONLY"),QStringLiteral("Samsung"),QStringLiteral("TRAINING-01"),QStringLiteral("synthetic-serial"),QStringLiteral("USB not detected"),QStringLiteral("CI technician"),QStringLiteral("Synthetic intake only"));
  if(order<=0||!workshop.updateOrder(order,QStringLiteral("Repairing"),QStringLiteral("CI technician"),12500,QStringLiteral("Private synthetic notes"))||!portal.start(QStringLiteral("127.0.0.1"),0,true))return smokeFailure("synthetic-workshop-portal-start",11);
  const auto tracking=portal.link(order,1,true);if(tracking.isEmpty())return smokeFailure("synthetic-portal-link",11);
  if(auto *consent=window->findChild<QObject *>(QStringLiteral("portalConsent")))consent->setProperty("checked",true);
  if(auto *consent=window->findChild<QObject *>(QStringLiteral("trackingShareConsent")))consent->setProperty("checked",true);
  if(auto *hub=window->findChild<QObject *>(QStringLiteral("workshopHub"))){hub->setProperty("customerData",workshop.customerProfile(workshop.invoice(order).value(QStringLiteral("customer_id")).toInt()));hub->setProperty("deviceData",workshop.deviceProfile(workshop.invoice(order).value(QStringLiteral("device_id")).toInt()));hub->setProperty("issuedLink",tracking);}
  const auto portalOk=std::make_shared<bool>(false);auto *network=new QNetworkAccessManager(&app);auto *reply=network->get(QNetworkRequest(QUrl(tracking)));
  QObject::connect(reply,&QNetworkReply::finished,&app,[reply,portalOk]{const auto body=reply->readAll();*portalOk=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200&&body.contains("Repairing")&&!body.contains("Private synthetic notes")&&!body.contains("TEST-ONLY");reply->deleteLater();});
  auto *timer=new QTimer(&app);
 timer->setInterval(160);
  int page=0;
  QObject::connect(timer,&QTimer::timeout,&app,[&,timer,window,page,portalOk]() mutable {
   if(page<48){window->setProperty("darkTheme",page<24);window->setProperty("ar",page%24<12);window->setProperty("currentPage",page%12);++page;return;}
   timer->stop();window->setProperty("darkTheme",true);window->setProperty("ar",true);window->setProperty("currentPage",3);
   if(auto *viewer=window->findChild<QObject *>(QStringLiteral("boardWorkspace")); viewer && board.components().size()>2) {viewer->setProperty("selected",board.components().at(2));}
   QTimer::singleShot(400,&app,[&,window] {
    bool ok=smokeCheck(*portalOk,"loopback-portal-response")&&smokeCheck(!firmware.busy(),"firmware-operation-complete")&&smokeCheck(firmware.avbReport().value(QStringLiteral("status")).toString()==QStringLiteral("metadata-only"),"synthetic-avb-metadata")&&smokeCheck(!firmware.compatible(),"synthetic-avb-unverified-capability")&&smokeCheck(!recovery.busy(),"recovery-operation-complete")&&smokeCheck(recovery.evidence().size()==2,"synthetic-recovery-evidence-count")&&smokeCheck(recovery.error().isEmpty(),"synthetic-recovery-error-free");
    if(cli.isSet(QStringLiteral("screenshot"))) {const auto path=cli.value(QStringLiteral("screenshot"));QDir().mkpath(QFileInfo(path).absolutePath());ok=window->grabWindow().save(path)&&ok;}
    window->setProperty("currentPage",8);
    if(auto *workspace=window->findChild<QObject *>(QStringLiteral("recoveryWorkspace")))workspace->setProperty("step",6);
    if(auto *tabs=window->findChild<QObject *>(QStringLiteral("recoveryTabs")))tabs->setProperty("currentIndex",1);
    QTimer::singleShot(250,&app,[&,window,ok]() mutable {
     if(cli.isSet(QStringLiteral("screenshot"))){
      const QFileInfo original(cli.value(QStringLiteral("screenshot")));
      const auto path=QDir(original.absolutePath()).filePath(original.completeBaseName()+QStringLiteral("-recovery.png"));
      ok=window->grabWindow().save(path)&&ok;
     }
     ok=recovery.exportFormattedReport(QDir(dir).filePath(recovery.caseId()+QStringLiteral("-synthetic-report.html")),QStringLiteral("html"),false,true)&&ok;
     ok=recovery.exportFormattedReport(QDir(dir).filePath(recovery.caseId()+QStringLiteral("-synthetic-report.pdf")),QStringLiteral("pdf"),true,true)&&ok;
     ok=recovery.exportTimeline(QDir(dir).filePath(recovery.caseId()+QStringLiteral("-synthetic-timeline.json")),true)&&ok;
     window->setProperty("currentPage",9);
     if(auto *tabs=window->findChild<QObject *>(QStringLiteral("forensicTabs")))tabs->setProperty("currentIndex",1);
     if(auto *workspace=window->findChild<QObject *>(QStringLiteral("forensicEvidenceWorkspace")); workspace&&!recovery.evidence().isEmpty())workspace->setProperty("selectedId",recovery.evidence().first().toMap().value(QStringLiteral("id")));
     QTimer::singleShot(250,&app,[&,window,ok]() mutable {
      if(cli.isSet(QStringLiteral("screenshot"))){const QFileInfo original(cli.value(QStringLiteral("screenshot")));ok=window->grabWindow().save(QDir(original.absolutePath()).filePath(original.completeBaseName()+QStringLiteral("-forensic-ar.png")))&&ok;}
      window->setProperty("ar",false);window->setProperty("darkTheme",false);
      QTimer::singleShot(250,&app,[&,window,ok]() mutable {
       if(cli.isSet(QStringLiteral("screenshot"))){const QFileInfo original(cli.value(QStringLiteral("screenshot")));ok=window->grabWindow().save(QDir(original.absolutePath()).filePath(original.completeBaseName()+QStringLiteral("-forensic-en-light.png")))&&ok;}
       // Exercise every new workshop/diagnostic tab in both directions/themes,
       // retaining actual deployed captures in the validation artifact.
       auto *capture=new QTimer(&app);capture->setInterval(180);int step=0;
       QObject::connect(capture,&QTimer::timeout,&app,[&,window,capture,step,ok]() mutable {
        const QStringList names={QStringLiteral("profiles"),QStringLiteral("tracking"),QStringLiteral("tree"),QStringLiteral("security"),QStringLiteral("drivers"),QStringLiteral("services"),QStringLiteral("catalog"),QStringLiteral("avb")};
        if(step>0&&cli.isSet(QStringLiteral("screenshot"))){const int previous=step-1;const QFileInfo file(cli.value(QStringLiteral("screenshot")));const auto suffix=QStringLiteral("-%1-%2-%3.png").arg(names[previous%8],previous%16<8?QStringLiteral("ar"):QStringLiteral("en"),previous<16?QStringLiteral("dark"):QStringLiteral("light"));ok=window->grabWindow().save(QDir(file.absolutePath()).filePath(file.completeBaseName()+suffix))&&ok;}
        if(step==32){capture->stop();operationPolicy.revokeAll();if(!deployedPolicyAudit(operationPolicy,cli.isSet("screenshot")?cli.value("screenshot"):QString())){app.exit(smokeFailure("policy-audit-deployed-verification",16));return;}identity.logout();if(!smokeCheck(operationPolicy.offline(),"policy-offline-after-revoke")||!smokeCheck(!portal.running(),"portal-stopped-after-revoke")){app.exit(smokeFailure("policy-revocation-boundary",15));return;}governmentPolicySmoke(window,operationPolicy,cli.isSet("screenshot")?cli.value("screenshot"):QString(),[&,window,ok](bool policyOk){governmentArtifactSmoke(window,law,dir,cli.isSet("screenshot")?cli.value("screenshot"):QString(),[&,window,ok,policyOk](bool indexOk){indexOk=indexOk&&policyOk;window->setProperty("allowClose",true);window->close();if(!ok||!indexOk||qmlWarning)std::fprintf(stderr,"Synthetic GUI smoke failed: composite=%d governmentIndex=%d qmlWarnings=%d\n",int(ok),int(indexOk),int(qmlWarning));app.exit(ok&&indexOk&&!qmlWarning?0:6);});});return;}
        const int tab=step%8;window->setProperty("ar",step%16<8);window->setProperty("darkTheme",step<16);window->setProperty("currentPage",tab==7?4:tab<2?5:tab<5?2:1);
        // Capture the new service/catalog workspaces at the supported minimum.
        window->resize(tab>=5?QSize(1180,760):QSize(1536,980));
        const auto tabs=tab==7?QStringLiteral("firmwareTabs"):tab<2?QStringLiteral("workshopTabs"):tab<5?QStringLiteral("diagnosticTabs"):QStringLiteral("deviceTabs");if(auto *bar=window->findChild<QObject *>(tabs))bar->setProperty("currentIndex",tab==7?1:tab<2?tab+1:tab<5?tab-1:tab-4);else ok=false;
        if(tab==2){if(auto *tree=window->findChild<QObject *>(QStringLiteral("faultTreePage")))QMetaObject::invokeMethod(tree,"start");}
        ++step;
       });capture->start();
      });
     });
    });
   });
  });
 timer->start();
  QTimer::singleShot(45000,&app,[&app]{std::fputs("Synthetic GUI smoke rendering deadline exceeded\n",stderr);app.exit(7);});
 }
 const auto exitCode=app.exec();
 if(smoke){std::fprintf(stderr,"Synthetic GUI smoke event-loop exit: code=%d\n",exitCode);std::fflush(stderr);}
 return exitCode;
}

