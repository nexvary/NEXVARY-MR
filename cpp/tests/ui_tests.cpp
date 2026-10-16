#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>
#include <QTemporaryDir>
#include "ToolBridge.hpp"
#include "RepairBridge.hpp"
#include "WorkshopBridge.hpp"
#include "BoardBridge.hpp"
#include "FirmwareBridge.hpp"
class Setup:public QObject {
 Q_OBJECT
 QTemporaryDir temp;
 ToolBridge tools;
 RepairBridge repair;
 WorkshopBridge workshop{temp.filePath("ui.db")};
 BoardBridge board;
 FirmwareBridge firmware;
public slots:
 void qmlEngineAvailable(QQmlEngine *engine) {
  engine->rootContext()->setContextProperty("toolBridge",&tools);
  engine->rootContext()->setContextProperty("repairBridge",&repair);
  engine->rootContext()->setContextProperty("workshopBridge",&workshop);
  engine->rootContext()->setContextProperty("boardBridge",&board);
  engine->rootContext()->setContextProperty("firmwareBridge",&firmware);
 }
};
QUICK_TEST_MAIN_WITH_SETUP(mr_ui,Setup)
#include "ui_tests.moc"
