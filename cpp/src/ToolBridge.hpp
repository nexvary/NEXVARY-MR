#pragma once

#include <QObject>
#include "MirrorSession.hpp"
#include <QProcess>
#include <QTimer>
#include <QMap>
#include <QVariantList>
#include <QVariantMap>
#include <QQueue>
#include <QFutureWatcher>
#include <QTemporaryDir>
#include <atomic>
#include <memory>

// One foreground command runner plus a bounded background discovery reader.
// No arbitrary shell API; foreground actions supersede background discovery.
class ToolBridge final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool mirrorActive READ mirrorActive NOTIFY mirrorChanged)
  Q_PROPERTY(QString mirrorState READ mirrorState NOTIFY mirrorChanged)
  Q_PROPERTY(QString mirrorStatus READ mirrorStatus NOTIFY mirrorChanged)
  Q_PROPERTY(QString mirrorOutput READ mirrorOutput NOTIFY mirrorChanged)
  Q_PROPERTY(bool screenViewAvailable READ screenViewAvailable CONSTANT)
  Q_PROPERTY(bool foregroundBusy READ foregroundBusy NOTIFY busyChanged)
  Q_PROPERTY(QString activeOperation READ activeOperation NOTIFY busyChanged)
  Q_PROPERTY(bool backgroundRefreshing READ backgroundRefreshing NOTIFY backgroundRefreshChanged)
  Q_PROPERTY(bool laboratoryBuild READ laboratoryBuild CONSTANT)
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(QVariantList forensicTools READ forensicTools NOTIFY toolsChanged)
  Q_PROPERTY(QVariantList tools READ tools NOTIFY toolsChanged)
  Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
  Q_PROPERTY(QString selectedSerial READ selectedSerial WRITE selectDevice NOTIFY selectedSerialChanged)
  Q_PROPERTY(QVariantMap deviceInfo READ deviceInfo NOTIFY deviceInfoChanged)
  Q_PROPERTY(QString logcat READ logcat NOTIFY logcatChanged)
  Q_PROPERTY(QString driverInfo READ driverInfo NOTIFY driverInfoChanged)
  Q_PROPERTY(QVariantMap driverBinding READ driverBinding NOTIFY driverInfoChanged)
  Q_PROPERTY(QVariantMap connectionDiagnosis READ connectionDiagnosis NOTIFY connectionDiagnosisChanged)
  Q_PROPERTY(QVariantMap deviceIntelligence READ deviceIntelligence NOTIFY deviceIntelligenceChanged)
  Q_PROPERTY(bool autoRefresh READ autoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)
  Q_PROPERTY(QVariantList serviceActions READ serviceActions NOTIFY serviceChanged)
  Q_PROPERTY(QVariantMap serviceProfile READ serviceProfile NOTIFY serviceChanged)
  Q_PROPERTY(QString serviceOutput READ serviceOutput NOTIFY serviceChanged)
  Q_PROPERTY(QVariantList serviceHistory READ serviceHistory NOTIFY serviceChanged)
  Q_PROPERTY(QString servicePhase READ servicePhase NOTIFY serviceChanged)
  Q_PROPERTY(QVariantList requestedModels READ requestedModels CONSTANT)
public:
  explicit ToolBridge(QObject *parent = nullptr);
  ~ToolBridge() override;
  bool laboratoryBuild() const;
  bool screenViewAvailable()const;
  bool foregroundBusy()const;
  QString activeOperation()const;
  bool backgroundRefreshing()const{return polling_;}
  QString status() const { return status_; }
  bool busy() const { return active_ || serviceRunning_ || mirror_.active(); }
  QVariantList tools() const;
  QVariantList forensicTools() const;
  QVariantList devices() const { return devices_; }
  QString selectedSerial() const { return selectedSerial_; }
  QVariantMap deviceInfo() const { return deviceInfo_; }
  QString logcat() const { return logcat_; }
  QString driverInfo() const { return driverInfo_; }
  QVariantMap driverBinding() const { return driverBinding_; }
  QVariantMap connectionDiagnosis() const;
  QVariantMap deviceIntelligence() const;
  bool autoRefresh() const { return pollTimer_.isActive(); }
  Q_INVOKABLE void refreshTools();
  Q_INVOKABLE bool configureTool(const QString &name, const QString &localPath);
  Q_INVOKABLE bool clearTool(const QString &name);
  Q_INVOKABLE bool loadToolConfiguration(const QString &configPath);
  Q_INVOKABLE void scanAdb();
  Q_INVOKABLE void scanFastboot();
  bool mirrorActive()const{return mirror_.active();}
  QString mirrorState()const{return mirror_.state();}
  QString mirrorStatus()const{return mirror_.status();}
  QString mirrorOutput()const{return mirror_.output();}
  Q_INVOKABLE bool startMirror(const QString &expectedSerial=QString(),const QVariantMap &approval=QVariantMap());
  Q_INVOKABLE void stopMirror();
  Q_INVOKABLE void selectDevice(const QString &serial);
  Q_INVOKABLE void readDeviceInfo();
  Q_INVOKABLE void readLogcat();
  Q_INVOKABLE void inspectDrivers();
  Q_INVOKABLE void inspectSelectedDriver();
  Q_INVOKABLE void inspectUsbInventory();
  Q_INVOKABLE void cancel();
  Q_INVOKABLE void setAutoRefresh(bool enabled);
  static bool isValidSerial(const QString &serial);
  static QVariantList parseAdbDevices(const QString &output);
  static QVariantList parseFastbootDevices(const QString &output);
  static QVariantMap parseDeviceProperties(const QString &output);
  QVariantList serviceActions() const;
  QVariantMap serviceProfile() const { return serviceProfile_; }
  QString serviceOutput() const { return serviceOutput_; }
  QString servicePhase() const { return servicePhase_; }
  QVariantList serviceHistory() const { return serviceHistory_; }
  static QVariantList requestedModels();
  Q_INVOKABLE bool startService(const QString &action, const QString &expectedSerial, const QVariantMap &approval);
  Q_INVOKABLE bool exportServiceReport(const QString &localPath, bool plaintextApproved);
signals:
  void backgroundRefreshChanged();
  void backgroundRefreshFinished(bool success);
  void mirrorChanged();
  void statusChanged();
  void busyChanged();
  void toolsChanged();
  void devicesChanged();
  void selectedSerialChanged();
  void deviceInfoChanged();
  void logcatChanged();
  void driverInfoChanged();
  void connectionDiagnosisChanged();
  void deviceIntelligenceChanged();
  void autoRefreshChanged();
  void serviceChanged();
  void operationFinished(bool success);
  void diagnosticCompleted(const QString &operation, bool success);
private:
  enum class Operation { None, AdbScan, FastbootScan, Properties, Logcat, Drivers, Service, DriverBinding, UsbInventory };
  void recordConnectionObservation(Operation operation,const QString &state);
  void runUsbInventory(Operation operation);
  bool runReadOnly(const QString &program, const QStringList &arguments, Operation operation);
  bool selectedAdbReady();
  QString resolveExecutable(const QString &name) const;
  bool persistToolConfiguration(const QMap<QString, QString> &paths);
  QMap<QString, QString> configuredTools_;
  QString toolConfigurationPath_;
  void collectOutput();
  void finish(int exitCode, QProcess::ExitStatus exitStatus);
  void replaceDevices(const QVariantList &found, const QString &transport,bool preserveUnchanged=false);
  void setStatus(const QString &value);
  MirrorSession mirror_;
  void pollDiscovery();
  void finishPoll(int exitCode,QProcess::ExitStatus exitStatus);
  void cancelPoll();
  void publishDeviceIntelligence();
  QProcess pollProcess_;
  QTimer pollDeadline_;
  QByteArray pollOutput_;
  qsizetype pollErrorBytes_=0;
  bool polling_=false,pollSuperseded_=false,pollOverflow_=false;
  QVariantMap lastIntelligence_;
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
  QVariantMap driverBinding_;
  QVariantMap discoveryObservations_,usbObservation_;
  QString propertiesObservedAtUtc_;
  QTimer diagnosisClock_;
  struct ServiceStep { QStringList arguments; QString key; bool optional=false; };
  QVariantMap selectedServiceDevice() const;
  bool serviceAvailable(const QString &action, QString *reason=nullptr) const;
  void initServices();
  void shutdownServices();
  void serviceNext();
  void serviceStepFinished(bool success, const QString &out, const QString &err);
  void serviceAfterState();
  void serviceDispatch();
  void serviceComplete(bool success, const QString &reason);
  void serviceInvalidate();
  bool serviceRunning_=false;
  QString serviceAction_,serviceTarget_,serviceTool_,servicePhase_,serviceOutput_;
  ServiceStep serviceCurrent_;
  QQueue<ServiceStep> serviceSteps_;
  QVariantMap serviceProfile_,serviceScratch_,serviceApproval_;
  QVariantList serviceHistory_;
  QTimer serviceExpiry_;
  QFutureWatcher<QVariantMap> serviceCopy_;
  std::shared_ptr<std::atomic_bool> serviceCancel_;
  std::unique_ptr<QTemporaryDir> serviceDirectory_;
};

