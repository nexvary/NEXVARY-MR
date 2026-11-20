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
#include "ToolBridge.hpp"
#include "RepairBridge.hpp"
#include "WorkshopBridge.hpp"
#include "BoardBridge.hpp"
#include "FirmwareBridge.hpp"
#include "RecoveryBridge.hpp"
int main(int argc,char *argv[]) {
 QGuiApplication app(argc,argv);
 QGuiApplication::setApplicationName(QStringLiteral("NEXVARY-MR"));
 QGuiApplication::setOrganizationName(QStringLiteral("NEXVARY"));
 QGuiApplication::setApplicationVersion(QStringLiteral("0.4.0"));
 QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/nexvary-mr.png")));
 QQuickStyle::setStyle(QStringLiteral("Fusion"));
 QCommandLineParser cli;
 cli.addHelpOption();
 cli.addVersionOption();
 cli.addOption({QStringLiteral("data-dir"),QStringLiteral("Local data directory"),QStringLiteral("path")});
 cli.addOption({QStringLiteral("smoke-test"),QStringLiteral("Exercise all pages in an isolated test profile, capture and close")});
 cli.addOption({QStringLiteral("screenshot"),QStringLiteral("Runtime PNG output in smoke test"),QStringLiteral("path")});
 cli.process(app);
 const bool smoke=cli.isSet(QStringLiteral("smoke-test"));
 if(smoke&&!cli.isSet(QStringLiteral("data-dir"))) return 2;
 const QString dir=cli.isSet(QStringLiteral("data-dir"))?cli.value(QStringLiteral("data-dir")):QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
 if(!QDir().mkpath(dir)) return 3;
 RecoveryBridge recovery(QDir(dir).filePath(QStringLiteral("recovery-cases")),smoke);
 ToolBridge tools;
 RepairBridge repair;
 WorkshopBridge workshop(QDir(dir).filePath(QStringLiteral("repairs.db")));
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
 bool qmlWarning=false;
 QObject::connect(&engine,&QQmlEngine::warnings,&app,[&qmlWarning](const QList<QQmlError> &warnings){if(!warnings.isEmpty())qmlWarning=true;});
 engine.load(QUrl(QStringLiteral("qrc:/NexvaryMR/qml/Main.qml")));
 if(engine.rootObjects().isEmpty()) return 1;
 if(smoke) {
  if(!workshop.ready()||workshop.checkIntegrity()!=QStringLiteral("ok"))return 4;
  auto *window=qobject_cast<QQuickWindow *>(engine.rootObjects().first());
 if(!window)return 5;
  if(QImage(QStringLiteral(":/assets/codec-check.jpg")).size()!=QSize(8,8)) return 8;
  if(QImage(QStringLiteral(":/assets/nexvary-mr.png")).isNull()) return 8;
  if(!board.importBoard(QStringLiteral(":/samples/training-board.json"),true)) return 9;
  // Original synthetic raw-image fixture exercises deployed recovery decoding,
  // source verification and Windows DPAPI persistence without a phone.
  QFile png(QStringLiteral(":/assets/nexvary-mr.png")),jpeg(QStringLiteral(":/assets/codec-check.jpg"));
  if(!png.open(QIODevice::ReadOnly)||!jpeg.open(QIODevice::ReadOnly))return 10;
  const auto fixturePath=QDir(dir).filePath(QStringLiteral("synthetic-smoke.dd"));
  QFile fixture(fixturePath);
 if(!fixture.open(QIODevice::WriteOnly))return 10;
  const QByteArray fixtureBytes=QByteArray(65536,'\0')+png.readAll()+QByteArray(65536,'\0')+jpeg.readAll();
  if(fixture.write(fixtureBytes)!=fixtureBytes.size())return 10;
 fixture.close();
  if(!recovery.createCase(QStringLiteral("SYNTHETIC smoke case"),QStringLiteral("automated fixture consent"),QStringLiteral("Synthetic"),QStringLiteral("No phone"),QString(),QStringLiteral("deleted"),QStringLiteral("none"),true)||!recovery.scanImage(fixturePath,true))return 10;
  auto *timer=new QTimer(&app);
 timer->setInterval(160);
  int page=0;
  QObject::connect(timer,&QTimer::timeout,&app,[&,timer,window,page]() mutable {
   if(page<18){window->setProperty("ar",page<9);window->setProperty("currentPage",page%9);++page;return;}
   timer->stop();window->setProperty("ar",true);window->setProperty("currentPage",3);
   if(auto *viewer=window->findChild<QObject *>(QStringLiteral("boardWorkspace")); viewer && board.components().size()>2) {viewer->setProperty("selected",board.components().at(2));}
   QTimer::singleShot(400,&app,[&,window] {
    bool ok=!recovery.busy()&&recovery.evidence().size()==2&&recovery.error().isEmpty();
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
     window->setProperty("allowClose",true);window->close();app.exit(ok&&!qmlWarning?0:6);
    });
   });
  });
 timer->start();
  QTimer::singleShot(20000,&app,[&app]{app.exit(7);});
 }
 return app.exec();
}
