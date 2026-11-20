#pragma once
#include <QByteArray>
#include <QFutureWatcher>
#include <QObject>
#include <QString>

struct FirmwareManifestValidation {
  bool valid = false;
  QString error;
  QString expectedSha256;
  QString build;
  QString sourceUrl;
};
struct FirmwareInspection {
  bool matched = false;
  QString message;
};

// Matching metadata and SHA256 is an integrity check, never proof of authenticity.
// This bridge cannot download, unlock, flash, or modify a connected device.
class FirmwareBridge final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
  Q_PROPERTY(bool compatible READ compatible NOTIFY compatibleChanged)
public:
  explicit FirmwareBridge(QObject *parent = nullptr);
  ~FirmwareBridge() override;
  QString status() const { return status_; }
  bool busy() const { return busy_; }
  int progress() const { return progress_; }
  bool compatible() const { return compatible_; }
  Q_INVOKABLE void inspect(const QString &manifestPath, const QString &firmwarePath,
                           const QString &actualBrand, const QString &actualModel);
  Q_INVOKABLE void cancel();
  Q_INVOKABLE void invalidate();
  static FirmwareManifestValidation validateManifest(const QByteArray &json,
      const QString &actualBrand, const QString &actualModel);
signals:
  void statusChanged();
  void busyChanged();
  void progressChanged();
  void compatibleChanged();
  void inspectionFinished(bool matched);
private:
  void setStatus(const QString &text);
  void reject(const QString &message);
  QFutureWatcher<FirmwareInspection> watcher_;
  QString status_;
  int progress_ = 0;
  bool busy_ = false;
  bool compatible_ = false;
  bool invalidated_ = false;
};
