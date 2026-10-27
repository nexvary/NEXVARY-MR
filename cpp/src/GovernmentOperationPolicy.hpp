#pragma once
#include <QElapsedTimer>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <memory>
#include <functional>
class IdentityService;
class PolicyAudit;

// Application operation gate, not an OS network/process sandbox. Grants are
// deliberately memory-only and independent of the currently displayed
// workspace.
class GovernmentOperationPolicy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool offline READ offline NOTIFY changed)
  Q_PROPERTY(QVariantList permissions READ permissions NOTIFY changed)
  Q_PROPERTY(QString error READ error NOTIFY errorChanged)
  Q_PROPERTY(QVariantList requests READ requests NOTIFY changed)
  Q_PROPERTY(QVariantMap auditStatus READ auditStatus NOTIFY auditChanged)
  Q_PROPERTY(
      QVariantList operationReceipts READ operationReceipts NOTIFY auditChanged)
public:
  explicit GovernmentOperationPolicy(IdentityService *identity,
                                     QObject *parent = nullptr,
                                     int lifetimeMs = 600000);
  ~GovernmentOperationPolicy() override;
  bool offline() const;
  QVariantList permissions() const;
  QString error() const { return error_; }
  // Native consumers must use permit(), not a QML visibility/enabled flag.
  bool permit(const QString &capability);
  bool granted(const QString &capability) const;
  Q_INVOKABLE bool grant(const QString &capability, const QString &reason,
                         bool approved);
  Q_INVOKABLE void revoke(const QString &capability);
  Q_INVOKABLE void revokeAll();
  Q_INVOKABLE bool openExternal(const QString &url);
  static QStringList capabilities();
  QVariantList requests() const;
  QVariantMap auditStatus() const;
  Q_INVOKABLE QString requestException(const QString &capability,
                                       const QString &reason,
                                       const QString &caseRef, int seconds,
                                       bool consent);
  Q_INVOKABLE bool reviewException(const QString &id, const QString &reason);
  Q_INVOKABLE bool decideException(const QString &id, bool approve,
                                   const QString &reason);
  Q_INVOKABLE bool activateException(const QString &id);
  Q_INVOKABLE bool revokeException(const QString &id);
  Q_INVOKABLE bool exportAudit(const QString &path, bool consent);
  Q_INVOKABLE bool verifyAudit(const QString &path,
                               const QString &retainedAnchorJson);
  Q_INVOKABLE void refreshAudit();
  // Native consumers only. No caller-controlled labels, paths or case content.
  QString beginOperation(const QString &capability, const QString &kind);
  QVariantMap finishOperation(const QString &id, const QString &status,
                              const QString &sourceDigest = {},
                              const QString &outputDigest = {});
  QVariantList operationReceipts() const;
  // Native case adapter only; local RBAC/grants and evidence checks remain in LawCaseBridge/Store.
  bool recordCaseOperation(const QVariantMap &record);
  void setSyntheticAuditWriteCheckpoint(std::function<void()> callback);
  QVariantList caseOperationStates();
  bool caseOperationHistoryHealthy() const { return caseHistoryHealthy_; }
signals:
  void changed();
  void errorChanged();
  void auditChanged();

private:
  void refresh();
  bool fail(const QString &error);
  bool audit(const QString &action, const QVariantMap &record,
             const QString &result, bool human);
  bool available();
  bool approvalCurrent(const QString &id);
  bool transition(const QString &id, const QString &action,
                  const QString &status, const QString &reason = QString());
  void closeGrant(const QString &capability, const QString &action);
  IdentityService *identity_;
  struct Grant {
    QString reason, actor, expiresUtc, id;
    qint64 deadline = 0;
  };
  QMap<QString, Grant> grants_;
  QElapsedTimer clock_;
  QTimer monitor_;
  int lifetimeMs_;
  bool refreshing_ = false;
  bool caseHistoryHealthy_ = false;
  QString error_;
  std::unique_ptr<PolicyAudit> audit_;
  QMap<QString, QVariantMap> requests_;
  QVariantMap verification_;
  QMap<QString, QVariantMap> operations_;
  QMap<QString, QString> operationClosures_;
};
