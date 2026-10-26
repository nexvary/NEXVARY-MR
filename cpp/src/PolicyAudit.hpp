#pragma once
#include <QVariantList>
#include <QVariantMap>
class IdentityService;

// Bounded, atomic protected snapshot. Ed25519 signs exact event bytes; every
// event binds its sequence and preceding digest. External retained anchors are
// required to detect whole-file rollback/reset across application restarts.
class PolicyAudit final {
public:
  explicit PolicyAudit(const QString &path, bool synthetic);
  ~PolicyAudit();
  PolicyAudit(const PolicyAudit &) = delete;
  PolicyAudit &operator=(const PolicyAudit &) = delete;
  bool healthy() const { return healthy_; }
  QString error() const { return error_; }
  bool check();
  bool append(const QString &action, const QVariantMap &record,
              const QString &result, IdentityService *actor = nullptr);
  QVariantList events() const { return events_; }
  QVariantMap anchor() const;
  QByteArray exportBytes() const;
  static QVariantMap parseAnchor(const QByteArray &bytes);
  static bool verifyExport(const QByteArray &bytes, const QString &expectedKey,
                           const QVariantMap &retainedAnchor, QString &error);

private:
  bool fail(const QString &message);
  bool save(const QVariantMap &state);
  QString path_, error_, journalId_, publicKey_;
  bool synthetic_ = false, healthy_ = false;
  QByteArray secret_, fileDigest_;
  QVariantList events_;
};
