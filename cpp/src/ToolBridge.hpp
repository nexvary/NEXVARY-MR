#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

// Only fixed, read-only device commands are exposed to QML. No shell command API.
class ToolBridge final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(QVariantList tools READ tools NOTIFY toolsChanged)
  Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
  Q_PROPERTY(QString selectedSerial READ selectedSerial WRITE selectDevice NOTIFY selectedSerialChanged)
  Q_PROPERTY(QVariantMap deviceInfo READ deviceInfo NOTIFY deviceInfoChanged)
  Q_PROPERTY(QString logcat READ logcat NOTIFY logcatChanged)
  Q_PROPERTY(QString driverInfo READ driverInfo NOTIFY driverInfoChanged)
  Q_PROPERTY(bool autoRefresh READ autoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)
public:
  explicit ToolBridge(QObject *parent = nullptr);
  ~ToolBridge() override;
  QString status() const { return status_; }
  bool busy() const { return active_; }
  QVariantList tools() const;
  QVariantList devices() const { return devices_; }
  QString selectedSerial() const { return selectedSerial_; }
  QVariantMap deviceInfo() const { return deviceInfo_; }
  QString logcat() const { return logcat_; }
  QString driverInfo() const { return driverInfo_; }
  bool autoRefresh() const { return pollTimer_.isActive(); }
  Q_INVOKABLE void refreshTools();
  Q_INVOKABLE void scanAdb();
  Q_INVOKABLE void scanFastboot();
  Q_INVOKABLE void startMirror();
  Q_INVOKABLE void selectDevice(const QString &serial);
  Q_INVOKABLE void readDeviceInfo();
  Q_INVOKABLE void readLogcat();
  Q_INVOKABLE void inspectDrivers();
  Q_INVOKABLE void cancel();
  Q_INVOKABLE void setAutoRefresh(bool enabled);
  static bool isValidSerial(const QString &serial);
  static QVariantList parseAdbDevices(const QString &output);
  static QVariantList parseFastbootDevices(const QString &output);
  static QVariantMap parseDeviceProperties(const QString &output);
signals:
  void statusChanged();
  void busyChanged();
  void toolsChanged();
  void devicesChanged();
  void selectedSerialChanged();
  void deviceInfoChanged();
  void logcatChanged();
  void driverInfoChanged();
  void autoRefreshChanged();
  void operationFinished(bool success);
private:
  enum class Operation { None, AdbScan, FastbootScan, Properties, Logcat, Drivers };
  bool runReadOnly(const QString &program, const QStringList &arguments, Operation operation);
  bool selectedAdbReady();
  void collectOutput();
  void finish(int exitCode, QProcess::ExitStatus exitStatus);
  void replaceDevices(const QVariantList &found, const QString &transport);
  void setStatus(const QString &value);
  QProcess process_;
  QTimer timeoutTimer_;
  QTimer pollTimer_;
  bool active_ = false;
  Operation operation_ = Operation::None;
  QByteArray output_;
  QByteArray errors_;
  QString terminalReason_;
  QString commandSerial_;
  QString status_;
  QVariantList devices_;
  QString selectedSerial_;
  QVariantMap deviceInfo_;
  QString logcat_;
  QString driverInfo_;
};
