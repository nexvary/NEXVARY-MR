#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>
#include <QTemporaryDir>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QCoreApplication>
#include <QQuickWindow>
#include <QImage>
#include <QQuickStyle>
#include <QtEndian>
#include <memory>
#include "ToolBridge.hpp"
#include "FirmwareZipFixture.hpp"
#include "ExifFixture.hpp"
#include "FontReadability.hpp"
#include "DeviceCompatibility.hpp"
#include "RepairBridge.hpp"
#include "WorkshopBridge.hpp"
#include "TrackingPortal.hpp"
#include "CloudTracking.hpp"
#include "SecurityBridge.hpp"
#include "ServiceCatalog.hpp"
#include "BoardBridge.hpp"
#include "FirmwareBridge.hpp"
#include "RecoveryBridge.hpp"
#include "LawCaseBridge.hpp"
#include "IdentityService.hpp"
#include "ResearchBridge.hpp"
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
 std::unique_ptr<LawCaseBridge> law;
 std::unique_ptr<IdentityService> identity;
 std::unique_ptr<ResearchBridge> research;
 QByteArray originalSyntheticModeEnv;
 bool syntheticModeEnvChanged=false;
public:
 Q_INVOKABLE bool requestFrame(QObject *object) { auto *window=qobject_cast<QQuickWindow *>(object); if(!window||!window->isVisible())return false; window->update();return true; }
 Q_INVOKABLE bool hasGlyphCoverage(const QString &text) { return mr::ui::textHasGlyphs(text,QFont(QStringLiteral("Segoe UI"))); }
 Q_INVOKABLE bool captureWindow(QObject *object,const QString &path) {
  auto *window=qobject_cast<QQuickWindow *>(object);
  return window&&!window->grabWindow().isNull()&&window->grabWindow().save(path);
 }
 Q_INVOKABLE bool setSyntheticDeviceMode(const QString &mode) {
  if(!tools||tools->busy()||!QStringList{"connected","device","unauthorized","offline","disconnected","none","error","progress"}.contains(mode))return false;
  const auto path=temp.filePath(QStringLiteral("ui-device-mode.txt"));
  QSaveFile file(path);
  const auto bytes=mode.toUtf8();
  if(!file.open(QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.commit())return false;
  if(!syntheticModeEnvChanged) {
   originalSyntheticModeEnv=qgetenv("MR_UI_DEVICE_MODE");syntheticModeEnvChanged=true;
  }
  qputenv("MR_UI_DEVICE_MODE",QFile::encodeName(path));
  if(!tools->configureTool(QStringLiteral("adb"),QStringLiteral(MR_UI_DEVICE_FIXTURE_PATH)))return false;
  tools->scanAdb();return tools->busy();
 }
 Q_INVOKABLE bool clearSyntheticTool() {
  // Callers must complete a disconnected scan before clearing the override.
  // Clearing it never starts a PATH executable or performs another discovery.
  if(!tools||tools->busy()||!tools->devices().isEmpty()||!tools->selectedSerial().isEmpty())return false;
  return tools->clearTool(QStringLiteral("adb"));
 }
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
  identity=std::make_unique<IdentityService>(temp.filePath("identity.dat"),true);
  // Explicit synthetic credentials only; production never provisions defaults.
  const QString fixturePassword=QStringLiteral("Synthetic-only UI password 2026!");
  if(!identity->bootstrap("ui-admin","Synthetic UI administrator",fixturePassword)||!identity->login("ui-admin",fixturePassword)||identity->createUser("ui-investigator","Synthetic UI investigator","investigator",fixturePassword).isEmpty()||!identity->login("ui-investigator",fixturePassword))qFatal("Synthetic identity fixture failed");
  law=std::make_unique<LawCaseBridge>(temp.filePath("law.db"),tools.get(),true,nexvary::law::Role::Investigator,nullptr,identity.get());
  research=std::make_unique<ResearchBridge>(temp.filePath("research.json"),tools.get(),firmware.get(),nullptr,identity.get());
 }
 void qmlEngineAvailable(QQmlEngine *engine) {
  const auto fixture=temp.filePath(QStringLiteral("synthetic-evidence.txt"));
  QFile file(fixture);if(file.open(QIODevice::WriteOnly))file.write("SYNTHETIC UI EVIDENCE ONLY\n");file.close();
  const auto avb=temp.filePath(QStringLiteral("synthetic-vbmeta.img"));
  QByteArray avbBytes(320, '\0'); avbBytes.replace(0,4,"AVB0");
  qToBigEndian<quint32>(1,avbBytes.data()+4); qToBigEndian<quint64>(64,avbBytes.data()+20);
  qToBigEndian<quint64>(16,avbBytes.data()+104); qToBigEndian<quint64>(2,avbBytes.data()+256);
  QFile avbFile(avb);if(!avbFile.open(QIODevice::WriteOnly)||avbFile.write(avbBytes)!=avbBytes.size())qFatal("Cannot create synthetic AVB UI fixture");avbFile.close();
  engine->rootContext()->setContextProperty("syntheticAvbPath",avb);
  engine->rootContext()->setContextProperty("avbScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-avb-export-ui-"));
  const auto package=temp.filePath(QStringLiteral("synthetic-firmware.zip"));
  QFile packageFile(package);if(packageFile.open(QIODevice::WriteOnly))packageFile.write(syntheticFirmwareZip());packageFile.close();
  engine->rootContext()->setContextProperty("syntheticFirmwarePackagePath",package);
  engine->rootContext()->setContextProperty("packageScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-package-ui-"));
  const auto capture=temp.filePath(QStringLiteral("synthetic-ui.pcap"));
  QFile captureFile(capture);if(captureFile.open(QIODevice::WriteOnly))captureFile.write(QByteArray::fromHex("d4c3b2a1020004000000000000000000ffff00000100000001000000000000000300000003000000616263"));captureFile.close();
  const auto modern=mr::service::deviceCompatibility({{"transport","adb"},{"ro.build.version.sdk","37"},{"ro.build.version.codename","REL"},{"ro.build.version.preview_sdk","0"},{"ro.boot.dynamic_partitions","true"},{"ro.virtual_ab.enabled","true"},{"observation:page-size","16384"},{"ro.crypto.type","file"},{"ro.crypto.state","encrypted"}});
  engine->rootContext()->setContextProperty("syntheticModernCompatibility",modern);
  engine->rootContext()->setContextProperty("modernScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-modern-ui-"));
  engine->rootContext()->setContextProperty("workshopScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-workshop-barcode-ui-"));
  engine->rootContext()->setContextProperty("serviceScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-service-scroll-"));
  engine->rootContext()->setContextProperty("uiEvidence",this);
  engine->rootContext()->setContextProperty("uiScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-professional-ui-"));
  engine->rootContext()->setContextProperty("uiScaleTag",qEnvironmentVariable("QT_SCALE_FACTOR",QStringLiteral("1")));
  engine->rootContext()->setContextProperty("pcapScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-pcap-ui-"));
  engine->rootContext()->setContextProperty("syntheticCapturePath",capture);
  const auto exif=temp.filePath(QStringLiteral("synthetic-ui.jpg"));
  QFile exifFile(exif);if(!exifFile.open(QIODevice::WriteOnly)||exifFile.write(syntheticExifJpeg())<0)qFatal("Cannot create Exif UI fixture");exifFile.close();
  engine->rootContext()->setContextProperty("syntheticExifPath",exif);
  engine->rootContext()->setContextProperty("exifScreenshotBase",QDir(QCoreApplication::applicationDirPath()).filePath("smoke-exif-ui-"));
  engine->rootContext()->setContextProperty("syntheticEvidencePath",fixture);
  engine->rootContext()->setContextProperty("syntheticWorkingCopyPath",temp.filePath("government-working-copy"));
  engine->rootContext()->setContextProperty("lawCaseBridge",law.get());
  engine->rootContext()->setContextProperty("identityBridge",identity.get());
  engine->rootContext()->setContextProperty("researchBridge",research.get());
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
  research.reset();law.reset();identity.reset();catalog.reset();security.reset();cloud.reset();portal.reset();recovery.reset();firmware.reset();board.reset();workshop.reset();repair.reset();tools.reset();
  if(syntheticModeEnvChanged) {
   if(originalSyntheticModeEnv.isNull())qunsetenv("MR_UI_DEVICE_MODE");else qputenv("MR_UI_DEVICE_MODE",originalSyntheticModeEnv);
  }
 }
};
QUICK_TEST_MAIN_WITH_SETUP(mr_ui,Setup)
#include "ui_tests.moc"

