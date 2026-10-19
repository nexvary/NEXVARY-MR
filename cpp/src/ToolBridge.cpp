#include "ToolBridge.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUrl>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

namespace {
constexpr qsizetype maxOutputBytes = 512 * 1024;
const QString adbTransport = QStringLiteral("adb");
const QString fastbootTransport = QStringLiteral("fastboot");
QString pathExecutable(const QString &name) {
#ifdef Q_OS_WIN
  if (name == QStringLiteral("pnputil")) {
    const QString root = qEnvironmentVariable("SystemRoot");
    if (!root.isEmpty()) return QDir(root).filePath(QStringLiteral("System32/pnputil.exe"));
    return {};
  }
#endif
  return QStandardPaths::findExecutable(name);
}
bool configurableTool(const QString &name) {
  return name == QStringLiteral("adb") || name == QStringLiteral("fastboot") || name == QStringLiteral("scrcpy") || name == QStringLiteral("mmls");
}
QString localAbsolutePath(const QString &input) {
  if (input.isEmpty() || input != input.trimmed() || input.startsWith(QStringLiteral("//")) || input.startsWith(QStringLiteral("\\\\"))) return {};
  for (const auto ch : input) if (ch.isNull() || ch.category() == QChar::Other_Control) return {};
  QString path = input;
  if (input.startsWith(QStringLiteral("file:"), Qt::CaseInsensitive)) {
    const QUrl url(input, QUrl::StrictMode);
    if (!url.isValid() || !url.isLocalFile() || !url.host().isEmpty() || url.hasQuery() || url.hasFragment()) return {};
    path = url.toLocalFile();
  }
  if (!QDir::isAbsolutePath(path) || path.startsWith(QStringLiteral("//")) || path.startsWith(QStringLiteral("\\\\"))) return {};
  return QDir::cleanPath(path);
}
QString validatedExecutable(const QString &input) {
  const QString path = localAbsolutePath(input);
  if (path.isEmpty()) return {};
  const QFileInfo file(path);
  if (!file.isFile() || !file.isReadable()) return {};
#ifdef Q_OS_WIN
  if (file.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0) return {};
#else
  if (!file.isExecutable()) return {};
#endif
  const QString canonical = file.canonicalFilePath();
  return localAbsolutePath(canonical);
}
}

ToolBridge::ToolBridge(QObject *parent) : QObject(parent) {
  timeoutTimer_.setSingleShot(true);
  timeoutTimer_.setTimerType(Qt::PreciseTimer);
  timeoutTimer_.setInterval(12000);
  pollTimer_.setInterval(5000);
  connect(&timeoutTimer_, &QTimer::timeout, this, [this] {
    if (!active_) return;
    terminalReason_ = QStringLiteral("انتهت مهلة الفحص؛ تحقق من اتصال USB واستجابة الأداة");
    process_.kill();
  });
  connect(&pollTimer_, &QTimer::timeout, this, [this] { if (!busy()) scanAdb(); });
  connect(&process_, &QProcess::readyReadStandardOutput, this, &ToolBridge::collectOutput);
  connect(&process_, &QProcess::readyReadStandardError, this, &ToolBridge::collectOutput);
  connect(&process_, &QProcess::finished, this, &ToolBridge::finish);
  connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
    if (!active_) return;
    if (terminalReason_.isEmpty()) terminalReason_ = QStringLiteral("تعذر إكمال تشغيل الأداة: ") + process_.errorString();
    // FailedToStart does not emit finished. Crashes do, so they use normal cleanup.
    if (error == QProcess::FailedToStart) finish(-1, QProcess::CrashExit);
  });
  refreshTools();
  initServices();
}
ToolBridge::~ToolBridge() {
  shutdownServices();
  pollTimer_.stop();
  timeoutTimer_.stop();
  if (process_.state() != QProcess::NotRunning) {
    process_.kill();
    process_.waitForFinished(1000);
  }
}
QVariantList ToolBridge::tools() const {
  QVariantList result;
  for (const auto &name : {QStringLiteral("adb"), QStringLiteral("fastboot"), QStringLiteral("scrcpy")}) {
    const QString path = resolveExecutable(name);
    result.append(QVariantMap{{QStringLiteral("name"), name}, {QStringLiteral("available"), !path.isEmpty()}, {QStringLiteral("path"), path}, {QStringLiteral("configured"), configuredTools_.contains(name)}, {QStringLiteral("configuredPath"), configuredTools_.value(name)}});
  }
  return result;
}
QVariantList ToolBridge::forensicTools() const {
 struct Candidate{const char *name;const char *executable;const char *source;const char *license;};
 static const Candidate candidates[]{
  {"The Sleuth Kit / mmls","mmls","https://github.com/sleuthkit/sleuthkit","Component-specific IPL/CPL and other notices"},
  {"Autopsy","autopsy","https://github.com/sleuthkit/autopsy","Apache-2.0 plus dependency notices"},
  {"ALEAPP","aleapp","https://github.com/abrignoni/ALEAPP","MIT plus dependency notices"},
  {"iLEAPP","ileapp","https://github.com/abrignoni/iLEAPP","MIT plus dependency notices"},
  {"PhotoRec","photorec","https://www.cgsecurity.org/","GPL; review version and dependencies"},
  {"TestDisk","testdisk","https://www.cgsecurity.org/","GPL; includes writing operations"},
  {"Volatility 3","vol","https://github.com/volatilityfoundation/volatility3","Volatility Software License 1.0"},
  {"MVT","mvt-ios","https://github.com/mvt-project/mvt","MVT License 1.1; consent restriction"},
  {"Plaso","log2timeline","https://github.com/log2timeline/plaso","Apache-2.0 plus dependency notices"},
  {"bulk_extractor","bulk_extractor","https://github.com/simsong/bulk_extractor","GPL-3.0-or-later for post-NPS project code; mixed components"}
 };
 QVariantList result;
 for(const auto &candidate:candidates){const auto name=QString::fromLatin1(candidate.executable);const auto path=resolveExecutable(name);
  result.append(QVariantMap{{"name",QString::fromLatin1(candidate.name)},{"executable",name},{"path",path},{"available",!path.isEmpty()},{"configuredPath",configuredTools_.value(name)},{"configured",configuredTools_.contains(name)},{"adapter",name==QStringLiteral("mmls")},{"source",QString::fromLatin1(candidate.source)},{"license",QString::fromLatin1(candidate.license)}});
 }
 return result;
}
QString ToolBridge::resolveExecutable(const QString &name) const {
  if (configurableTool(name) && configuredTools_.contains(name)) return validatedExecutable(configuredTools_.value(name));
  return pathExecutable(name);
}
bool ToolBridge::persistToolConfiguration(const QMap<QString, QString> &paths) {
  if (toolConfigurationPath_.isEmpty()) return true; // Unit tests and unconfigured instances stay ephemeral.
  QJsonObject toolsObject;
  for (auto it = paths.cbegin(); it != paths.cend(); ++it) toolsObject.insert(it.key(), it.value());
  const QByteArray json = QJsonDocument(QJsonObject{{QStringLiteral("schemaVersion"), 1}, {QStringLiteral("tools"), toolsObject}}).toJson();
  QSaveFile file(toolConfigurationPath_);
  if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit()) {
    setStatus(QStringLiteral("تعذر حفظ إعداد مسارات الأدوات؛ لم يُطبّق التغيير"));
    return false;
  }
  return true;
}
bool ToolBridge::configureTool(const QString &name, const QString &localPath) {
  if (busy() || !configurableTool(name)) { setStatus(QStringLiteral("اسم أداة غير مدعوم أو هناك فحص جارٍ")); return false; }
  const QString path = validatedExecutable(localPath);
  if (path.isEmpty()) { setStatus(QStringLiteral("اختر ملفًا تنفيذيًا محليًا صالحًا دون وسائط أو رابط شبكة")); return false; }
  auto updated = configuredTools_;
  updated.insert(name, path);
  if (!persistToolConfiguration(updated)) return false;
  configuredTools_ = updated;
  if (name == adbTransport || name == fastbootTransport) replaceDevices({}, name);
  refreshTools(); // Selection never starts the executable.
  return true;
}
bool ToolBridge::clearTool(const QString &name) {
  if (busy() || !configurableTool(name)) { setStatus(QStringLiteral("اسم أداة غير مدعوم أو هناك فحص جارٍ")); return false; }
  auto updated = configuredTools_;
  updated.remove(name);
  if (!persistToolConfiguration(updated)) return false;
  configuredTools_ = updated;
  if (name == adbTransport || name == fastbootTransport) replaceDevices({}, name);
  refreshTools();
  return true;
}
bool ToolBridge::loadToolConfiguration(const QString &configPath) {
  if (busy()) return false;
  const QString path = localAbsolutePath(configPath);
  if (path.isEmpty()) { setStatus(QStringLiteral("مسار إعداد الأدوات يجب أن يكون ملفًا محليًا مطلقًا")); return false; }
  // Remember an explicitly chosen destination even if its old contents are
  // corrupt: a later user selection can repair it and must not silently become ephemeral.
  toolConfigurationPath_ = path;
  QMap<QString, QString> loaded;
  if (QFileInfo::exists(path)) {
    QFile file(path);
    if (!QFileInfo(path).isFile() || !file.open(QIODevice::ReadOnly) || file.size() > 65536) { setStatus(QStringLiteral("تعذر قراءة إعداد الأدوات أو تجاوز الحد المسموح")); return false; }
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.read(65537), &error);
    const auto object = document.object();
    if (error.error != QJsonParseError::NoError || !document.isObject() || object.size() != 2 || object.value(QStringLiteral("schemaVersion")).toDouble() != 1.0 || !object.value(QStringLiteral("tools")).isObject()) {
      setStatus(QStringLiteral("صيغة إعداد الأدوات غير صالحة")); return false;
    }
    const auto entries = object.value(QStringLiteral("tools")).toObject();
    for (auto it = entries.constBegin(); it != entries.constEnd(); ++it) {
      const QString executablePath = it.value().isString() ? validatedExecutable(it.value().toString()) : QString();
      if (!configurableTool(it.key()) || executablePath.isEmpty()) { setStatus(QStringLiteral("إعداد الأدوات يتضمن اسمًا أو مسارًا غير مسموح")); return false; }
      loaded.insert(it.key(), executablePath);
    }
  }
  configuredTools_ = loaded;
  replaceDevices({}, adbTransport);
  replaceDevices({}, fastbootTransport);
  toolConfigurationPath_ = path;
  refreshTools();
  return true;
}
void ToolBridge::setStatus(const QString &value) {
  if (status_ == value) return;
  status_ = value;
  emit statusChanged();
}
void ToolBridge::refreshTools() {
  emit toolsChanged();
  setStatus(QStringLiteral("تم فحص مسارات الأدوات المحددة والأدوات المتاحة في PATH"));
}
bool ToolBridge::isValidSerial(const QString &serial) {
  static const QRegularExpression valid(QStringLiteral("\\A[A-Za-z0-9][A-Za-z0-9_.:-]{0,127}\\z"));
  return valid.match(serial).hasMatch();
}
QVariantList ToolBridge::parseAdbDevices(const QString &output) {
  QVariantList result;
  QSet<QString> seen;
  for (const auto &line : output.split(QLatin1Char('\n'))) {
    const auto fields = line.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (fields.size() < 2 || !isValidSerial(fields[0]) || seen.contains(fields[0])) continue;
    const QString state = fields[1];
    if (state != QStringLiteral("device") && state != QStringLiteral("offline") && state != QStringLiteral("unauthorized") && state != QStringLiteral("recovery") && state != QStringLiteral("sideload") && state != QStringLiteral("rescue") && state != QStringLiteral("bootloader") && !(state == QStringLiteral("no") && fields.value(2) == QStringLiteral("permissions"))) continue;
    QVariantMap item{{QStringLiteral("serial"), fields[0]}, {QStringLiteral("state"), state == QStringLiteral("no") ? QStringLiteral("no permissions") : state}, {QStringLiteral("transport"), adbTransport}, {QStringLiteral("model"), QString()}, {QStringLiteral("product"), QString()}};
    for (qsizetype index = 2; index < fields.size(); ++index) {
      const auto split = fields[index].indexOf(QLatin1Char(':'));
      if (split <= 0) continue;
      const auto key = fields[index].left(split);
      if (key == QStringLiteral("model") || key == QStringLiteral("product") || key == QStringLiteral("device") || key == QStringLiteral("transport_id")) item.insert(key, fields[index].mid(split + 1));
    }
    result.append(item);
    seen.insert(fields[0]);
    if (result.size() >= 256) break;
  }
  return result;
}
QVariantList ToolBridge::parseFastbootDevices(const QString &output) {
  QVariantList result;
  QSet<QString> seen;
  for (const auto &line : output.split(QLatin1Char('\n'))) {
    const auto fields = line.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (fields.size() < 2 || fields[1] != fastbootTransport || !isValidSerial(fields[0]) || seen.contains(fields[0])) continue;
    result.append(QVariantMap{{QStringLiteral("serial"), fields[0]}, {QStringLiteral("state"), fastbootTransport}, {QStringLiteral("transport"), fastbootTransport}, {QStringLiteral("model"), QString()}});
    seen.insert(fields[0]);
    if (result.size() >= 256) break;
  }
  return result;
}
QVariantMap ToolBridge::parseDeviceProperties(const QString &output) {
  static const QSet<QString> allowed{
    QStringLiteral("ro.product.manufacturer"), QStringLiteral("ro.product.brand"), QStringLiteral("ro.product.model"), QStringLiteral("ro.product.device"), QStringLiteral("ro.product.name"),
    QStringLiteral("ro.build.version.release"), QStringLiteral("ro.build.version.sdk"), QStringLiteral("ro.build.version.security_patch"), QStringLiteral("ro.build.fingerprint"),
    QStringLiteral("ro.bootloader"), QStringLiteral("ro.boot.bootreason"), QStringLiteral("ro.bootmode"), QStringLiteral("ro.boot.verifiedbootstate"), QStringLiteral("ro.boot.flash.locked"),
    QStringLiteral("sys.boot_completed"), QStringLiteral("init.svc.bootanim"), QStringLiteral("ro.hardware"), QStringLiteral("ro.soc.manufacturer"), QStringLiteral("ro.soc.model")};
  static const QRegularExpression property(QStringLiteral("^\\[([^]\\r\\n]+)\\]: \\[([^\\r\\n]*)\\]$"));
  QVariantMap result;
  for (const auto &line : output.split(QLatin1Char('\n'))) {
    const auto match = property.match(line.trimmed());
    if (match.hasMatch() && allowed.contains(match.captured(1))) result.insert(match.captured(1), match.captured(2).left(4096));
  }
  return result;
}
bool ToolBridge::runReadOnly(const QString &program, const QStringList &arguments, Operation operation) {
  if (active_ || (serviceRunning_ && operation != Operation::Service)) { setStatus(QStringLiteral("هناك عملية جارية بالفعل")); return false; }
  if (operation == Operation::Properties) { deviceInfo_.clear(); emit deviceInfoChanged(); }
  if (operation == Operation::Logcat) { logcat_.clear(); emit logcatChanged(); }
  if (operation == Operation::Drivers) { driverInfo_.clear(); emit driverInfoChanged(); }
  const QString path = resolveExecutable(program);
  if (path.isEmpty()) {
    if (operation == Operation::AdbScan) replaceDevices({}, adbTransport);
    if (operation == Operation::FastbootScan) replaceDevices({}, fastbootTransport);
    setStatus(QStringLiteral("الأداة غير مثبتة أو غير موجودة في PATH: ") + program);
    if(operation!=Operation::Service)emit operationFinished(false);
    return false;
  }
  output_.clear(); errors_.clear(); terminalReason_.clear();
  operation_ = operation;
  commandSerial_ = selectedSerial_;
  active_ = true;
  emit busyChanged();
  setStatus(operation==Operation::Service?QStringLiteral("جار تنفيذ خدمة الجهاز المحددة؛ راقب الهاتف…"):QStringLiteral("جار تنفيذ فحص قراءة فقط…"));
  process_.setProgram(path);
  process_.setArguments(arguments);
  timeoutTimer_.start();
  process_.start();
  return true;
}
void ToolBridge::collectOutput() {
  const QByteArray out = process_.readAllStandardOutput();
  const QByteArray err = process_.readAllStandardError();
  if (!active_) return;
  const qsizetype room = qMax<qsizetype>(0, maxOutputBytes - output_.size() - errors_.size());
  output_.append(out.left(room));
  errors_.append(err.left(qMax<qsizetype>(0, room - out.size())));
  if (operation_ == Operation::Service && serviceCurrent_.key == QStringLiteral("execute")) {
    serviceOutput_ = QString::fromUtf8(output_ + errors_).left(8192);
    emit serviceChanged();
  }
  if (out.size() + err.size() > room && terminalReason_.isEmpty()) {
    terminalReason_ = QStringLiteral("تجاوزت نتيجة الفحص الحد المسموح (512 KiB)؛ أُوقف لحماية الذاكرة");
    process_.kill();
  }
}
void ToolBridge::finish(int exitCode, QProcess::ExitStatus exitStatus) {
  if (!active_) return;
  collectOutput();
  timeoutTimer_.stop();
  const bool success = terminalReason_.isEmpty() && exitStatus == QProcess::NormalExit && exitCode == 0;
  const QString output = QString::fromUtf8(output_).trimmed();
  if (operation_ == Operation::Service) {
    active_=false;
    operation_=Operation::None;
    serviceStepFinished(success,output,QString::fromUtf8(errors_).trimmed());
    return;
  }
  if (success) {
    switch (operation_) {
    case Operation::AdbScan: replaceDevices(parseAdbDevices(output), adbTransport); break;
    case Operation::FastbootScan: replaceDevices(parseFastbootDevices(output), fastbootTransport); break;
    case Operation::Properties:
      if (commandSerial_ == selectedSerial_) { deviceInfo_ = parseDeviceProperties(output); deviceInfo_.insert(QStringLiteral("serial"), commandSerial_); emit deviceInfoChanged(); }
      break;
    case Operation::Logcat:
      if (commandSerial_ == selectedSerial_) { logcat_ = output; emit logcatChanged(); }
      break;
    case Operation::Drivers: driverInfo_ = output; emit driverInfoChanged(); break;
    case Operation::None: break;
    case Operation::Service: break;
    }
    if (operation_ == Operation::AdbScan || operation_ == Operation::FastbootScan) setStatus(QStringLiteral("اكتمل فحص الاتصال؛ عدد الأجهزة المكتشفة: ") + QString::number(devices_.size()));
    else setStatus(output.isEmpty() ? QStringLiteral("اكتمل الفحص دون بيانات متاحة") : QStringLiteral("اكتمل الفحص بنجاح"));
  } else {
    // Failed scans invalidate that transport; never leave an old authorized device usable.
    if (operation_ == Operation::AdbScan) replaceDevices({}, adbTransport);
    if (operation_ == Operation::FastbootScan) replaceDevices({}, fastbootTransport);
    setStatus(!terminalReason_.isEmpty() ? terminalReason_ : QStringLiteral("فشل الفحص (رمز ") + QString::number(exitCode) + QStringLiteral("): ") + QString::fromUtf8(errors_).trimmed().left(2048));
  }
  const QStringList operationNames{QStringLiteral("none"),QStringLiteral("adb-scan"),QStringLiteral("fastboot-scan"),QStringLiteral("properties"),QStringLiteral("logcat"),QStringLiteral("drivers")};
  emit diagnosticCompleted(operationNames.value(static_cast<int>(operation_)), success);
  active_ = false;
  operation_ = Operation::None;
  emit busyChanged();
  emit operationFinished(success);
}
void ToolBridge::replaceDevices(const QVariantList &found, const QString &transport) {
  if (!serviceRunning_) serviceInvalidate();
  QVariantList combined;
  for (const auto &entry : devices_) if (entry.toMap().value(QStringLiteral("transport")).toString() != transport) combined.append(entry);
  combined.append(found);
  if (devices_ != combined) { devices_ = combined; emit devicesChanged(); }
  bool selectedPresent = false;
  for (const auto &entry : devices_) if (entry.toMap().value(QStringLiteral("serial")).toString() == selectedSerial_) selectedPresent = true;
  if (!selectedPresent && !selectedSerial_.isEmpty()) selectDevice({});
  if (transport == adbTransport && !selectedSerial_.isEmpty()) {
    bool authorized = false;
    for (const auto &entry : devices_) {
      const auto item = entry.toMap();
      if (item.value(QStringLiteral("serial")).toString() == selectedSerial_ && item.value(QStringLiteral("transport")).toString() == adbTransport && item.value(QStringLiteral("state")).toString() == QStringLiteral("device")) authorized = true;
    }
    if (!authorized) { deviceInfo_.clear(); logcat_.clear(); emit deviceInfoChanged(); emit logcatChanged(); }
  }
}
void ToolBridge::selectDevice(const QString &serial) {
  if (serial == selectedSerial_) return;
  if (!serial.isEmpty()) {
    if (!isValidSerial(serial)) { setStatus(QStringLiteral("معرّف الجهاز غير صالح")); return; }
    bool found = false;
    for (const auto &entry : devices_) if (entry.toMap().value(QStringLiteral("serial")).toString() == serial) found = true;
    if (!found) { setStatus(QStringLiteral("اختر جهازًا من نتائج الفحص الحالية")); return; }
  }
  selectedSerial_ = serial;
  if (serviceRunning_) cancel();
  serviceInvalidate();
  deviceInfo_.clear(); logcat_.clear();
  emit selectedSerialChanged(); emit deviceInfoChanged(); emit logcatChanged();
}
bool ToolBridge::selectedAdbReady() {
  for (const auto &entry : devices_) {
    const auto item = entry.toMap();
    if (item.value(QStringLiteral("serial")).toString() == selectedSerial_ && item.value(QStringLiteral("transport")).toString() == adbTransport && item.value(QStringLiteral("state")).toString() == QStringLiteral("device")) return true;
  }
  setStatus(QStringLiteral("اختر جهاز ADB متصلًا ومصرّحًا؛ وافق على طلب USB debugging على الهاتف ثم أعد الفحص"));
  return false;
}
void ToolBridge::scanAdb() { runReadOnly(QStringLiteral("adb"), {QStringLiteral("devices"), QStringLiteral("-l")}, Operation::AdbScan); }
void ToolBridge::scanFastboot() { runReadOnly(QStringLiteral("fastboot"), {QStringLiteral("devices")}, Operation::FastbootScan); }
void ToolBridge::readDeviceInfo() {
  if (selectedAdbReady()) runReadOnly(QStringLiteral("adb"), {QStringLiteral("-s"), selectedSerial_, QStringLiteral("shell"), QStringLiteral("getprop")}, Operation::Properties);
}
void ToolBridge::readLogcat() {
  if (selectedAdbReady()) runReadOnly(QStringLiteral("adb"), {QStringLiteral("-s"), selectedSerial_, QStringLiteral("logcat"), QStringLiteral("-d"), QStringLiteral("-t"), QStringLiteral("200"), QStringLiteral("-v"), QStringLiteral("threadtime")}, Operation::Logcat);
}
void ToolBridge::inspectDrivers() {
#ifdef Q_OS_WIN
  runReadOnly(QStringLiteral("pnputil"), {QStringLiteral("/enum-devices"), QStringLiteral("/connected"), QStringLiteral("/class"), QStringLiteral("USB")}, Operation::Drivers);
#else
  driverInfo_ = QStringLiteral("فحص تعريفات Windows متاح على Windows فقط. يُستخدم PnPUtil للقراءة دون تثبيت أو تعديل أي تعريف.");
  emit driverInfoChanged(); setStatus(driverInfo_);
#endif
}
void ToolBridge::cancel() {
  if (serviceRunning_) {
    serviceCancel_->store(true);
    serviceSteps_.clear();
    if (active_) { terminalReason_=QStringLiteral("Cancelled; device changes already started cannot be undone.");process_.kill(); }
    else if (!serviceCopy_.isRunning()) serviceComplete(false,QStringLiteral("Cancelled; verify device state before continuing."));
    return;
  }
  if (!active_) return;
  terminalReason_ = QStringLiteral("أُلغي الفحص بواسطة المستخدم");
  process_.kill();
}
void ToolBridge::setAutoRefresh(bool enabled) {
  if (enabled == autoRefresh()) return;
  if (enabled) { pollTimer_.start(); if (!busy()) scanAdb(); }
  else pollTimer_.stop();
  emit autoRefreshChanged();
}
void ToolBridge::startMirror() {
  if (busy()) { setStatus(QStringLiteral("انتظر اكتمال الفحص الحالي")); return; }
  if (!selectedAdbReady()) return;
  const QString path = resolveExecutable(QStringLiteral("scrcpy"));
  if (path.isEmpty()) { setStatus(QStringLiteral("scrcpy غير مثبتة؛ استخدم النسخة الرسمية فقط")); return; }
  const bool launched = QProcess::startDetached(path, {QStringLiteral("--serial"), selectedSerial_});
  setStatus(launched ? QStringLiteral("تم طلب تشغيل scrcpy للجهاز المحدد؛ راجع نافذتها الخارجية") : QStringLiteral("تعذر تشغيل scrcpy"));
}
