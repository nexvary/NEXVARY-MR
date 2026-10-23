#include "UpdateVerifier.hpp"
#include "RecoveryFile.hpp"
#include "ServiceOperationPolicy.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSet>
#include <QDir>
#include <QTemporaryDir>
#include <array>
#include <utility>
#include <sodium.h>

namespace mr::update {
namespace {
CheckResult fail(const char *code) { return {QString::fromLatin1(code), {}}; }
bool decimal(const QString &value, quint64 &number) {
  static const QRegularExpression pattern(QStringLiteral("\\A(?:0|[1-9][0-9]{0,19})\\z"));
  if (!pattern.match(value).hasMatch()) return false;
  bool ok = false; number = value.toULongLong(&ok); return ok;
}
bool version(const QString &value, std::array<quint64,3> &parts) {
  const auto components = value.split(QLatin1Char('.'));
  if (components.size() != 3) return false;
  for (qsizetype i = 0; i < 3; ++i) {
    auto &part = parts[static_cast<std::size_t>(i)];
    if (!decimal(components[i], part) || part > 65535) return false;
  }
  return true;
}
bool keyId(const QString &value) {
  static const QRegularExpression pattern(QStringLiteral("\\A[a-z0-9][a-z0-9._-]{0,63}\\z"));
  return pattern.match(value).hasMatch();
}
bool utcTime(const QString &value, QDateTime &time) {
  time = QDateTime::fromString(value, Qt::ISODate);
  return time.isValid() && value.endsWith(QLatin1Char('Z')) &&
         value == time.toUTC().toString(Qt::ISODate);
}
CheckResult verifyOpened(QFile &file, CheckResult result,
                         const std::shared_ptr<std::atomic_bool> &cancelled,
                         QFile *copy = nullptr) {
  const auto stopped = [&] { return cancelled && cancelled->load(); };
  if (!file.isOpen() || !file.seek(0)) return fail("package-read");
  const auto expectedSize = result.report.value("packageBytes").toString().toLongLong();
  if (file.size() != expectedSize) return fail("package-size-mismatch");
  const auto modified = file.fileTime(QFileDevice::FileModificationTime);
  QCryptographicHash hash(QCryptographicHash::Sha256);
  qint64 read = 0;
  while (!file.atEnd()) {
    if (stopped()) return fail("cancelled");
    const auto chunk = file.read(1024 * 1024);
    if (chunk.isEmpty() || file.error() != QFileDevice::NoError) return fail("package-read");
    read += chunk.size();
    if (read > expectedSize) return fail("package-changed");
    if (copy && copy->write(chunk) != chunk.size()) return fail("staging-write");
    hash.addData(chunk);
  }
  if (stopped()) return fail("cancelled");
  if (read != expectedSize || file.size() != expectedSize || file.fileTime(QFileDevice::FileModificationTime) != modified)
    return fail("package-changed");
  if (QString::fromLatin1(hash.result().toHex()) != result.report.value("sha256").toString()) return fail("package-hash-mismatch");
  if (copy && !copy->flush()) return fail("staging-write");
  result.report.insert("status", "package-hash-verified");
  result.report.insert("packageHashVerified", true);
  result.report.insert("immutableSnapshotCreated", false);
  result.report.insert("authenticodeVerified", false);
  result.report.insert("installed", false);
  return result;
}

}
Verifier::Verifier(TrustedKeys keys, InstalledState installed)
  : keys_(std::move(keys)), installed_(std::move(installed)) {}
QByteArray Verifier::signingMessage(const QByteArray &manifest) {
  return QByteArrayLiteral("NEXVARY-MR update manifest v1\n") + manifest;
}
QString Verifier::compiledProfile() {
  return mr::service::laboratoryBuild() ? QStringLiteral("laboratory-experimental")
                                      : QStringLiteral("production-read-only");
}
CheckResult Verifier::checkManifest(const QByteArray &manifest, const QByteArray &signature,
                                   const QDateTime &trustedNow) const {
  if (manifest.isEmpty() || manifest.size() > maximumManifestBytes) return fail("manifest-size-limit");
  if (signature.size() != crypto_sign_BYTES) return fail("signature-size");
  if (keys_.isEmpty()) return fail("no-approved-trust-anchors");
  if (keys_.size() > 16) return fail("invalid-trust-store");
  for (auto key = keys_.cbegin(); key != keys_.cend(); ++key)
    if (!keyId(key.key()) || key.value().size() != crypto_sign_PUBLICKEYBYTES) return fail("invalid-trust-store");
  QJsonParseError parseError;
  const auto document = QJsonDocument::fromJson(manifest, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) return fail("manifest-json");
  // The protocol accepts exactly Qt's compact ASCII JSON profile. This also
  // rejects duplicate keys, BOMs, alternate escapes and ambiguous whitespace.
  if (document.toJson(QJsonDocument::Compact) != manifest) return fail("noncanonical-manifest");
  const auto object = document.object();
  const auto id = object.value(QStringLiteral("keyId")).toString();
  if (!keyId(id) || !keys_.contains(id)) return fail("unapproved-signing-key");
  static const int initialized = sodium_init();
  if (initialized < 0) return fail("crypto-initialization");
  if (QByteArray(SODIUM_VERSION_STRING) != "1.0.22" || QByteArray(sodium_version_string()) != "1.0.22")
    return fail("crypto-version-unapproved");
  const auto message = signingMessage(manifest);
  const auto publicKey = keys_.value(id);
  if (crypto_sign_verify_detached(reinterpret_cast<const unsigned char *>(signature.constData()),
       reinterpret_cast<const unsigned char *>(message.constData()),
       static_cast<unsigned long long>(message.size()),
       reinterpret_cast<const unsigned char *>(publicKey.constData())) != 0) return fail("invalid-signature");
  const QSet<QString> fields = {"schema", "product", "platform", "profile", "channel", "keyId", "version",
                              "sequence", "packageKind", "packageBytes", "sha256", "issuedUtc", "expiresUtc"};
  if (object.size() != fields.size()) return fail("manifest-fields");
  for (auto field = object.constBegin(); field != object.constEnd(); ++field)
    if (!fields.contains(field.key()) || !field.value().isString() || field.value().toString().size() > 128)
      return fail("manifest-fields");
  if (object.value("schema") != QStringLiteral("nexvary-update/v1") ||
      object.value("product") != QStringLiteral("NEXVARY-MR") ||
      object.value("platform") != QStringLiteral("windows-x64") ||
      object.value("channel") != QStringLiteral("stable")) return fail("unsupported-target");
  if (object.value("profile").toString() != compiledProfile()) return fail("build-profile-mismatch");
  const auto kind = object.value("packageKind").toString();
  if (kind != QStringLiteral("portable-zip") && kind != QStringLiteral("installer-exe")) return fail("package-kind");
  std::array<quint64,3> current{}, candidate{};
  if (!version(installed_.version, current)) return fail("invalid-installed-state");
  if (!version(object.value("version").toString(), candidate)) return fail("invalid-version");
  if (candidate <= current) return fail("version-not-newer");
  quint64 sequence = 0, packageSize = 0;
  if (!decimal(object.value("sequence").toString(), sequence) || sequence <= installed_.sequenceFloor)
    return fail("sequence-not-newer");
  if (!decimal(object.value("packageBytes").toString(), packageSize) || packageSize == 0 ||
      packageSize > static_cast<quint64>(maximumPackageBytes)) return fail("package-size-limit");
  static const QRegularExpression hashPattern(QStringLiteral("\\A[0-9a-f]{64}\\z"));
  if (!hashPattern.match(object.value("sha256").toString()).hasMatch()) return fail("invalid-package-hash");
  QDateTime issued, expires;
  if (!trustedNow.isValid() || !utcTime(object.value("issuedUtc").toString(), issued) ||
      !utcTime(object.value("expiresUtc").toString(), expires) || issued > trustedNow || expires <= trustedNow ||
      expires <= issued || issued.secsTo(expires) > 31LL * 24 * 60 * 60) return fail("manifest-time-window");
  QJsonObject report = object;
  report.insert("status", "manifest-verified");
  report.insert("signatureVerified", true);
  report.insert("signingKeySha256", QString::fromLatin1(QCryptographicHash::hash(publicKey,QCryptographicHash::Sha256).toHex()));
  report.insert("manifestSha256", QString::fromLatin1(QCryptographicHash::hash(manifest,QCryptographicHash::Sha256).toHex()));
  report.insert("packageHashVerified", false);
  report.insert("installationAuthorized", false);
  report.insert("rollbackStatePersisted", false);
  report.insert("publisherIdentityCertified", false);
  return {{}, report};
}
CheckResult Verifier::checkPackage(const QByteArray &manifest, const QByteArray &signature,
                                  const QString &input, const QDateTime &trustedNow,
                                  const std::shared_ptr<std::atomic_bool> &cancelled) const {
  const auto stopped = [&] { return cancelled && cancelled->load(); };
  if (stopped()) return fail("cancelled");
  auto result = checkManifest(manifest, signature, trustedNow);
  if (!result.accepted()) return result; // No file open before authentication/policy.
  if (stopped()) return fail("cancelled");
  const auto path = mr::recovery::localFilePath(input);
  const QFileInfo info(path);
  if (path.isEmpty() || !info.isFile() || info.isSymLink()) return fail("local-regular-package-required");
  QFile file(path);
  if (!mr::recovery::openRegularInput(file)) return fail("package-open");
  return verifyOpened(file, std::move(result), cancelled);
}

SnapshotResult Verifier::stagePackage(const QByteArray &manifest, const QByteArray &signature,
                                      const QString &input, const QString &stagingDirectory,
                                      const QDateTime &trustedNow,
                                      const std::shared_ptr<std::atomic_bool> &cancelled) const {
  const auto reject = [](const char *code) -> SnapshotResult { return {fail(code), nullptr}; };
  if (cancelled && cancelled->load()) return reject("cancelled");
  auto result = checkManifest(manifest, signature, trustedNow);
  if (!result.accepted()) return {std::move(result), nullptr};
  const auto path = mr::recovery::localFilePath(input);
  const QFileInfo sourceInfo(path);
  if (path.isEmpty() || !sourceInfo.isFile() || sourceInfo.isSymLink()) return reject("local-regular-package-required");
  QFile source(path);
  if (!mr::recovery::openRegularInput(source)) return reject("package-open");
  if (source.size() != result.report.value("packageBytes").toString().toLongLong()) return reject("package-size-mismatch");
  const auto root = mr::recovery::localFilePath(stagingDirectory);
  const QFileInfo rootInfo(root);
  if (root.isEmpty() || !rootInfo.isDir() || rootInfo.isSymLink()) return reject("local-staging-directory-required");
  const auto canonicalRoot = rootInfo.canonicalFilePath();
  if (canonicalRoot.isEmpty()) return reject("local-staging-directory-required");
  if (cancelled && cancelled->load()) return reject("cancelled");
  auto directory = std::make_unique<QTemporaryDir>(QDir(canonicalRoot).filePath("nexvary-update-XXXXXX"));
  if (!directory->isValid()) return reject("staging-create");
  auto copy = std::make_unique<QFile>(directory->filePath("package.bin"));
  if (!copy->open(QIODevice::ReadWrite | QIODevice::NewOnly)) return reject("staging-create");
#ifndef Q_OS_WIN
  if (!copy->setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) return reject("staging-permissions");
#endif
  result = verifyOpened(source, std::move(result), cancelled, copy.get());
  if (!result.accepted()) return {std::move(result), nullptr};
  auto snapshot = std::unique_ptr<StagedPackage>(new StagedPackage(manifest, signature, std::move(directory), std::move(copy)));
  result = snapshot->recheck(*this, trustedNow, cancelled); // Verify the bytes actually written.
  if (!result.accepted()) return {std::move(result), nullptr};
  return {std::move(result), std::move(snapshot)};
}
StagedPackage::StagedPackage(QByteArray manifest, QByteArray signature,
                             std::unique_ptr<QTemporaryDir> directory, std::unique_ptr<QFile> file)
  : manifest_(std::move(manifest)), signature_(std::move(signature)),
    directory_(std::move(directory)), file_(std::move(file)) {}
StagedPackage::~StagedPackage() = default;
CheckResult StagedPackage::recheck(const Verifier &currentPolicy, const QDateTime &trustedNow,
                                  const std::shared_ptr<std::atomic_bool> &cancelled) {
  if (cancelled && cancelled->load()) return fail("cancelled");
  auto result = currentPolicy.checkManifest(manifest_, signature_, trustedNow);
  if (!result.accepted()) return result;
  result = verifyOpened(*file_, std::move(result), cancelled);
  if (!result.accepted()) return result;
  result.report.insert("stagedCopyCreated", true);
  result.report.insert("retainedHandleVerified", true);
  return result;
}
}
