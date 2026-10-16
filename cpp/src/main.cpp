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
#include <QFileInfo>
#include "ToolBridge.hpp"
#include "RepairBridge.hpp"
#include "WorkshopBridge.hpp"
#include "BoardBridge.hpp"
#include "FirmwareBridge.hpp"
int main(int argc,char *argv[]) {
 QGuiApplication app(argc,argv);
 QGuiApplication::setApplicationName(QStringLiteral("NEXVARY-MR"));
 QGuiApplication::setOrganizationName(QStringLiteral("NEXVARY"));
 QGuiApplication::setApplicationVersion(QStringLiteral("0.3.0"));
 QQuickStyle::setStyle(QStringLiteral("Fusion"));
 QCommandLineParser cli;cli.addHelpOption();cli.addVersionOption();
 cli.addOption({QStringLiteral("data-dir"),QStringLiteral("Local data directory"),QStringLiteral("path")});
 cli.addOption({QStringLiteral("smoke-test"),QStringLiteral("Exercise all pages in an isolated test profile, capture and close")});
 cli.addOption({QStringLiteral("screenshot"),QStringLiteral("Runtime PNG output in smoke test"),QStringLiteral("path")});
 cli.process(app);
 const bool smoke=cli.isSet(QStringLiteral("smoke-test"));
 if(smoke&&!cli.isSet(QStringLiteral("data-dir"))) return 2;
 const QString dir=cli.isSet(QStringLiteral("data-dir"))?cli.value(QStringLiteral("data-dir")):QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
 if(!QDir().mkpath(dir)) return 3;
 ToolBridge tools;RepairBridge repair;WorkshopBridge workshop(QDir(dir).filePath(QStringLiteral("repairs.db")));BoardBridge board;FirmwareBridge firmware;
 QQmlApplicationEngine engine;
 engine.rootContext()->setContextProperty(QStringLiteral("toolBridge"),&tools);
 engine.rootContext()->setContextProperty(QStringLiteral("repairBridge"),&repair);
 engine.rootContext()->setContextProperty(QStringLiteral("workshopBridge"),&workshop);
 engine.rootContext()->setContextProperty(QStringLiteral("boardBridge"),&board);
 engine.rootContext()->setContextProperty(QStringLiteral("firmwareBridge"),&firmware);
 bool qmlWarning=false;
 QObject::connect(&engine,&QQmlEngine::warnings,&app,[&qmlWarning](const QList<QQmlError> &warnings){if(!warnings.isEmpty())qmlWarning=true;});
 engine.load(QUrl(QStringLiteral("qrc:/NexvaryMR/qml/Main.qml")));
 if(engine.rootObjects().isEmpty()) return 1;
 if(smoke) {
  if(!workshop.ready()||workshop.checkIntegrity()!=QStringLiteral("ok"))return 4;
  auto *window=qobject_cast<QQuickWindow *>(engine.rootObjects().first());if(!window)return 5;
  board.importBoard(QStringLiteral(":/samples/training-board.json"),true);
  auto *timer=new QTimer(&app);timer->setInterval(160);
  int page=0;
  QObject::connect(timer,&QTimer::timeout,&app,[&,timer,window,page]() mutable {
   if(page<16){window->setProperty("ar",page<8);window->setProperty("currentPage",page%8);++page;return;}
   timer->stop();window->setProperty("ar",true);window->setProperty("currentPage",3);
   QTimer::singleShot(400,&app,[&,window] {
    bool ok=true;
    if(cli.isSet(QStringLiteral("screenshot"))) {const auto path=cli.value(QStringLiteral("screenshot"));QDir().mkpath(QFileInfo(path).absolutePath());ok=window->grabWindow().save(path);}
    window->setProperty("allowClose",true);window->close();app.exit(ok&&!qmlWarning?0:6);
   });
  });timer->start();
  QTimer::singleShot(20000,&app,[&app]{app.exit(7);});
 }
 return app.exec();
}
