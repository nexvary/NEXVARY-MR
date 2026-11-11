#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QTimer>
#include <QVariantList>
#include <QJsonObject>
#include <QSet>
class QNetworkReply;
class IdentityService;
class GovernmentOperationPolicy;
// Streamable HTTP MCP client. Endpoint/key come from the user's provider account;
// no guessed vendor REST routes, automatic searches, or plaintext credential fallback.
class LampyreConnection final:public QObject {
 Q_OBJECT
 Q_PROPERTY(QString endpoint READ endpoint NOTIFY changed)
 Q_PROPERTY(bool hasCredential READ hasCredential NOTIFY changed)
 Q_PROPERTY(bool connected READ connected NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString state READ state NOTIFY changed)
 Q_PROPERTY(QString errorCode READ errorCode NOTIFY changed)
 Q_PROPERTY(QVariantList tools READ tools NOTIFY changed)
 Q_PROPERTY(QString resultText READ resultText NOTIFY changed)
 Q_PROPERTY(QVariantList resultItems READ resultItems NOTIFY changed)
public:
 explicit LampyreConnection(const QString &directory,IdentityService *identity,GovernmentOperationPolicy *policy,QObject *parent=nullptr,bool syntheticTests=false,int timeoutMs=30000);
 ~LampyreConnection() override;
 QString endpoint()const{return endpoint_;}
 bool hasCredential()const{return !key_.isEmpty();}
 bool connected()const{return connected_;}
 bool busy()const{return busy_;}
 QString state()const{return state_;}
 QString errorCode()const{return error_;}
 QVariantList tools()const{return tools_;}
 QString resultText()const{return result_;}
 QVariantList resultItems()const;
 Q_INVOKABLE bool configure(const QString &endpoint,const QString &key,bool remember);
 Q_INVOKABLE bool connectProvider(bool consent);
 Q_INVOKABLE bool callTool(const QString &name,const QString &argumentsJson,bool paidRequestConsent);
 Q_INVOKABLE void cancel();
 Q_INVOKABLE void forget();
 Q_INVOKABLE bool exportResult(const QString &path,bool consent);
 static bool validEndpoint(const QString &endpoint,bool syntheticTests=false);
 static bool validKey(const QString &key);
signals:
 void changed();
private:
 void reset();
 void load();
 bool save();
 QString credentialPath()const;
 bool fail(const QString &code);
 bool reject(const QString &code);
 bool authorized();
 void post(const QString &method,const QJsonObject &params,const QString &stage,bool notification=false);
 void read(QNetworkReply *reply,bool ended);
 void accept(QNetworkReply *reply,const QJsonObject &message);
 void finishReceipt(const QString &status);
 QString directory_,owner_,endpoint_,key_,state_="disconnected",error_,result_,session_,version_,stage_,operation_,resultTool_,resultAt_;
 IdentityService *identity_;
 GovernmentOperationPolicy *policy_;
 bool synthetic_=false,connected_=false,busy_=false;
 int timeoutMs_,pages_=0;
 qint64 nextId_=0,pendingId_=0;
 QVariantList tools_;
 QSet<QString> cursors_;
 QNetworkAccessManager manager_;
 QPointer<QNetworkReply> reply_;
 QByteArray body_;
 QTimer timer_;
};
