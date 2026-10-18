#pragma once
#include <QObject>
#include <QVariantMap>
#include <QFutureWatcher>
#include <atomic>
#include <memory>
struct SecurityLimits {qint64 maximumFile=128LL*1024*1024;int timeoutMs=120000;qsizetype maximumOutput=512*1024;};
QVariantMap scanSecurityFile(const QString &source,const QString &engine,const QString &database,const std::shared_ptr<std::atomic_bool> &cancelled,const SecurityLimits &limits={});
class SecurityBridge final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString executable READ executable NOTIFY changed)
 Q_PROPERTY(QString database READ database NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QVariantMap result READ result NOTIFY changed)
public:
 explicit SecurityBridge(const QString &config,QObject *parent=nullptr);
 ~SecurityBridge() override;
 bool busy() const {return active_;}
 QString executable() const {return executable_;}
 QString database() const {return database_;}
 QString error() const {return error_;}
 QVariantMap result() const {return result_;}
 Q_INVOKABLE bool configure(const QString &executable,const QString &database);
 Q_INVOKABLE bool scan(const QString &source,bool authorized);
 Q_INVOKABLE void cancel();
 Q_INVOKABLE bool exportReport(const QString &path,bool consent);
signals:
 void changed();
private:
 QString config_,executable_,database_,error_;
 QVariantMap result_;
 bool active_=false;
 QFutureWatcher<QVariantMap> watcher_;
 std::shared_ptr<std::atomic_bool> cancelled_;
};
