#include "ToolBridge.hpp"
#include "UsbDriver.hpp"
#include <QJsonArray>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
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
  if (mode == QStringLiteral("slow")) QThread::msleep(10000);
  if (mode == QStringLiteral("flood")) {
    const QByteArray flood(600 * 1024, 'x');
    std::fwrite(flood.constData(), 1, static_cast<size_t>(flood.size()), stdout);
    return 0;
  }
  if (mode == "duplicate" && args == QStringList{"devices","-l"}) {
    std::fputs("List of devices attached\nDEVICE42 device\nDEVICE42 device\n",stdout);return 0;
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
  void diagnosisUsesCompletedDiscoveryAndInvalidatesFailures() {
    auto has=[](const QVariantMap &report,const QString &id){
      for(const auto &entry:report.value("findings").toList())if(entry.toMap().value("id").toString()==id)return true;
      return false;
    };
    ToolBridge bridge;bridge.scanAdb();QVERIFY(bridge.busy());
    QTRY_VERIFY_WITH_TIMEOUT(!bridge.busy(),5000);
    bridge.selectDevice("DEVICE42");QVERIFY(has(bridge.connectionDiagnosis(),"adb-observed"));
    bridge.selectDevice("LOCKED");QVERIFY(has(bridge.connectionDiagnosis(),"adb-unauthorized"));
    QVERIFY(!has(bridge.connectionDiagnosis(),"adb-observed"));
    qputenv("MR_TEST_TOOL_MODE","failure");bridge.scanAdb();
    QVERIFY(!has(bridge.connectionDiagnosis(),"adb-unauthorized"));
    QTRY_VERIFY_WITH_TIMEOUT(!bridge.busy(),5000);
    QVERIFY(has(bridge.connectionDiagnosis(),"adb-failed"));
    QVERIFY(!has(bridge.connectionDiagnosis(),"no-transport-device"));
    QVERIFY(bridge.configureTool("adb",QCoreApplication::applicationFilePath()));
    QVERIFY(!has(bridge.connectionDiagnosis(),"adb-failed"));
  }
  void usbSerialAndParentCorrelation() {
    const QJsonArray nodes{
      QJsonObject{{"instanceId","USB\\VID_1234&PID_5678\\DEVICE42"},{"name","Phone"}},
      QJsonObject{{"instanceId","USB\\VID_1234&PID_5678&MI_01\\6&ABC&0&0001"},{"parentId","USB\\VID_1234&PID_5678\\DEVICE42"},{"service","WinUSB"},{"infName","oem42.inf"},{"signed",true}},
      QJsonObject{{"instanceId","USB\\VID_1111&PID_2222\\OTHER"},{"name","Other phone"}}};
    const auto bytes=QJsonDocument(QJsonObject{{"schemaVersion",1},{"devices",nodes},{"signatureInventoryAvailable",true}}).toJson();
    const auto result=mr::usb::correlate(bytes,"device42");
    QCOMPARE(result.value("state").toString(),QString("serial-correlated"));
    QCOMPARE(result.value("nodes").toList().size(),2);
    QVERIFY(!result.value("interfaceBindingVerified").toBool());
    QCOMPARE(mr::usb::correlate(bytes,"DEVICE4").value("state").toString(),QString("no-match"));
    QCOMPARE(mr::usb::correlate(bytes,"192.168.1.2:5555").value("state").toString(),QString("unsupported-serial"));
  }
  void usbAmbiguityAndMalformedInventory() {
    QJsonArray nodes{QJsonObject{{"instanceId","USB\\VID_1234&PID_5678\\SAME"}},QJsonObject{{"instanceId","USB\\VID_0001&PID_0002\\SAME"}}};
    auto encode=[&]{return QJsonDocument(QJsonObject{{"schemaVersion",1},{"devices",nodes}}).toJson();};
    QCOMPARE(mr::usb::correlate(encode(),"SAME").value("state").toString(),QString("ambiguous"));
    nodes.append(nodes.first());
    QCOMPARE(mr::usb::correlate(encode(),"SAME").value("state").toString(),QString("unavailable"));
    QCOMPARE(mr::usb::correlate("not JSON","SAME").value("state").toString(),QString("unavailable"));
    QCOMPARE(mr::usb::correlate(QByteArray(513*1024,'x'),"SAME").value("state").toString(),QString("unavailable"));
  }
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
  void configureToolsAndPersistence() {
#ifdef Q_OS_WIN
    const QString executablePath = directory_.filePath(QStringLiteral("adb.exe"));
#else
    const QString executablePath = directory_.filePath(QStringLiteral("adb"));
#endif
    const QString configPath = directory_.filePath(QStringLiteral("tool-selection.json"));
    QFile::remove(configPath);
    ToolBridge bridge;
    QSignalSpy finished(&bridge, &ToolBridge::operationFinished);
    QVERIFY(bridge.loadToolConfiguration(configPath));
    QVERIFY(bridge.configureTool(QStringLiteral("adb"), QUrl::fromLocalFile(executablePath).toString()));
    QVERIFY(!bridge.busy());
    QVERIFY(finished.isEmpty()); // Selecting and loading files never runs them.
    QCOMPARE(bridge.tools()[0].toMap().value(QStringLiteral("configured")).toBool(), true);
    QCOMPARE(bridge.tools()[0].toMap().value(QStringLiteral("path")).toString(), QFileInfo(executablePath).canonicalFilePath());
    QVERIFY(QFileInfo::exists(configPath));
    ToolBridge separate;
    QVERIFY(!separate.tools()[0].toMap().value(QStringLiteral("configured")).toBool());
    QVERIFY(separate.loadToolConfiguration(configPath));
    QVERIFY(separate.tools()[0].toMap().value(QStringLiteral("configured")).toBool());
    separate.scanAdb();
    QSignalSpy separateFinished(&separate, &ToolBridge::operationFinished);
    QVERIFY(separateFinished.wait(5000));
    QCOMPARE(separate.devices().size(), 2);
    QVERIFY(separate.clearTool(QStringLiteral("adb")));
    QVERIFY(!separate.tools()[0].toMap().value(QStringLiteral("configured")).toBool());
    ToolBridge cleared;
    QVERIFY(cleared.loadToolConfiguration(configPath));
    QVERIFY(!cleared.tools()[0].toMap().value(QStringLiteral("configured")).toBool());
  }
  void rejectUnsafeToolConfiguration() {
#ifdef Q_OS_WIN
    const QString executablePath = directory_.filePath(QStringLiteral("adb.exe"));
#else
    const QString executablePath = directory_.filePath(QStringLiteral("adb"));
#endif
    ToolBridge bridge;
    QVERIFY(!bridge.configureTool(QStringLiteral("pnputil"), executablePath));
    QVERIFY(!bridge.clearTool(QStringLiteral("unknown")));
    for (const auto &bad : {QStringLiteral("https://example.com/adb.exe"), QStringLiteral("file://remote/share/adb.exe"), QStringLiteral("adb"), executablePath + QStringLiteral(" --version"), directory_.path(), QStringLiteral("//remote/share/adb.exe")}) QVERIFY(!bridge.configureTool(QStringLiteral("adb"), bad));
    QVERIFY(!bridge.tools()[0].toMap().value(QStringLiteral("configured")).toBool());
    const QString badConfig = directory_.filePath(QStringLiteral("unsafe-tools.json"));
    QFile file(badConfig);
    QVERIFY(file.open(QIODevice::WriteOnly));
    const auto data = QJsonDocument(QJsonObject{{QStringLiteral("schemaVersion"), 1}, {QStringLiteral("tools"), QJsonObject{{QStringLiteral("pnputil"), executablePath}}}}).toJson();
    QCOMPARE(file.write(data), qint64(data.size())); file.close();
    QVERIFY(!bridge.loadToolConfiguration(badConfig));
    QVERIFY(!bridge.loadToolConfiguration(QStringLiteral("https://example.com/config.json")));
    QVERIFY(!bridge.loadToolConfiguration(directory_.path()));
  }
  void serialValidation() {
    QVERIFY(ToolBridge::isValidSerial(QStringLiteral("192.168.1.2:5555")));
    QVERIFY(ToolBridge::isValidSerial(QStringLiteral("emulator-5554")));
    for (const auto &bad : {QStringLiteral("-s"), QStringLiteral("abc;reboot"), QStringLiteral("a b"), QStringLiteral("abc\nreboot"), QString(129, QLatin1Char('a')), QString()}) QVERIFY(!ToolBridge::isValidSerial(bad));
  }
  void parseDevicesAndPermissions() {
    const auto devices = ToolBridge::parseAdbDevices(QStringLiteral("* daemon started successfully\nList of devices attached\nABC device product:p model:Pixel_8 transport_id:2\nABC device\nOFF offline\nAUTH unauthorized\nUSB no permissions (missing udev rules)\n-s device\nERR random\n"));
    QCOMPARE(devices.size(), 4);
    QCOMPARE(devices[0].toMap().value(QStringLiteral("model")).toString(), QStringLiteral("Pixel_8"));
    QCOMPARE(devices[0].toMap().value("state").toString(),QStringLiteral("ambiguous"));
    QCOMPARE(devices[3].toMap().value(QStringLiteral("state")).toString(), QStringLiteral("no permissions"));
    const auto fastboot = ToolBridge::parseFastbootDevices(QStringLiteral("ABC fastboot\nABC fastboot\nwaiting for any device\n-s fastboot\n"));
    QCOMPARE(fastboot.size(), 1);
    QCOMPARE(fastboot[0].toMap().value("state").toString(),QStringLiteral("ambiguous"));
    QVERIFY(ToolBridge::parseAdbDevices(QStringLiteral("List of devices attached\n")).isEmpty());
  }
  void duplicateBeyondDisplayLimitRemainsAmbiguous() {
    QString adb="TARGET device\n",fast="TARGET fastboot\n";
    for(int i=0;i<300;++i){adb+=QString("ROW%1 device\n").arg(i);fast+=QString("ROW%1 fastboot\n").arg(i);}
    adb+="TARGET device\n";fast+="TARGET fastboot\n";
    const auto a=ToolBridge::parseAdbDevices(adb),f=ToolBridge::parseFastbootDevices(fast);
    QCOMPARE(a.size(),256);QCOMPARE(f.size(),256);
    QCOMPARE(a.first().toMap().value("state").toString(),QStringLiteral("ambiguous"));
    QCOMPARE(f.first().toMap().value("state").toString(),QStringLiteral("ambiguous"));
  }
  void propertyPrivacyAndMalformedInput() {
    const auto info = ToolBridge::parseDeviceProperties(QStringLiteral("[ro.product.model]: [Pixel 8]\n[gsm.sim.operator.alpha]: [Private]\n[ro.boot.bootreason]: [reboot,watchdog]\n[ro.product.brand: broken\n"));
    QCOMPARE(info.size(), 2);
    QCOMPARE(info.value(QStringLiteral("ro.product.model")).toString(), QStringLiteral("Pixel 8"));
    QVERIFY(!info.contains(QStringLiteral("gsm.sim.operator.alpha")));
  }
  void ambiguousSerialCannotReadPropertiesOrLogs() {
    qputenv("MR_TEST_TOOL_MODE","duplicate");ToolBridge bridge;QSignalSpy finished(&bridge,&ToolBridge::operationFinished);
    bridge.scanAdb();QVERIFY(finished.wait(5000));QCOMPARE(bridge.devices().size(),1);
    bridge.selectDevice("DEVICE42");finished.clear();bridge.readDeviceInfo();QVERIFY(!bridge.busy());
    bridge.readLogcat();QVERIFY(!bridge.busy());QVERIFY(finished.isEmpty());QVERIFY(bridge.deviceInfo().isEmpty());
    QVERIFY(!bridge.startService("probe","DEVICE42",{{"authorized",true}}));QVERIFY(!bridge.busy());
  }
  void manualRefreshRetainsDatedSnapshotUntilSuccessOrFailure(){
    ToolBridge b;QSignalSpy done(&b,&ToolBridge::operationFinished);b.scanAdb();QVERIFY(done.wait(5000));b.selectDevice("DEVICE42");done.clear();b.readDeviceInfo();QVERIFY(done.wait(5000));
    const auto before=b.deviceInfo();const auto at=b.deviceIntelligence().value("propertiesObservedAtUtc");
    qputenv("MR_TEST_TOOL_MODE","hang");done.clear();b.readDeviceInfo();QVERIFY(b.busy());QCOMPARE(b.deviceInfo(),before);QCOMPARE(b.deviceIntelligence().value("propertiesObservedAtUtc"),at);
    b.cancel();QVERIFY(done.wait(5000));QVERIFY(!done.last()[0].toBool());QVERIFY(b.deviceInfo().isEmpty());QVERIFY(b.deviceIntelligence().value("propertiesObservedAtUtc").toString().isEmpty());
  }
  void backgroundRefreshDoesNotPulseForegroundOrRebuildIdentity(){
    ToolBridge b;QSignalSpy completed(&b,&ToolBridge::operationFinished);b.scanAdb();QVERIFY(completed.wait(5000));b.selectDevice("DEVICE42");completed.clear();b.readDeviceInfo();QVERIFY(completed.wait(5000));
    const auto info=b.deviceInfo();const auto status=b.status();
    QSignalSpy busy(&b,&ToolBridge::busyChanged),identity(&b,&ToolBridge::deviceInfoChanged),intel(&b,&ToolBridge::deviceIntelligenceChanged),done(&b,&ToolBridge::backgroundRefreshFinished);
    b.setAutoRefresh(true);QVERIFY(b.backgroundRefreshing());QVERIFY(!b.busy());QVERIFY(!b.foregroundBusy());QVERIFY(b.activeOperation().isEmpty());QVERIFY(done.wait(5000));QVERIFY(done.last()[0].toBool());b.setAutoRefresh(false);
    QCOMPARE(b.deviceInfo(),info);QCOMPARE(b.status(),status);QCOMPARE(busy.size(),0);QCOMPARE(identity.size(),0);QCOMPARE(intel.size(),0);
  }
  void explicitReadSupersedesSlowBackgroundWithoutFailure(){
    ToolBridge b;QSignalSpy completed(&b,&ToolBridge::operationFinished);b.scanAdb();QVERIFY(completed.wait(5000));b.selectDevice("DEVICE42");completed.clear();
    qputenv("MR_TEST_TOOL_MODE","hang");b.setAutoRefresh(true);QVERIFY(b.backgroundRefreshing());QVERIFY(!b.busy());qunsetenv("MR_TEST_TOOL_MODE");
    b.readDeviceInfo();QVERIFY(b.foregroundBusy());QCOMPARE(b.activeOperation(),QString("properties"));QVERIFY(completed.wait(5000));QVERIFY(completed.last()[0].toBool());QTRY_VERIFY_WITH_TIMEOUT(!b.backgroundRefreshing(),3000);b.setAutoRefresh(false);
    QCOMPARE(completed.size(),1);QCOMPARE(b.deviceInfo().value("ro.product.model").toString(),QString("Test Phone"));QVERIFY(!b.foregroundBusy());
  }
  void backgroundFailuresInvalidateStaleTarget_data(){QTest::addColumn<QByteArray>("mode");for(const auto &value:{"failure","hang","flood"})QTest::newRow(value)<<QByteArray(value);}
  void backgroundFailuresInvalidateStaleTarget(){
    QFETCH(QByteArray,mode);
    ToolBridge b;QSignalSpy completed(&b,&ToolBridge::operationFinished);b.scanAdb();QVERIFY(completed.wait(5000));b.selectDevice("DEVICE42");
    qputenv("MR_TEST_TOOL_MODE",mode);QSignalSpy done(&b,&ToolBridge::backgroundRefreshFinished);b.setAutoRefresh(true);QVERIFY(done.wait(6000));b.setAutoRefresh(false);
    QVERIFY(!done.last()[0].toBool());QVERIFY(b.devices().isEmpty());QVERIFY(b.selectedSerial().isEmpty());QVERIFY(!b.busy());QVERIFY(b.status().contains("failed"));
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
    QVERIFY(!bridge.mirrorActive());
    QVERIFY(bridge.screenViewAvailable());
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
    // A stale 12 s deadline from the first command would fire 9.5 s into
    // the second command. Its own deadline still has 2 s of startup margin.
    QTest::qWait(2500);
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

