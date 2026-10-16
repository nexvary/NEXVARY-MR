#include "ToolBridge.hpp"

#include <QDir>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

namespace {
constexpr qsizetype maxOutputBytes = 512 * 1024;
const QString adbTransport = QStringLiteral("adb");
const QString fastbootTransport = QStringLiteral("fastboot");
QString executable(const QString &name) {
#ifdef Q_OS_WIN
  if (name == QStringLiteral("pnputil")) {
    const QString root = qEnvironmentVariable("SystemRoot");
    if (!root.isEmpty()) return QDir(root).filePath(QStringLiteral("System32/pnputil.exe"));
    return {};
  }
#endif
  return QStandardPaths::findExecutable(name);
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
}
ToolBridge::~ToolBridge() {
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
    const QString path = executable(name);
    result.append(QVariantMap{{QStringLiteral("name"), name}, {QStringLiteral("available"), !path.isEmpty()}, {QStringLiteral("path"), path}});
  }
  return result;
}
void ToolBridge::setStatus(const QString &value) {
  if (status_ == value) return;
  status_ = value;
  emit statusChanged();
}
void ToolBridge::refreshTools() {
  emit toolsChanged();
  setStatus(QStringLiteral("تم فحص الأدوات المتاحة في PATH"));
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
  if (busy()) { setStatus(QStringLiteral("هناك عملية جارية بالفعل")); return false; }
  if (operation == Operation::Properties) { deviceInfo_.clear(); emit deviceInfoChanged(); }
  if (operation == Operation::Logcat) { logcat_.clear(); emit logcatChanged(); }
  if (operation == Operation::Drivers) { driverInfo_.clear(); emit driverInfoChanged(); }
  const QString path = executable(program);
  if (path.isEmpty()) {
    if (operation == Operation::AdbScan) replaceDevices({}, adbTransport);
    if (operation == Operation::FastbootScan) replaceDevices({}, fastbootTransport);
    setStatus(QStringLiteral("الأداة غير مثبتة أو غير موجودة في PATH: ") + program);
    emit operationFinished(false);
    return false;
  }
  output_.clear(); errors_.clear(); terminalReason_.clear();
  operation_ = operation;
  commandSerial_ = selectedSerial_;
  active_ = true;
  emit busyChanged();
  setStatus(QStringLiteral("جار تنفيذ فحص قراءة فقط…"));
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
    }
    if (operation_ == Operation::AdbScan || operation_ == Operation::FastbootScan) setStatus(QStringLiteral("اكتمل فحص الاتصال؛ عدد الأجهزة المكتشفة: ") + QString::number(devices_.size()));
    else setStatus(output.isEmpty() ? QStringLiteral("اكتمل الفحص دون بيانات متاحة") : QStringLiteral("اكتمل الفحص بنجاح"));
  } else {
    // Failed scans invalidate that transport; never leave an old authorized device usable.
    if (operation_ == Operation::AdbScan) replaceDevices({}, adbTransport);
    if (operation_ == Operation::FastbootScan) replaceDevices({}, fastbootTransport);
    setStatus(!terminalReason_.isEmpty() ? terminalReason_ : QStringLiteral("فشل الفحص (رمز ") + QString::number(exitCode) + QStringLiteral("): ") + QString::fromUtf8(errors_).trimmed().left(2048));
  }
  active_ = false;
  operation_ = Operation::None;
  emit busyChanged();
  emit operationFinished(success);
}
void ToolBridge::replaceDevices(const QVariantList &found, const QString &transport) {
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
  const QString path = executable(QStringLiteral("scrcpy"));
  if (path.isEmpty()) { setStatus(QStringLiteral("scrcpy غير مثبتة؛ استخدم النسخة الرسمية فقط")); return; }
  const bool launched = QProcess::startDetached(path, {QStringLiteral("--serial"), selectedSerial_});
  setStatus(launched ? QStringLiteral("تم طلب تشغيل scrcpy للجهاز المحدد؛ راجع نافذتها الخارجية") : QStringLiteral("تعذر تشغيل scrcpy"));
}
