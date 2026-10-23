#pragma once
#include <QByteArray>
#include <QFutureWatcher>
#include <QObject>
#include <QJsonObject>
#include <QVariantMap>
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
  QJsonObject avbReport;
  QJsonObject packageReport{};
};

// Matching metadata and SHA256 is an integrity check, never proof of authenticity.
// This bridge cannot download, unlock, flash, or modify a connected device.
class FirmwareBridge final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap packageReport READ packageReport NOTIFY packageReportChanged)
  Q_PROPERTY(QString packageJson READ packageJson NOTIFY packageReportChanged)
  Q_PROPERTY(QVariantMap avbReport READ avbReport NOTIFY avbReportChanged)
  Q_PROPERTY(QString avbJson READ avbJson NOTIFY avbReportChanged)
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
  QVariantMap packageReport() const { return packageReport_.toVariantMap(); }
  QString packageJson() const;
  Q_INVOKABLE void inspectPackage(const QString &path);
  Q_INVOKABLE bool exportPackageReport(const QString &path, bool approved);
  Q_INVOKABLE void clearPackageReport();
  QVariantMap avbReport() const { return avbReport_.toVariantMap(); }
  QString avbJson() const;
  Q_INVOKABLE void inspectAvb(const QString &path);
  Q_INVOKABLE bool exportAvbReport(const QString &path);
  Q_INVOKABLE void clearAvbReport();
  Q_INVOKABLE void cancel();
  Q_INVOKABLE void invalidate();
  static FirmwareManifestValidation validateManifest(const QByteArray &json,
      const QString &actualBrand, const QString &actualModel);
signals:
  void avbReportChanged();
  void packageReportChanged();
  void statusChanged();
  void busyChanged();
  void progressChanged();
  void compatibleChanged();
  void inspectionFinished(bool matched);
private:
  void setStatus(const QString &text);
  void reject(const QString &message);
  QFutureWatcher<FirmwareInspection> watcher_;
  QJsonObject avbReport_;
  QJsonObject packageReport_;
  QString packageSourcePath_;
  QString avbSourcePath_;
  QString status_;
  int progress_ = 0;
  bool busy_ = false;
  bool compatible_ = false;
  bool invalidated_ = false;
};

