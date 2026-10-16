#include "ToolBridge.hpp"
#include <QStandardPaths>
#include <QVariantMap>
#include <QProcessEnvironment>
#include <QTimer>
#include <QFileInfo>
ToolBridge::ToolBridge(QObject *parent) : QObject(parent) {
  connect(&process_, &QProcess::started, this, &ToolBridge::busyChanged);
  connect(&process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
    const QString output = QString::fromUtf8(process_.readAllStandardOutput()).trimmed();
    const QString errors = QString::fromUtf8(process_.readAllStandardError()).trimmed();
    setStatus(exitStatus == QProcess::NormalExit && exitCode == 0
      ? (output.isEmpty() ? QStringLiteral("اكتملت العملية دون نتائج") : output)
      : QStringLiteral("فشلت العملية: ") + (errors.isEmpty() ? QString::number(exitCode) : errors));
    emit busyChanged();
  });
  connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
    setStatus(QStringLiteral("تعذر تشغيل الأداة: ") + process_.errorString());
    emit busyChanged();
  });
  refreshTools();
}
QVariantList ToolBridge::tools() const {
  QVariantList result;
  for (const QString &name : {QStringLiteral("adb"), QStringLiteral("fastboot"), QStringLiteral("scrcpy")}) {
    const QString path = QStandardPaths::findExecutable(name);
    QVariantMap item;
    item.insert(QStringLiteral("name"), name);
    item.insert(QStringLiteral("available"), !path.isEmpty());
    item.insert(QStringLiteral("path"), path);
    result.append(item);
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
void ToolBridge::runReadOnly(const QString &program, const QStringList &arguments) {
  if (busy()) { setStatus(QStringLiteral("هناك عملية جارية بالفعل")); return; }
  const QString path = QStandardPaths::findExecutable(program);
  if (path.isEmpty()) { setStatus(QStringLiteral("الأداة غير مثبتة أو غير موجودة في PATH: ") + program); return; }
  process_.setProgram(path);
  process_.setArguments(arguments);
  process_.start();
  QTimer::singleShot(12000, this, [this]() {
    if (busy()) { process_.kill(); setStatus(QStringLiteral("انتهت مهلة الفحص")); }
  });
}
void ToolBridge::scanAdb() { runReadOnly(QStringLiteral("adb"), {QStringLiteral("devices"), QStringLiteral("-l")}); }
void ToolBridge::scanFastboot() { runReadOnly(QStringLiteral("fastboot"), {QStringLiteral("devices")}); }
void ToolBridge::startMirror() {
  if (busy()) { setStatus(QStringLiteral("انتظر اكتمال الفحص الحالي")); return; }
  const QString path = QStandardPaths::findExecutable(QStringLiteral("scrcpy"));
  if (path.isEmpty()) { setStatus(QStringLiteral("scrcpy غير مثبتة؛ استخدم النسخة الرسمية فقط")); return; }
  const bool launched = QProcess::startDetached(path, {});
  setStatus(launched ? QStringLiteral("تم طلب تشغيل نافذة scrcpy الخارجية") : QStringLiteral("تعذر تشغيل scrcpy"));
}
