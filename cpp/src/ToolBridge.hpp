#pragma once
#include <QObject>
#include <QProcess>
#include <QString>
#include <QVariantList>
class ToolBridge final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(QVariantList tools READ tools NOTIFY toolsChanged)
public:
  explicit ToolBridge(QObject *parent = nullptr);
  QString status() const { return status_; }
  bool busy() const { return process_.state() != QProcess::NotRunning; }
  QVariantList tools() const;
  Q_INVOKABLE void refreshTools();
  Q_INVOKABLE void scanAdb();
  Q_INVOKABLE void scanFastboot();
  Q_INVOKABLE void startMirror();
signals:
  void statusChanged();
  void busyChanged();
  void toolsChanged();
private:
  void runReadOnly(const QString &program, const QStringList &arguments);
  void setStatus(const QString &value);
  QProcess process_;
  QString status_;
};
