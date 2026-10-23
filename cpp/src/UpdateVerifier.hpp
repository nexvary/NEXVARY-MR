#pragma once
#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <atomic>
#include <memory>

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

// Internal, read-only primitive. It does not download, stage, install, execute,
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
private:
  TrustedKeys keys_;
  InstalledState installed_;
};
}
