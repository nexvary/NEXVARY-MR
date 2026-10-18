#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>
#include <QTemporaryDir>
#include <QFile>
#include <QQuickStyle>
#include <memory>
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
class Setup:public QObject {
 Q_OBJECT
 QTemporaryDir temp;
 std::unique_ptr<ToolBridge> tools;
 std::unique_ptr<RepairBridge> repair;
 std::unique_ptr<WorkshopBridge> workshop;
 std::unique_ptr<ServiceCatalog> catalog;
 std::unique_ptr<SecurityBridge> security;
 std::unique_ptr<CloudTracking> cloud;
 std::unique_ptr<TrackingPortal> portal;
 std::unique_ptr<BoardBridge> board;
 std::unique_ptr<FirmwareBridge> firmware;
 std::unique_ptr<RecoveryBridge> recovery;
public slots:
 void applicationAvailable() {
  // QuickTest constructs Setup before QGuiApplication. Qt plug-ins, SQL and
  // process/thread machinery must be initialized only after the application.
  QQuickStyle::setStyle(QStringLiteral("Fusion"));
  tools=std::make_unique<ToolBridge>();repair=std::make_unique<RepairBridge>();
  workshop=std::make_unique<WorkshopBridge>(temp.filePath(QStringLiteral("ui.db")));
  catalog=std::make_unique<ServiceCatalog>(temp.filePath("guides.json"));
  security=std::make_unique<SecurityBridge>(temp.filePath("security.json"));
  cloud=std::make_unique<CloudTracking>(workshop.get(),temp.filePath("remote.dat"),true);
  portal=std::make_unique<TrackingPortal>(workshop.get());
  recovery=std::make_unique<RecoveryBridge>(temp.filePath(QStringLiteral("cases")),true);
  board=std::make_unique<BoardBridge>();firmware=std::make_unique<FirmwareBridge>();
 }
 void qmlEngineAvailable(QQmlEngine *engine) {
  const auto fixture=temp.filePath(QStringLiteral("synthetic-evidence.txt"));
  QFile file(fixture);if(file.open(QIODevice::WriteOnly))file.write("SYNTHETIC UI EVIDENCE ONLY\n");file.close();
  engine->rootContext()->setContextProperty("syntheticEvidencePath",fixture);
  engine->rootContext()->setContextProperty("toolBridge",tools.get());
  engine->rootContext()->setContextProperty("repairBridge",repair.get());
  engine->rootContext()->setContextProperty("workshopBridge",workshop.get());
  engine->rootContext()->setContextProperty("serviceCatalog",catalog.get());
  engine->rootContext()->setContextProperty("securityBridge",security.get());
  engine->rootContext()->setContextProperty("cloudTracking",cloud.get());
  engine->rootContext()->setContextProperty("trackingPortal",portal.get());
  engine->rootContext()->setContextProperty("boardBridge",board.get());
  engine->rootContext()->setContextProperty("firmwareBridge",firmware.get());
  engine->rootContext()->setContextProperty("recoveryBridge",recovery.get());
 }
 void cleanupTestCase() {
  // Release runtime-dependent objects before QuickTest destroys its application.
  catalog.reset();security.reset();cloud.reset();portal.reset();recovery.reset();firmware.reset();board.reset();workshop.reset();repair.reset();tools.reset();
 }
};
QUICK_TEST_MAIN_WITH_SETUP(mr_ui,Setup)
#include "ui_tests.moc"
