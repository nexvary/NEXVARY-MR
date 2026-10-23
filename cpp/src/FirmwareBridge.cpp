#include "FirmwareBridge.hpp"
#include "AvbMetadata.hpp"
#include "RecoveryFile.hpp"
#include <QSaveFile>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPromise>
#include <QRegularExpression>
#include <QUrl>
#include <QtConcurrent/QtConcurrentRun>

namespace {
constexpr qint64 maxManifestBytes = 64 * 1024;
constexpr qint64 maxFirmwareBytes = qint64(64) * 1024 * 1024 * 1024;
const QString trustNotice = QStringLiteral(" ملف الوصف غير موثوق بذاته؛ التطابق لا يثبت أصالة البرنامج الثابت أو سلامة تفليشه.");
bool plainField(const QString &text, qsizetype maximum) {
  if (text.isEmpty() || text.size() > maximum || text != text.trimmed()) return false;
  for (const auto character : text) if (character.isNull() || character.category() == QChar::Other_Control) return false;
  return true;
}
}
FirmwareBridge::FirmwareBridge(QObject *parent) : QObject(parent) {
  setStatus(QStringLiteral("اختر ملف وصف وحزمة محلية للتحقق من الهوية وSHA256 فقط.") + trustNotice);
  connect(&watcher_, &QFutureWatcher<FirmwareInspection>::progressValueChanged, this, [this](int value) {
    if (progress_ == value) return;
    progress_ = value;
    emit progressChanged();
  });
  connect(&watcher_, &QFutureWatcher<FirmwareInspection>::finished, this, [this] {
    const bool canceled = invalidated_ || watcher_.future().isCanceled();
    const auto result = canceled ? FirmwareInspection{false, QStringLiteral("أُلغي التحقق"), {}} : watcher_.result();
    avbReport_ = result.avbReport;
    emit avbReportChanged();
    compatible_ = result.matched;
    busy_ = false;
    setStatus(result.message + trustNotice);
    emit compatibleChanged();
    emit busyChanged();
    emit inspectionFinished(compatible_);
  });
}
FirmwareBridge::~FirmwareBridge() {
  watcher_.cancel();
  watcher_.waitForFinished();
}
void FirmwareBridge::setStatus(const QString &text) {
  if (status_ == text) return;
  status_ = text;
  emit statusChanged();
}
void FirmwareBridge::reject(const QString &message) {
  setStatus(message + trustNotice);
  emit inspectionFinished(false);
}
FirmwareManifestValidation FirmwareBridge::validateManifest(const QByteArray &json, const QString &actualBrand, const QString &actualModel) {
  FirmwareManifestValidation result;
  auto fail = [&result](const QString &message) { result.error = message; return result; };
  if (json.isEmpty() || json.size() > maxManifestBytes) return fail(QStringLiteral("ملف الوصف فارغ أو يتجاوز 64 KiB"));
  QJsonParseError error;
  const auto document = QJsonDocument::fromJson(json, &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) return fail(QStringLiteral("ملف الوصف ليس JSON صالحًا"));
  const auto object = document.object();
  if (!object.value(QStringLiteral("schemaVersion")).isDouble() || object.value(QStringLiteral("schemaVersion")).toDouble() != 1.0) return fail(QStringLiteral("إصدار صيغة الوصف غير مدعوم"));
  const QString brand = object.value(QStringLiteral("brand")).toString();
  const QString model = object.value(QStringLiteral("model")).toString();
  result.build = object.value(QStringLiteral("build")).toString();
  result.sourceUrl = object.value(QStringLiteral("sourceUrl")).toString();
  result.expectedSha256 = object.value(QStringLiteral("sha256")).toString();
  if (!plainField(brand, 128) || !plainField(model, 128) || !plainField(result.build, 256)) return fail(QStringLiteral("حقول الشركة والموديل والإصدار مطلوبة ويجب أن تكون نصوصًا صريحة صالحة"));
  const QUrl url(result.sourceUrl, QUrl::StrictMode);
  if (result.sourceUrl.size() > 2048 || !url.isValid() || url.scheme() != QStringLiteral("https") || url.host().isEmpty() || !url.userInfo().isEmpty() || url.hasFragment()) return fail(QStringLiteral("مطلوب رابط مصدر HTTPS صالح دون بيانات دخول أو fragment"));
  static const QRegularExpression sha256(QStringLiteral("\\A[0-9a-fA-F]{64}\\z"));
  if (!sha256.match(result.expectedSha256).hasMatch()) return fail(QStringLiteral("بصمة SHA256 مطلوبة وتتكون من 64 رقمًا سداسيًا"));
  if (!plainField(actualBrand, 128) || !plainField(actualModel, 128)) return fail(QStringLiteral("هوية الجهاز الفعلية غير متاحة؛ اقرأ الشركة والموديل من الجهاز أولًا"));
  if (brand != actualBrand || model != actualModel) return fail(QStringLiteral("الشركة أو الموديل لا يطابق بيانات الجهاز حرفيًا؛ لن يُفترض أي توافق"));
  result.expectedSha256 = result.expectedSha256.toLower();
  result.valid = true;
  return result;
}
void FirmwareBridge::inspect(const QString &manifestPath, const QString &firmwarePath, const QString &actualBrand, const QString &actualModel) {
  if (busy_) { setStatus(QStringLiteral("هناك تحقق جارٍ؛ ألغِه أو انتظر اكتماله") + trustNotice); return; }
  clearAvbReport();
  compatible_ = false; progress_ = 0;
  emit compatibleChanged(); emit progressChanged();
  const QString localManifest = mr::recovery::localFilePath(manifestPath);
  const QString localFirmware = mr::recovery::localFilePath(firmwarePath);
  QFile manifest(localManifest);
  if (!QFileInfo(localManifest).isFile() || QFileInfo(localManifest).isSymLink() || !mr::recovery::openRegularInput(manifest)) { reject(QStringLiteral("تعذر فتح ملف الوصف المحلي")); return; }
  const auto validation = validateManifest(manifest.read(maxManifestBytes + 1), actualBrand, actualModel);
  if (!validation.valid) { reject(validation.error); return; }
  const QFileInfo package(localFirmware);
  if (localFirmware.isEmpty() || !package.isFile() || package.isSymLink() || package.size() <= 0 || package.size() > maxFirmwareBytes) { reject(QStringLiteral("مطلوب ملف حزمة محلي غير فارغ بحجم لا يتجاوز 64 GiB؛ الوصف وحده لا يجتاز التحقق")); return; }
  invalidated_ = false;
  busy_ = true;
  emit busyChanged();
  setStatus(QStringLiteral("جار حساب SHA256 محليًا دون تعديل الحزمة أو الهاتف…") + trustNotice);
  watcher_.setFuture(QtConcurrent::run([](QPromise<FirmwareInspection> &promise, const QString &path, const QString &expected) {
    promise.setProgressRange(0, 100);
    QFile file(path);
    if (!mr::recovery::openRegularInput(file)) { promise.addResult(FirmwareInspection{false, QStringLiteral("تعذر قراءة الحزمة المحلية"), {}}); return; }
    const qint64 total = file.size();
    if (total <= 0 || total > maxFirmwareBytes) { promise.addResult(FirmwareInspection{false, QStringLiteral("حجم الحزمة غير مدعوم"), {}}); return; }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    qint64 processed = 0;
    while (!file.atEnd()) {
      if (promise.isCanceled()) return;
      const QByteArray chunk = file.read(1024 * 1024);
      if (chunk.isEmpty() || file.error() != QFileDevice::NoError) { promise.addResult(FirmwareInspection{false, QStringLiteral("خطأ أثناء قراءة الحزمة"), {}}); return; }
      processed += chunk.size();
      if (processed > maxFirmwareBytes) { promise.addResult(FirmwareInspection{false, QStringLiteral("تجاوز حجم الحزمة الحد المسموح أثناء القراءة"), {}}); return; }
      hash.addData(chunk);
      promise.setProgressValue(static_cast<int>(qMin<qint64>(99, processed * 100 / total)));
    }
    if (promise.isCanceled()) return;
    if (processed != total || file.size() != total) { promise.addResult(FirmwareInspection{false, QStringLiteral("تغير حجم الحزمة أثناء القراءة؛ أعد التحقق من نسخة ثابتة"), {}}); return; }
    const bool matched = QString::fromLatin1(hash.result().toHex()) == expected;
    promise.setProgressValue(100);
    promise.addResult(FirmwareInspection{matched, matched ? QStringLiteral("تطابقت الشركة والموديل المعلنان وبصمة SHA256 للحزمة. لا يُسمح بالتفليش من هذه الأداة.") : QStringLiteral("فشلت مطابقة SHA256؛ الحزمة تختلف عن البصمة المعلنة"), {}});
  }, localFirmware, validation.expectedSha256));
}
void FirmwareBridge::cancel() {
  if (busy_) watcher_.cancel();
}

void FirmwareBridge::invalidate() {
  invalidated_ = true;
  compatible_ = false;
  progress_ = 0;
  if (busy_) watcher_.cancel();
  setStatus(QStringLiteral("تغيرت هوية الجهاز أو مدخلات الحزمة؛ يلزم تحقق جديد") + trustNotice);
  emit compatibleChanged();
  emit progressChanged();
}


QString FirmwareBridge::avbJson() const {
  return avbReport_.isEmpty() ? QString{} : QString::fromUtf8(QJsonDocument(avbReport_).toJson(QJsonDocument::Indented));
}
void FirmwareBridge::clearAvbReport() {
  if (busy_) { invalidated_ = true; watcher_.cancel(); }
  avbReport_ = {};
  emit avbReportChanged();
}
void FirmwareBridge::inspectAvb(const QString &path) {
  if (busy_) return;
  clearAvbReport();
  compatible_ = false; progress_ = 0; invalidated_ = false;
  emit compatibleChanged(); emit progressChanged();
  const QString local = mr::recovery::localFilePath(path);
  const QFileInfo info(local);
  if (local.isEmpty() || !info.isFile() || info.isSymLink() || info.size() < 256 || info.size() > AvbMetadata::maxImageBytes) {
    reject(QStringLiteral("اختر صورة vbmeta خام محلية بين 256 بايت و16 MiB / Select a raw local vbmeta image (256 bytes–16 MiB)"));
    return;
  }
  avbSourcePath_ = info.canonicalFilePath();
  busy_ = true; emit busyChanged();
  setStatus(QStringLiteral("تحليل AVB محلي للقراءة فقط / Reading untrusted AVB metadata locally"));
  watcher_.setFuture(QtConcurrent::run([](QPromise<FirmwareInspection> &promise, const QString &filePath) {
    if (promise.isCanceled()) return;
    QFile file(filePath);
    if (!mr::recovery::openRegularInput(file)) { promise.addResult(FirmwareInspection{false, QStringLiteral("AVB: file-open-failed"), {}}); return; }
    const qint64 size = file.size();
    if (size < 256 || size > AvbMetadata::maxImageBytes) { promise.addResult(FirmwareInspection{false, QStringLiteral("AVB: image-size-limit"), {}}); return; }
    const auto bytes = file.read(AvbMetadata::maxImageBytes + 1);
    if (file.error() != QFileDevice::NoError || bytes.size() != size || file.size() != size) {
      promise.addResult(FirmwareInspection{false, QStringLiteral("AVB: file-read-or-size-changed"), {}}); return;
    }
    if (promise.isCanceled()) return;
    const auto report = AvbMetadata::inspect(bytes);
    if (promise.isCanceled()) return;
    promise.addResult(FirmwareInspection{false, report.value(QStringLiteral("status")).toString() == QStringLiteral("metadata-only")
      ? QStringLiteral("AVB: بنية البيانات مقروءة؛ التوقيع والثقة والتوافق غير متحقق منها / Metadata parsed; signature, trust and compatibility unverified")
      : QStringLiteral("AVB rejected: ") + report.value(QStringLiteral("error")).toString(), report});
  }, local));
}
bool FirmwareBridge::exportAvbReport(const QString &path) {
  if (busy_ || avbReport_.isEmpty()) return false;
  const QString local = mr::recovery::localFilePath(path);
  if (local.isEmpty() || QFileInfo(local).canonicalFilePath() == avbSourcePath_) return false;
  QSaveFile output(local);
  if (!output.open(QIODevice::WriteOnly)) return false;
  const auto bytes = QJsonDocument(avbReport_).toJson(QJsonDocument::Indented);
  return output.write(bytes) == bytes.size() && output.commit();
}
