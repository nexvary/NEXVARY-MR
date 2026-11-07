#pragma once
#include <QObject>
class GovernmentOperationPolicy;
#include <QNetworkAccessManager>
#include <QPointer>
#include <QTimer>
class WorkshopBridge;
class QNetworkReply;
class CloudTracking final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool configured READ configured NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(bool inSync READ inSync NOTIFY changed)
 Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QString lastSync READ lastSync NOTIFY changed)
public:
 void setOperationPolicy(GovernmentOperationPolicy *policy);
 CloudTracking(WorkshopBridge *workshop,const QString &storage,bool localTest=false,QObject *parent=nullptr);
 ~CloudTracking() override;
 bool configured() const {return !url_.isEmpty()&&!key_.isEmpty();}
 bool busy() const {return busy_;}
 bool inSync() const {return configured()&&!dirty_&&!lastSync_.isEmpty();}
 QString serverUrl() const {return url_;}
 QString error() const {return error_;}
 QString lastSync() const {return lastSync_;}
 Q_INVOKABLE bool importCredentials(const QString &path,bool approved);
 Q_INVOKABLE bool syncNow();
 Q_INVOKABLE QString issueLink(int orderId,int days,bool sharingApproved);
 Q_INVOKABLE bool copyLink(const QString &url);
 Q_INVOKABLE bool disconnectServer(bool confirmed);
private:
 GovernmentOperationPolicy *policy_=nullptr;
 bool applyCredentials(const QByteArray &bytes,bool save);
 void request(const QString &endpoint,const QByteArray &body={},qint64 revision=-1);
 void fail(const QString &message);
 signals:
 void changed();
private:
 WorkshopBridge *workshop_;
 QString storage_,url_,key_,source_,error_,lastSync_;
 QNetworkAccessManager network_;
 QPointer<QNetworkReply> reply_;
 QTimer periodic_,debounce_;
 bool localTest_=false,busy_=false,dirty_=true;
 quint64 generation_=0,sentGeneration_=0;
};
