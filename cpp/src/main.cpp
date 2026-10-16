#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ToolBridge.hpp"
int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName(QStringLiteral("NEXVARY MR"));
  ToolBridge tools;
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty(QStringLiteral("toolBridge"), &tools);
  engine.loadFromModule("NexvaryMR", "Main");
  if (engine.rootObjects().isEmpty()) return 1;
  return app.exec();
}
