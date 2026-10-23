#include "FirmwareBridge.hpp"
#include "AvbMetadata.hpp"
#include "PackagePreflight.hpp"
#include <QBuffer>
#include <QJsonArray>
#include <QStringDecoder>
#include <QDateTime>
#include "RecoveryFile.hpp"
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
bool exportNewReport(const QJsonObject &report, const QString &input, const QString &source, bool approved, bool busy) {
  const QString path = mr::recovery::localFilePath(input);
  if (!approved || busy || report.isEmpty() || path.isEmpty() || QFileInfo(path).canonicalFilePath() == source) return false;
  // NewOnly rejects existing files and aliases before any byte is written.
  QFile output(path);
  if (!output.open(QIODevice::WriteOnly | QIODevice::NewOnly)) return false;
  output.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
  const auto bytes = QJsonDocument(report).toJson(QJsonDocument::Indented);
  const bool ok = output.write(bytes) == bytes.size() && output.flush();
  output.close();
  if (!ok) QFile::remove(path);
  return ok;
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
    packageReport_ = result.packageReport;
    emit packageReportChanged();
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
  clearPackageReport();
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
  avbReport_ = {}; avbSourcePath_.clear();
  packageReport_ = {}; packageSourcePath_.clear();
  if (busy_) watcher_.cancel();
  setStatus(QStringLiteral("تغيرت هوية الجهاز أو مدخلات الحزمة؛ يلزم تحقق جديد") + trustNotice);
  emit avbReportChanged();
  emit packageReportChanged();
  emit compatibleChanged();
  emit progressChanged();
}


QString FirmwareBridge::avbJson() const {
  return avbReport_.isEmpty() ? QString{} : QString::fromUtf8(QJsonDocument(avbReport_).toJson(QJsonDocument::Indented));
}
void FirmwareBridge::clearAvbReport() {
  if (busy_) { invalidated_ = true; watcher_.cancel(); }
  avbReport_ = {}; avbSourcePath_.clear();
  emit avbReportChanged();
}
void FirmwareBridge::inspectAvb(const QString &path) {
  if (busy_) return;
  clearAvbReport();
  clearPackageReport();
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
bool FirmwareBridge::exportAvbReport(const QString &path, bool approved) {
  return exportNewReport(avbReport_, path, avbSourcePath_, approved, busy_);
}


QString FirmwareBridge::packageJson() const {
 return packageReport_.isEmpty()?QString():QString::fromUtf8(QJsonDocument(packageReport_).toJson(QJsonDocument::Indented));
}
void FirmwareBridge::clearPackageReport(){
 if(busy_){invalidated_=true;watcher_.cancel();}
 packageReport_={};packageSourcePath_.clear();emit packageReportChanged();
}
void FirmwareBridge::inspectPackage(const QString &input){
 if(busy_)return;
 clearAvbReport();clearPackageReport();compatible_=false;progress_=0;invalidated_=false;
 emit compatibleChanged();emit progressChanged();
 constexpr qint64 limit=128*1024*1024;
 const auto path=mr::recovery::localFilePath(input);const QFileInfo info(path);
 if(path.isEmpty()||!info.isFile()||info.isSymLink()||info.size()<22||info.size()>limit){reject(QStringLiteral("ZIP inspector: choose a regular local ZIP (22 bytes–128 MiB) / اختر ZIP محليًا ضمن الحد"));return;}
 packageSourcePath_=info.canonicalFilePath();busy_=true;emit busyChanged();
 setStatus(QStringLiteral("قراءة نسخة ZIP إلى الذاكرة دون استخراج / Reading bounded ZIP snapshot without extraction"));
 watcher_.setFuture(QtConcurrent::run([](QPromise<FirmwareInspection> &promise,const QString &source){
  constexpr qint64 maxBytes=128*1024*1024;
  promise.setProgressRange(0,100);QFile file(source);
  auto fail=[&promise](const QString &error){promise.addResult(FirmwareInspection{false,QStringLiteral("ZIP rejected: ")+error,{},{}});};
  if(!mr::recovery::openRegularInput(file)){fail(QStringLiteral("file-open-failed"));return;}
  const auto size=file.size();const auto modified=file.fileTime(QFileDevice::FileModificationTime);
  if(size<22||size>maxBytes){fail(QStringLiteral("snapshot-size-limit"));return;}
  QByteArray bytes;bytes.reserve(static_cast<qsizetype>(size));
  while(!file.atEnd()){
   if(promise.isCanceled())return;
   const auto chunk=file.read(1024*1024);
   if(chunk.isEmpty()||file.error()!=QFileDevice::NoError||bytes.size()+chunk.size()>maxBytes){fail(QStringLiteral("read-or-size-limit"));return;}
   bytes.append(chunk);promise.setProgressValue(static_cast<int>(qMin<qint64>(90,bytes.size()*90/size)));
  }
  if(bytes.size()!=size||file.size()!=size||file.fileTime(QFileDevice::FileModificationTime)!=modified){fail(QStringLiteral("source-changed"));return;}
  if(promise.isCanceled())return;
  QBuffer snapshot(&bytes);snapshot.open(QIODevice::ReadOnly);QList<mr::service::PackageEntry> entries;
  if(!mr::service::validPackageStructure(snapshot,false,&entries)){fail(QStringLiteral("unsupported-or-malformed-zip (ZIP64, encrypted and multi-disk unsupported)"));return;}
  QJsonArray listing,candidates;quint64 compressed=0,uncompressed=0;
  const QSet<QString> imageNames={"boot.img","init_boot.img","vendor_boot.img","vbmeta.img","super.img","system.img","vendor.img","product.img","odm.img","dtbo.img","recovery.img"};
  for(const auto &entry:entries){
   if(promise.isCanceled())return;
   QStringDecoder decoder(QStringDecoder::Utf8);const QString name=decoder(entry.name);
   bool ascii=true,control=false;for(const auto c:entry.name)if(static_cast<unsigned char>(c)>=128)ascii=false;
   for(const auto c:name)if(c.category()==QChar::Other_Control||c.category()==QChar::Other_Format||c.category()==QChar::Separator_Line||c.category()==QChar::Separator_Paragraph)control=true;
   if(decoder.hasError()||control||(!ascii&&!(entry.flags&0x800))){fail(QStringLiteral("unsupported-entry-name-encoding"));return;}
   compressed+=entry.compressed;uncompressed+=entry.uncompressed;
   if(listing.size()<256)listing.append(QJsonObject{{"name",name},{"compressedBytes",QString::number(entry.compressed)},{"declaredBytes",QString::number(entry.uncompressed)},{"method",entry.method==0?"stored":"deflate"}});
   if(imageNames.contains(name.section('/',-1))&&candidates.size()<256)candidates.append(name);
  }
  if(promise.isCanceled())return;
  QJsonObject report{{"schemaVersion",1},{"status","structure-only"},{"format","zip"},{"snapshotBytes",QString::number(size)},
   {"snapshotSha256",QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())},
   {"entryCount",entries.size()},{"listedEntriesTruncated",entries.size()>256},{"entries",listing},{"imageNameCandidates",candidates},
   {"compressedBytes",QString::number(compressed)},{"declaredUncompressedBytes",QString::number(uncompressed)},
   {"oemSignatureVerified",false},{"payloadCrcVerified",false},{"partitionLayoutVerified",false},{"deviceCompatibilityVerified",false},{"extracted",false},
   {"notice","ZIP declarations and memory-snapshot SHA-256 only. Image names are candidates, not a partition map. No decompression, payload/AVB verification, OEM authenticity, device compatibility or flashing approval. ZIP64/encrypted/multi-disk and files over 128 MiB unsupported."}};
  promise.setProgressValue(100);promise.addResult(FirmwareInspection{false,QStringLiteral("ZIP metadata parsed; all device/authenticity gates remain closed / قُرئت البنية فقط"),{},report});
 },path));
}
bool FirmwareBridge::exportPackageReport(const QString &input,bool approved){
 return exportNewReport(packageReport_, input, packageSourcePath_, approved, busy_);
}
