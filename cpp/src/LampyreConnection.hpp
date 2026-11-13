#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QTimer>
#include <QVariantList>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <functional>
class QNetworkReply;
class IdentityService;
class GovernmentOperationPolicy;
// Streamable HTTP MCP client. Production destination is fixed internally;
// no guessed vendor REST routes, automatic searches, or plaintext credential fallback.
class LampyreConnection final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool liveProfile READ liveProfile CONSTANT)
 Q_PROPERTY(QVariantMap currentInvoice READ currentInvoice NOTIFY changed)
 Q_PROPERTY(qint64 totalDueMinorUnits READ totalDueMinorUnits NOTIFY changed)
 Q_PROPERTY(QVariantList invoices READ invoices NOTIFY changed)
 Q_PROPERTY(double retailPriceEur READ retailPriceEur CONSTANT)
 Q_PROPERTY(QVariantList categories READ categories NOTIFY changed)
 Q_PROPERTY(QVariantList services READ services NOTIFY changed)
 Q_PROPERTY(double creditBalance READ creditBalance NOTIFY changed)
 Q_PROPERTY(QString selectedCategory READ selectedCategory NOTIFY changed)
 Q_PROPERTY(QString catalogCheckedAt READ catalogCheckedAt NOTIFY changed)
 Q_PROPERTY(bool hasCredential READ hasCredential NOTIFY changed)
 Q_PROPERTY(bool credentialSaved READ credentialSaved NOTIFY changed)
 Q_PROPERTY(int requestLimit READ requestLimit NOTIFY changed)
 Q_PROPERTY(QVariantMap spendingAllowances READ spendingAllowances NOTIFY changed)
 Q_PROPERTY(int submittedRequests READ submittedRequests NOTIFY changed)
 Q_PROPERTY(bool connected READ connected NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(QString state READ state NOTIFY changed)
 Q_PROPERTY(QString errorCode READ errorCode NOTIFY changed)
 Q_PROPERTY(QVariantList tools READ tools NOTIFY changed)
 Q_PROPERTY(QString resultText READ resultText NOTIFY changed)
 Q_PROPERTY(QVariantList resultItems READ resultItems NOTIFY changed)
 Q_PROPERTY(QVariantList findings READ findings NOTIFY changed)
 Q_PROPERTY(int pendingResultCount READ pendingResultCount NOTIFY changed)
public:
 explicit LampyreConnection(const QString &directory,IdentityService *identity,GovernmentOperationPolicy *policy,QObject *parent=nullptr,bool syntheticTests=false,int timeoutMs=120000);
 ~LampyreConnection() override;
 bool liveProfile()const;
 QVariantMap currentInvoice()const{return resultInvoice().toVariantMap();}
 qint64 totalDueMinorUnits()const;
 QVariantList invoices()const;
 QVariantList providerUsage()const{return invoiceRows_;} // Native owner reporting only; never a customer invoice.
 double retailPriceEur()const;
 QVariantList categories()const{return categories_;}
 QVariantList services()const{return services_;}
 double creditBalance()const{return creditBalance_;}
 QString selectedCategory()const{return selectedCategory_;}
 QString catalogCheckedAt()const{return catalogCheckedAt_;}
 Q_INVOKABLE bool refreshCatalog();
 Q_INVOKABLE bool discoverServices(const QString &category);
 Q_INVOKABLE bool runService(const QString &name,const QString &arguments,double approvedPhotons,bool consent);
 QString endpoint()const{return endpoint_;}
 bool hasCredential()const{return !key_.isEmpty();}
 bool credentialSaved()const{return saved_;}
 int requestLimit()const{return requestLimit_;}
 int submittedRequests()const{return submitted_;}
 Q_INVOKABLE bool setRequestLimit(int limit);
 Q_INVOKABLE bool setSpendingLimit(const QString &accountId,int submissions);
 Q_INVOKABLE int remainingSubmissions(const QString &accountId)const;
 QVariantMap spendingAllowances()const;
 void setCaseIdentity(std::function<QString()> reader){caseIdentity_=std::move(reader);}
 void setResultAttachment(std::function<QString(const QString &,const QByteArray &)> writer){attachment_=std::move(writer);}
 Q_INVOKABLE QString attachResult(const QString &device,bool consent);
 void setCaseAuthorization(std::function<bool()> check){caseAuthorization_=std::move(check);}
 bool connected()const{return connected_;}
 bool busy()const{return busy_;}
 QString state()const{return state_;}
 QString errorCode()const{return error_;}
 QVariantList tools()const{return tools_;}
 QString resultText()const{return result_;}
 QVariantList resultItems()const;
 QVariantList findings()const;
 int pendingResultCount()const{return static_cast<int>(pendingTasks_.size());}
 Q_INVOKABLE bool collectResults();
 // Native-only destination override is restricted to loopback synthetic fixtures.
 bool configure(const QString &endpoint,const QString &key,bool remember);
 Q_INVOKABLE bool configureCredential(const QString &key,bool remember);
 Q_INVOKABLE QString displayText(const QString &text)const;
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
 bool acceptTasks(const QJsonObject &result,bool collecting);
 bool freeResultCollector()const;
 void loadInvoices();
 bool writeInvoices(const QVariantList &rows);
 bool prepareInvoice(const QVariantMap &service);
 bool finishInvoice(const QString &status);
 bool freeMetadataTool(const QString &name)const;
 bool metadataCall(const QString &name,const QJsonObject &args,const QString &stage);
 QJsonObject resultInvoice()const;
 bool consumeSubmission();
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
 QString directory_,owner_,endpoint_,key_,state_="disconnected",error_,result_,session_,version_,stage_,operation_,resultTool_,resultAt_,sourceDigest_;
 IdentityService *identity_;
 GovernmentOperationPolicy *policy_;
 bool synthetic_=false,connected_=false,busy_=false,saved_=false,retailQuoted_=false;
 int requestLimit_=0,submitted_=0;
 std::function<bool()> caseAuthorization_;
 std::function<QString()> caseIdentity_;
 std::function<QString(const QString &,const QByteArray &)> attachment_;
 QString resultCaseId_,resultOperationId_;
 int timeoutMs_,pages_=0;
 qint64 nextId_=0,pendingId_=0;
 QJsonArray pendingTasks_,taskRows_;
 QVariantList tools_,categories_,services_,invoiceRows_;
 QString invoiceId_,invoiceOperationId_,resultInvoiceId_;
 QStringList requestSecrets_;
 bool invoicePrepared_=false;
 double creditBalance_=-1;
 QString selectedCategory_,catalogCheckedAt_;
 qint64 serviceCatalogAt_=0;
 QSet<QString> cursors_;
 QNetworkAccessManager manager_;
 QPointer<QNetworkReply> reply_;
 QByteArray body_;
 QTimer timer_;
};
