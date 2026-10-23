#pragma once
#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <atomic>
#include <memory>
class QFile;
class QTemporaryDir;

namespace mr::update {
inline constexpr qsizetype maximumManifestBytes = 16 * 1024;
inline constexpr qint64 maximumPackageBytes = 512 * 1024 * 1024;
// Public keys must come from the caller's independently approved trust policy,
// never the manifest/package. No production or fixture key is embedded here.
using TrustedKeys = QMap<QString, QByteArray>;
struct InstalledState {
  QString version;
  quint64 sequenceFloor = 0;
};
struct CheckResult {
  QString error;
  QJsonObject report;
  bool accepted() const { return error.isEmpty() && !report.isEmpty(); }
};

class StagedPackage;
struct SnapshotResult;
// Internal verification primitive. It does not download, install, execute,
// persist anti-rollback state or authorize an installer from a verified path.
// Persistent trusted state, key provisioning and atomic installation are still
// required before integrating an updater into the product UI.
class Verifier final {
public:
  Verifier(TrustedKeys keys, InstalledState installed);
  static QByteArray signingMessage(const QByteArray &compactManifest);
  static QString compiledProfile();
  CheckResult checkManifest(const QByteArray &manifest, const QByteArray &signature,
                            const QDateTime &trustedNow) const;
  CheckResult checkPackage(const QByteArray &manifest, const QByteArray &signature,
                           const QString &localPath, const QDateTime &trustedNow,
                           const std::shared_ptr<std::atomic_bool> &cancelled = {}) const;
  // The caller supplies an existing, approved local staging directory. The
  // result owns a new temporary child and an open file; no pathname is exposed.
  SnapshotResult stagePackage(const QByteArray &manifest, const QByteArray &signature,
                              const QString &localPath, const QString &stagingDirectory,
                              const QDateTime &trustedNow,
                              const std::shared_ptr<std::atomic_bool> &cancelled = {}) const;
private:
  TrustedKeys keys_;
  InstalledState installed_;
};
class StagedPackage final {
public:
  ~StagedPackage();
  StagedPackage(const StagedPackage &) = delete;
  StagedPackage &operator=(const StagedPackage &) = delete;
  // Reauthenticate with CURRENT caller policy and rehash the retained handle,
  // never reopening the mutable source. Call from one worker at a time.
  CheckResult recheck(const Verifier &currentPolicy, const QDateTime &trustedNow,
                      const std::shared_ptr<std::atomic_bool> &cancelled = {});
private:
  friend class Verifier;
  StagedPackage(QByteArray manifest, QByteArray signature,
                std::unique_ptr<QTemporaryDir> directory, std::unique_ptr<QFile> file);
  QByteArray manifest_, signature_;
  std::unique_ptr<QTemporaryDir> directory_;
  std::unique_ptr<QFile> file_; // Closed before deleting the owned directory.
};
struct SnapshotResult {
  CheckResult check;
  std::unique_ptr<StagedPackage> snapshot;
  bool accepted() const { return check.accepted() && snapshot != nullptr; }
};
}
