#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ToolBridge.hpp"
#include "RepairBridge.hpp"
int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName(QStringLiteral("NEXVARY MR"));
  ToolBridge tools;
  RepairBridge repair;
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty(QStringLiteral("toolBridge"), &tools);
  engine.rootContext()->setContextProperty(QStringLiteral("repairBridge"), &repair);
  engine.loadFromModule("NexvaryMR", "Main");
  if (engine.rootObjects().isEmpty()) return 1;
  return app.exec();
}
