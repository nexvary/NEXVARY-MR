#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
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
#include "ServiceOperationPolicy.hpp"
#include "FontReadability.hpp"
#include "ToolBridge.hpp"
#include "RepairBridge.hpp"
#include "WorkshopBridge.hpp"
#include "TrackingPortal.hpp"
#include "CloudTracking.hpp"
#include "SecurityBridge.hpp"
#include "ServiceCatalog.hpp"
#include "BoardBridge.hpp"
#include "FirmwareBridge.hpp"
#include "RecoveryBridge.hpp"
int main(int argc,char *argv[]) {
 QGuiApplication app(argc,argv);
 QGuiApplication::setApplicationName(mr::service::laboratoryBuild()?QStringLiteral("NEXVARY-MR-Laboratory"):QStringLiteral("NEXVARY-MR"));
 QGuiApplication::setOrganizationName(QStringLiteral("NEXVARY"));
 QGuiApplication::setApplicationVersion(QStringLiteral("0.4.0"));
 QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/nexvary-mr.png")));
 QQuickStyle::setStyle(QStringLiteral("Fusion"));
 QCommandLineParser cli;
 cli.addOption({QStringLiteral("build-profile"),QStringLiteral("Print the compiled device-operation profile and exit")});
 cli.addHelpOption();
 cli.addVersionOption();
 cli.addOption({QStringLiteral("data-dir"),QStringLiteral("Local data directory"),QStringLiteral("path")});
 cli.addOption({QStringLiteral("smoke-test"),QStringLiteral("Exercise all pages in an isolated test profile, capture and close")});
 cli.addOption({QStringLiteral("screenshot"),QStringLiteral("Runtime PNG output in smoke test"),QStringLiteral("path")});
 cli.process(app);
 if(cli.isSet(QStringLiteral("build-profile"))) {
  std::puts(mr::service::laboratoryBuild()?"laboratory-experimental":"production-read-only");return 0;
 }
 const bool smoke=cli.isSet(QStringLiteral("smoke-test"));
 if(smoke&&!cli.isSet(QStringLiteral("data-dir"))) return 2;
 const QString dir=cli.isSet(QStringLiteral("data-dir"))?cli.value(QStringLiteral("data-dir")):QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
 if(!QDir().mkpath(dir)) return 3;
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
 tools.loadToolConfiguration(QDir(dir).filePath(QStringLiteral("tools.json")));
 QObject::connect(&tools,&ToolBridge::diagnosticCompleted,&workshop,&WorkshopBridge::recordDiagnostic);
 QQmlApplicationEngine engine;
 engine.rootContext()->setContextProperty(QStringLiteral("toolBridge"),&tools);
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
 QObject::connect(&engine,&QQmlEngine::warnings,&app,[&qmlWarning](const QList<QQmlError> &warnings){if(!warnings.isEmpty())qmlWarning=true;});
 engine.load(QUrl(QStringLiteral("qrc:/NexvaryMR/qml/Main.qml")));
 if(engine.rootObjects().isEmpty()) return 1;
 if(smoke) {
  // Packaging smoke must fail if HTTPS cannot load outside the Qt SDK.
  if(!QSslSocket::supportsSsl())return 12;
  if(!mr::ui::textHasGlyphs(QString::fromUtf8("NEXVARYالعربية"),QFont(QStringLiteral("Segoe UI"))))return 14;
  if(!workshop.ready()||workshop.checkIntegrity()!=QStringLiteral("ok"))return 4;
  auto *window=qobject_cast<QQuickWindow *>(engine.rootObjects().first());
 if(!window)return 5;
  if(QImage(QStringLiteral(":/assets/codec-check.jpg")).size()!=QSize(8,8)) return 8;
  if(QImage(QStringLiteral(":/assets/nexvary-mr.png")).isNull()||QImage(QStringLiteral(":/assets/icons/home.svg")).isNull()) return 8;
  if(!board.importBoard(QStringLiteral(":/samples/training-board.json"),true)) return 9;
  // Original synthetic raw-image fixture exercises deployed recovery decoding,
  // source verification and Windows DPAPI persistence without a phone.
  QFile png(QStringLiteral(":/assets/nexvary-mr.png")),jpeg(QStringLiteral(":/assets/codec-check.jpg"));
  if(!png.open(QIODevice::ReadOnly)||!jpeg.open(QIODevice::ReadOnly))return 10;
  const auto fixturePath=QDir(dir).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces)+QStringLiteral("-synthetic-smoke.dd"));
  QFile fixture(fixturePath);
 if(!fixture.open(QIODevice::WriteOnly))return 10;
  const QByteArray fixtureBytes=QByteArray(65536,'\0')+png.readAll()+QByteArray(65536,'\0')+jpeg.readAll();
  if(fixture.write(fixtureBytes)!=fixtureBytes.size())return 10;
 fixture.close();
  if(!recovery.createCase(QStringLiteral("SYNTHETIC smoke case"),QStringLiteral("automated fixture consent"),QStringLiteral("Synthetic"),QStringLiteral("No phone"),QString(),QStringLiteral("deleted"),QStringLiteral("none"),true)||!recovery.recordOperator(QStringLiteral("Synthetic CI operator"),QStringLiteral("Automated fixture authorization; no customer data"),true)||!recovery.scanImage(fixturePath,true))return 10;
  // Local synthetic AVB declarations; no phone or OEM authenticity implied.
  const auto avbFixturePath=QDir(dir).filePath(QStringLiteral("synthetic-vbmeta.img"));
  QByteArray avbFixture(256,'\0');avbFixture.replace(0,4,"AVB0");
  qToBigEndian<quint32>(1,avbFixture.data()+4);qToBigEndian<quint64>(42,avbFixture.data()+112);
  QFile avbFile(avbFixturePath);if(!avbFile.open(QIODevice::WriteOnly)||avbFile.write(avbFixture)!=avbFixture.size())return 13;avbFile.close();
  if(auto *input=window->findChild<QObject *>(QStringLiteral("avbPath")))input->setProperty("text",QUrl::fromLocalFile(avbFixturePath).toString());
  firmware.inspectAvb(avbFixturePath);
  const int order=workshop.createOrder(QStringLiteral("SYNTHETIC workshop customer"),QStringLiteral("TEST-ONLY"),QStringLiteral("Samsung"),QStringLiteral("TRAINING-01"),QStringLiteral("synthetic-serial"),QStringLiteral("USB not detected"),QStringLiteral("CI technician"),QStringLiteral("Synthetic intake only"));
  if(order<=0||!workshop.updateOrder(order,QStringLiteral("Repairing"),QStringLiteral("CI technician"),12500,QStringLiteral("Private synthetic notes"))||!portal.start(QStringLiteral("127.0.0.1"),0,true))return 11;
  const auto tracking=portal.link(order,1,true);if(tracking.isEmpty())return 11;
  if(auto *consent=window->findChild<QObject *>(QStringLiteral("portalConsent")))consent->setProperty("checked",true);
  if(auto *consent=window->findChild<QObject *>(QStringLiteral("trackingShareConsent")))consent->setProperty("checked",true);
  if(auto *hub=window->findChild<QObject *>(QStringLiteral("workshopHub"))){hub->setProperty("customerData",workshop.customerProfile(workshop.invoice(order).value(QStringLiteral("customer_id")).toInt()));hub->setProperty("deviceData",workshop.deviceProfile(workshop.invoice(order).value(QStringLiteral("device_id")).toInt()));hub->setProperty("issuedLink",tracking);}
  const auto portalOk=std::make_shared<bool>(false);auto *network=new QNetworkAccessManager(&app);auto *reply=network->get(QNetworkRequest(QUrl(tracking)));
  QObject::connect(reply,&QNetworkReply::finished,&app,[reply,portalOk]{const auto body=reply->readAll();*portalOk=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()==200&&body.contains("Repairing")&&!body.contains("Private synthetic notes")&&!body.contains("TEST-ONLY");reply->deleteLater();});
  auto *timer=new QTimer(&app);
 timer->setInterval(160);
  int page=0;
  QObject::connect(timer,&QTimer::timeout,&app,[&,timer,window,page,portalOk]() mutable {
   if(page<40){window->setProperty("darkTheme",page<20);window->setProperty("ar",page%20<10);window->setProperty("currentPage",page%10);++page;return;}
   timer->stop();window->setProperty("darkTheme",true);window->setProperty("ar",true);window->setProperty("currentPage",3);
   if(auto *viewer=window->findChild<QObject *>(QStringLiteral("boardWorkspace")); viewer && board.components().size()>2) {viewer->setProperty("selected",board.components().at(2));}
   QTimer::singleShot(400,&app,[&,window] {
    bool ok=*portalOk&&!firmware.busy()&&firmware.avbReport().value(QStringLiteral("status")).toString()==QStringLiteral("metadata-only")&&!firmware.compatible()&&!recovery.busy()&&recovery.evidence().size()==2&&recovery.error().isEmpty();
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
        if(step==32){capture->stop();portal.stop();window->setProperty("allowClose",true);window->close();app.exit(ok&&!qmlWarning?0:6);return;}
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
  QTimer::singleShot(20000,&app,[&app]{app.exit(7);});
 }
 return app.exec();
}

