#include "ToolBridge.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdio>

// The same executable doubles as a controlled adb child. A temporary directory
// takes priority in PATH; tests never address a real phone or invoke a real adb.
static int fakeAdb(const QStringList &args) {
  const QString mode = qEnvironmentVariable("MR_TEST_TOOL_MODE");
  if (mode == QStringLiteral("failure")) { std::fputs("USB disconnected\n", stderr); return 7; }
  if (mode == QStringLiteral("hang")) QThread::msleep(30000);
  if (mode == QStringLiteral("slow")) QThread::msleep(11600);
  if (mode == QStringLiteral("flood")) {
    const QByteArray flood(600 * 1024, 'x');
    std::fwrite(flood.constData(), 1, static_cast<size_t>(flood.size()), stdout);
    return 0;
  }
  if (args == QStringList{QStringLiteral("devices"), QStringLiteral("-l")}) {
    std::fputs("List of devices attached\nDEVICE42\tdevice product:test model:Test_Phone transport_id:1\nLOCKED\tunauthorized\n", stdout);
    return 0;
  }
  if (args == QStringList{QStringLiteral("-s"), QStringLiteral("DEVICE42"), QStringLiteral("shell"), QStringLiteral("getprop")}) {
    std::fputs("[ro.product.model]: [Test Phone]\n[sys.boot_completed]: [1]\n[gsm.sim.operator.alpha]: [PRIVATE]\n", stdout);
    return 0;
  }
  if (args == QStringList{QStringLiteral("-s"), QStringLiteral("DEVICE42"), QStringLiteral("logcat"), QStringLiteral("-d"), QStringLiteral("-t"), QStringLiteral("200"), QStringLiteral("-v"), QStringLiteral("threadtime")}) {
    std::fputs("10-16 10:00:00.000 123 123 I Test: safe bounded fixture\n", stdout);
    return 0;
  }
  std::fputs("Unexpected arguments: command allowlist test failed\n", stderr);
  return 99;
}

class ToolTests : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() {
    QVERIFY(directory_.isValid());
    oldPath_ = qgetenv("PATH");
    oldMode_ = qgetenv("MR_TEST_TOOL_MODE");
#ifdef Q_OS_WIN
    const QString name = QStringLiteral("adb.exe");
#else
    const QString name = QStringLiteral("adb");
#endif
    QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(), directory_.filePath(name)));
    QVERIFY(QFile::setPermissions(directory_.filePath(name), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
  }
  void init() {
    const QByteArray path = QFile::encodeName(directory_.path()) + QDir::listSeparator().toLatin1() + QFile::encodeName(QCoreApplication::applicationDirPath()) + QDir::listSeparator().toLatin1() + oldPath_;
    qputenv("PATH", path);
    qunsetenv("MR_TEST_TOOL_MODE");
  }
  void cleanup() { qputenv("PATH", oldPath_); qputenv("MR_TEST_TOOL_MODE", oldMode_); }
  void serialValidation() {
    QVERIFY(ToolBridge::isValidSerial(QStringLiteral("192.168.1.2:5555")));
    QVERIFY(ToolBridge::isValidSerial(QStringLiteral("emulator-5554")));
    for (const auto &bad : {QStringLiteral("-s"), QStringLiteral("abc;reboot"), QStringLiteral("a b"), QStringLiteral("abc\nreboot"), QString(129, QLatin1Char('a')), QString()}) QVERIFY(!ToolBridge::isValidSerial(bad));
  }
  void parseDevicesAndPermissions() {
    const auto devices = ToolBridge::parseAdbDevices(QStringLiteral("* daemon started successfully\nList of devices attached\nABC device product:p model:Pixel_8 transport_id:2\nABC device\nOFF offline\nAUTH unauthorized\nUSB no permissions (missing udev rules)\n-s device\nERR random\n"));
    QCOMPARE(devices.size(), 4);
    QCOMPARE(devices[0].toMap().value(QStringLiteral("model")).toString(), QStringLiteral("Pixel_8"));
    QCOMPARE(devices[3].toMap().value(QStringLiteral("state")).toString(), QStringLiteral("no permissions"));
    const auto fastboot = ToolBridge::parseFastbootDevices(QStringLiteral("ABC fastboot\nABC fastboot\nwaiting for any device\n-s fastboot\n"));
    QCOMPARE(fastboot.size(), 1);
    QVERIFY(ToolBridge::parseAdbDevices(QStringLiteral("List of devices attached\n")).isEmpty());
  }
  void propertyPrivacyAndMalformedInput() {
    const auto info = ToolBridge::parseDeviceProperties(QStringLiteral("[ro.product.model]: [Pixel 8]\n[gsm.sim.operator.alpha]: [Private]\n[ro.boot.bootreason]: [reboot,watchdog]\n[ro.product.brand: broken\n"));
    QCOMPARE(info.size(), 2);
    QCOMPARE(info.value(QStringLiteral("ro.product.model")).toString(), QStringLiteral("Pixel 8"));
    QVERIFY(!info.contains(QStringLiteral("gsm.sim.operator.alpha")));
  }
  void missingToolAndNoSelection() {
    qputenv("PATH", QByteArray());
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    bridge.scanAdb();
    QVERIFY(!bridge.busy());
    QCOMPARE(finished.size(), 1);
    QCOMPARE(finished[0][0].toBool(), false);
    bridge.startMirror();
    QVERIFY(bridge.status().contains(QStringLiteral("ADB")));
    bridge.selectDevice(QStringLiteral("unknown"));
    QVERIFY(bridge.selectedSerial().isEmpty());
  }
  void asynchronousReadOnlyFlowAndDisconnect() {
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    bridge.scanAdb();
    QVERIFY(bridge.busy());
    QVERIFY(finished.wait(5000));
    QCOMPARE(finished.last()[0].toBool(), true);
    QCOMPARE(bridge.devices().size(), 2);
    bridge.selectDevice(QStringLiteral("LOCKED"));
    bridge.readDeviceInfo();
    QVERIFY(!bridge.busy());
    bridge.selectDevice(QStringLiteral("DEVICE42"));
    bridge.readDeviceInfo();
    QVERIFY(finished.wait(5000));
    QCOMPARE(bridge.deviceInfo().value(QStringLiteral("ro.product.model")).toString(), QStringLiteral("Test Phone"));
    QVERIFY(!bridge.deviceInfo().contains(QStringLiteral("gsm.sim.operator.alpha")));
    bridge.readLogcat();
    QVERIFY(finished.wait(5000));
    QVERIFY(bridge.logcat().contains(QStringLiteral("bounded fixture")));
    qputenv("MR_TEST_TOOL_MODE", "failure");
    bridge.scanAdb();
    QVERIFY(finished.wait(5000));
    QCOMPARE(finished.last()[0].toBool(), false);
    QVERIFY(bridge.devices().isEmpty());
    QVERIFY(bridge.selectedSerial().isEmpty());
    QVERIFY(bridge.deviceInfo().isEmpty());
    QVERIFY(bridge.logcat().isEmpty());
  }
  void cancellationAndOutputLimit() {
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    qputenv("MR_TEST_TOOL_MODE", "hang");
    bridge.scanAdb();
    QTest::qWait(50);
    bridge.cancel();
    QVERIFY(finished.wait(5000));
    QVERIFY(!bridge.busy());
    QCOMPARE(finished.last()[0].toBool(), false);
    QVERIFY(bridge.status().contains(QStringLiteral("أُلغي")));
    qputenv("MR_TEST_TOOL_MODE", "flood");
    bridge.scanAdb();
    QVERIFY(finished.wait(5000));
    QCOMPARE(finished.last()[0].toBool(), false);
    QVERIFY(bridge.status().contains(QStringLiteral("512")));
    QVERIFY(bridge.status().size() < 4096);
  }
  void failedStartDoesNotLeaveBusy() {
    QTemporaryDir invalid;
    QVERIFY(invalid.isValid());
#ifdef Q_OS_WIN
    const QString path = invalid.filePath(QStringLiteral("adb.exe"));
#else
    const QString path = invalid.filePath(QStringLiteral("adb"));
#endif
    QFile bad(path);
    QVERIFY(bad.open(QIODevice::WriteOnly));
    QCOMPARE(bad.write("invalid executable\n"), qint64(19));
    bad.close();
    QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    qputenv("PATH", QFile::encodeName(invalid.path()));
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    bridge.scanAdb();
    if (finished.isEmpty()) QVERIFY(finished.wait(5000));
    QVERIFY(!bridge.busy());
    QCOMPARE(finished.last()[0].toBool(), false);
  }
  void timeoutStopsUnresponsiveChild() {
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    qputenv("MR_TEST_TOOL_MODE", "hang");
    bridge.scanAdb();
    QVERIFY(finished.wait(15000));
    QVERIFY(!bridge.busy());
    QCOMPARE(finished.last()[0].toBool(), false);
    QVERIFY(bridge.status().contains(QStringLiteral("مهلة")));
  }
  void completedCommandCannotTimeoutNextCommand() {
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    bridge.scanAdb();
    QVERIFY(finished.wait(5000));
    QTest::qWait(700);
    qputenv("MR_TEST_TOOL_MODE", "slow");
    bridge.scanAdb();
    QVERIFY(finished.wait(15000));
    QCOMPARE(finished.last()[0].toBool(), true);
    QVERIFY(!bridge.busy());
    qunsetenv("MR_TEST_TOOL_MODE");
    bridge.scanAdb();
    QVERIFY(finished.wait(5000));
    QCOMPARE(finished.last()[0].toBool(), true);
  }
private:
  QTemporaryDir directory_;
  QByteArray oldPath_;
  QByteArray oldMode_;
};
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  if (QFileInfo(app.applicationFilePath()).completeBaseName() == QStringLiteral("adb")) return fakeAdb(app.arguments().mid(1));
  ToolTests tests;
  return QTest::qExec(&tests, argc, argv);
}
#include "tool_tests.moc"
