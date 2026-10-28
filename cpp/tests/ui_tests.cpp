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
#include <QCryptographicHash>
#include <QJsonDocument>
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
#include "GovernmentOperationPolicy.hpp"
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
 std::unique_ptr<GovernmentOperationPolicy> operationPolicy;
 QByteArray originalSyntheticModeEnv;
 bool syntheticModeEnvChanged=false;
public:
 Q_INVOKABLE QString recordSyntheticOperation(const QString &kind) {
  const QString pass="Synthetic-only UI password 2026!";
  if(!identity->syntheticProfile()||!identity->login("ui-investigator",pass))return {};
  const auto request=operationPolicy->requestException("external-analysis","Synthetic operation UI",{},600,true);
  if(request.isEmpty()||!identity->login("ui-admin",pass)||!operationPolicy->reviewException(request,"Synthetic separate account")||!operationPolicy->decideException(request,true,"Synthetic decision")||!identity->login("ui-investigator",pass)||!operationPolicy->activateException(request))return {};
  const auto id=operationPolicy->beginOperation("external-analysis",kind);
  if(id.isEmpty()||operationPolicy->finishOperation(id,"completed",QString(64,'b'),QString(64,'a')).isEmpty())return {};
  return id;
 }
 Q_INVOKABLE bool requestFrame(QObject *object) { auto *window=qobject_cast<QQuickWindow *>(object); if(!window||!window->isVisible())return false; window->update();return true; }
 Q_INVOKABLE bool hasGlyphCoverage(const QString &text) { return mr::ui::textHasGlyphs(text,QFont(QStringLiteral("Segoe UI"))); }
 Q_INVOKABLE bool captureWindow(QObject *object,const QString &path) {
  auto *window=qobject_cast<QQuickWindow *>(object);
  return window&&!window->grabWindow().isNull()&&window->grabWindow().save(path);
 }
 Q_INVOKABLE QString headerFingerprint(QObject *object) {
  auto *window=qobject_cast<QQuickWindow *>(object);
  if(!window||window->width()<=0)return {};
  const auto frame=window->grabWindow();if(frame.isNull())return {};
  const auto header=frame.copy(0,0,frame.width(),qRound(72.0*frame.width()/window->width()));
  const QByteArray pixels(reinterpret_cast<const char *>(header.constBits()),header.sizeInBytes());
  return QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex());
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
  operationPolicy=std::make_unique<GovernmentOperationPolicy>(identity.get());
  law=std::make_unique<LawCaseBridge>(temp.filePath("law.db"),tools.get(),true,nexvary::law::Role::Investigator,nullptr,identity.get());
  law->setOperationPolicy(operationPolicy.get());
  research=std::make_unique<ResearchBridge>(temp.filePath("research.json"),tools.get(),firmware.get(),nullptr,identity.get());
 }
 void qmlEngineAvailable(QQmlEngine *engine) {
  const auto fixture=temp.filePath(QStringLiteral("synthetic-evidence.txt"));
  QFile file(fixture);if(file.open(QIODevice::WriteOnly))file.write("SYNTHETIC UI EVIDENCE ONLY\n");file.close();
  const auto timeline=temp.filePath("synthetic-timeline.json");
  QVariantMap times;for(int i=0;i<15;++i)times.insert(QString("observed-%1").arg(i),QString("2026-10-25T12:00:00+03:00"));
  times.insert("unresolved",QString("2026-10-25T12:00:00"));
  QFile timelineFile(timeline);const auto timelineBytes=QJsonDocument::fromVariant(times).toJson(QJsonDocument::Compact);
  if(!timelineFile.open(QIODevice::WriteOnly)||timelineFile.write(timelineBytes)!=timelineBytes.size())qFatal("Cannot create synthetic timeline fixture");
  timelineFile.close();
  engine->rootContext()->setContextProperty("syntheticTimelinePath",timeline);
  const auto artifactPath=temp.filePath("synthetic-artifacts.json");
  QVariantMap artifacts;for(int i=0;i<15;++i)artifacts.insert(QString("finding-%1").arg(i,2,10,QChar('0')),i==0?QString("أحمد"):QString("SYNTHETIC observation %1").arg(i));
  const auto artifactBytes=QJsonDocument::fromVariant(artifacts).toJson(QJsonDocument::Compact);QFile artifactFile(artifactPath);
  if(!artifactFile.open(QIODevice::WriteOnly)||artifactFile.write(artifactBytes)!=artifactBytes.size())qFatal("Cannot create synthetic artifact UI fixture");
  artifactFile.close();engine->rootContext()->setContextProperty("syntheticArtifactPath",artifactPath);
  QFile adbReference(MR_ADB_REFERENCE_PATH);
  if(!adbReference.open(QIODevice::ReadOnly))qFatal("Cannot read synthetic ADB reference");
  const auto adbBytes=QByteArray::fromBase64(QJsonDocument::fromJson(adbReference.readAll()).toVariant().toMap().value("backupBase64").toString().toLatin1());
  const auto adbPath=temp.filePath("synthetic-adb-v5.ab");QFile adbFile(adbPath);
  if(adbBytes.isEmpty()||!adbFile.open(QIODevice::WriteOnly)||adbFile.write(adbBytes)!=adbBytes.size())qFatal("Cannot create synthetic ADB UI fixture");
  adbFile.close();engine->rootContext()->setContextProperty("syntheticAdbBackupPath",adbPath);
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
  const auto bookmarksPath=temp.filePath("synthetic-bookmarks.json");QFile bookmarksFile(bookmarksPath);
  if(!bookmarksFile.open(QIODevice::WriteOnly)||bookmarksFile.write("{\"roots\":{\"bookmark_bar\":{\"type\":\"folder\",\"name\":\"Synthetic lab\",\"children\":[{\"type\":\"url\",\"name\":\"Authorized fixture\",\"url\":\"https://example.invalid/\",\"date_added\":\"13300000000000000\"}]}}}")<0)qFatal("Synthetic bookmarks fixture failed");
  bookmarksFile.close();engine->rootContext()->setContextProperty("syntheticBookmarksPath",bookmarksPath);
  QFile payloadInput(bookmarksPath);if(!payloadInput.open(QIODevice::ReadOnly))qFatal("Cannot read manifest fixture payload");
  const auto payloadHash=QString::fromLatin1(QCryptographicHash::hash(payloadInput.readAll(),QCryptographicHash::Sha256).toHex());
  const auto manifestPath=temp.filePath("synthetic-manifest.json");QFile manifestFile(manifestPath);
  const auto manifestBytes=QJsonDocument::fromVariant(QVariantList{QVariantMap{{"path","bookmarks.json"},{"sha256",payloadHash}}}).toJson(QJsonDocument::Compact);
  if(!manifestFile.open(QIODevice::WriteOnly)||manifestFile.write(manifestBytes)!=manifestBytes.size())qFatal("Cannot create manifest UI fixture");
  manifestFile.close();engine->rootContext()->setContextProperty("syntheticManifestPath",manifestPath);
  QFile familyFixture(MR_SQLITE_FAMILY_REFERENCE_PATH);if(!familyFixture.open(QIODevice::ReadOnly))qFatal("Missing SQLite family UI fixture");
  const auto family=QJsonDocument::fromJson(familyFixture.readAll()).toVariant().toMap();
  for(const auto &role:QStringList{"main","wal","shm"}){
   const auto name=temp.filePath("sqlite-"+role);QFile member(name);const auto bytes=QByteArray::fromBase64(family.value(role+"Base64").toString().toLatin1());
   if(!member.open(QIODevice::WriteOnly)||member.write(bytes)!=bytes.size())qFatal("Cannot create SQLite family UI evidence");
   member.close();
   engine->rootContext()->setContextProperty("syntheticSqlite"+role+"Path",name);
  }
  QFile historyFile(MR_HISTORY_REFERENCE_PATH);if(!historyFile.open(QIODevice::ReadOnly))qFatal("Missing History fixture");
  const auto history=QJsonDocument::fromJson(historyFile.readAll()).toVariant().toMap();
  for(const auto &role:QStringList{"main","wal"}){
   const auto name=temp.filePath("history-"+role);QFile member(name);const auto bytes=QByteArray::fromBase64(history.value(role+"Base64").toByteArray());
   if(!member.open(QIODevice::WriteOnly)||member.write(bytes)!=bytes.size())qFatal("History fixture write failed");
   member.close();engine->rootContext()->setContextProperty("syntheticHistory"+role+"Path",name);
  }

  QFile downloadsFile(MR_DOWNLOADS_REFERENCE_PATH);if(!downloadsFile.open(QIODevice::ReadOnly))qFatal("Missing downloads fixture");
  const auto downloads=QJsonDocument::fromJson(downloadsFile.readAll()).toVariant().toMap();
  for(const auto &role:QStringList{"main","wal","shm"}){
   const auto name=temp.filePath("downloads-"+role);QFile member(name);const auto bytes=QByteArray::fromBase64(downloads.value(role+"Base64").toByteArray());
   if(!member.open(QIODevice::WriteOnly)||member.write(bytes)!=bytes.size())qFatal("Downloads fixture write failed");
   member.close();engine->rootContext()->setContextProperty("syntheticDownloads"+role+"Path",name);
  }

  engine->rootContext()->setContextProperty("syntheticEvidencePath",fixture);
  engine->rootContext()->setContextProperty("syntheticWorkingCopyPath",temp.filePath("government-working-copy"));
  engine->rootContext()->setContextProperty("syntheticProtectedBackupPath",temp.filePath("government-case.nmrbackup"));
  engine->rootContext()->setContextProperty("operationPolicy",operationPolicy.get());
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

