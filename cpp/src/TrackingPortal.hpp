#pragma once
#include <QObject>
class GovernmentOperationPolicy;
#include <QTcpServer>
#include <QStringList>
class WorkshopBridge;
class TrackingPortal final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool running READ running NOTIFY changed)
 Q_PROPERTY(QString baseUrl READ baseUrl NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QStringList addresses READ addresses NOTIFY changed)
public:
 void setOperationPolicy(GovernmentOperationPolicy *policy);
 explicit TrackingPortal(WorkshopBridge *workshop,QObject *parent=nullptr);
 bool running() const {return server_.isListening();}
 QString baseUrl() const;
 QString error() const {return error_;}
 QStringList addresses() const;
 Q_INVOKABLE bool start(const QString &address,int port,bool trustedNetworkConsent);
 Q_INVOKABLE void stop();
 Q_INVOKABLE QString link(int orderId,int days,bool sharingConsent);
 Q_INVOKABLE bool copyLink(const QString &url);
 static QByteArray html(const QVariantMap &snapshot);
signals:
 void changed();
private:
 GovernmentOperationPolicy *policy_=nullptr;
 void accept();
 WorkshopBridge *workshop_;
 QTcpServer server_;
 QString error_;
 int clients_=0;
};
