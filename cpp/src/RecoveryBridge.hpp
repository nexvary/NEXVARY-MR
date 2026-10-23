#pragma once
#include <QObject>
#include <QJsonObject>
#include <QFutureWatcher>
#include <QVariantList>
#include <atomic>
#include <functional>
#include <memory>
class RecoveryBridge final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool active READ active NOTIFY changed)
 Q_PROPERTY(bool busy READ busy NOTIFY changed)
 Q_PROPERTY(double progress READ progress NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QString status READ status NOTIFY changed)
 Q_PROPERTY(QString casePath READ casePath NOTIFY changed)
 Q_PROPERTY(QString caseTitle READ caseTitle NOTIFY changed)
 Q_PROPERTY(QString caseId READ caseId NOTIFY changed)
 Q_PROPERTY(QString caseOperator READ caseOperator NOTIFY changed)
 Q_PROPERTY(QString authorization READ authorization NOTIFY changed)
 Q_PROPERTY(QVariantList timeline READ timeline NOTIFY changed)
 Q_PROPERTY(QString protection READ protection CONSTANT)
 Q_PROPERTY(bool vaultReady READ vaultReady CONSTANT)
 Q_PROPERTY(QVariantList evidence READ evidence NOTIFY changed)
 Q_PROPERTY(QVariantList audit READ audit NOTIFY changed)
 Q_PROPERTY(QVariantMap device READ device NOTIFY changed)
 Q_PROPERTY(QString lossKind READ lossKind NOTIFY changed)
 Q_PROPERTY(QString encryptionKind READ encryptionKind NOTIFY changed)
public:
 explicit RecoveryBridge(const QString &directory,bool syntheticTestProfile=false,QObject *parent=nullptr);
 ~RecoveryBridge() override;
 bool active() const{return !document_.isEmpty();}
 bool busy() const{return watcher_.isRunning()||running_;}
 double progress() const{return progress_;}
 QString error() const{return error_;}
 QString status() const{return status_;}
 QString casePath() const{return path_;}
 QString caseTitle() const{return document_.value("title").toString();}
 QString caseId() const{return document_.value("id").toString();}
 QString caseOperator() const{return document_.value("operator").toString();}
 QString authorization() const{return document_.value("authorization").toString();}
 QVariantList timeline() const;
 QString protection() const;
 bool vaultReady() const;
 QVariantList evidence() const;
 QVariantList audit() const;
 QVariantMap device() const;
 QString lossKind()const{return document_.value("loss").toString();}
 QString encryptionKind()const{return document_.value("encryption").toString();}
 Q_INVOKABLE QString assess(const QString &loss,const QString &encryption,bool external,bool backup,bool arabic) const;
 Q_INVOKABLE bool createCase(const QString &title,const QString &ownerReference,const QString &brand,const QString &model,const QString &androidVersion,const QString &loss,const QString &encryption,bool consent);
 Q_INVOKABLE bool openCase(const QString &path);
 Q_INVOKABLE bool importAgent(const QString &path,bool consent);
 Q_INVOKABLE bool importVisibleFile(const QString &path,bool consent);
 Q_INVOKABLE bool scanImage(const QString &path,bool consent);
 Q_INVOKABLE bool exportReport(const QString &path,bool plaintextConfirmed);
 Q_INVOKABLE bool exportEvidence(int index,const QString &path,bool plaintextConfirmed);
 Q_INVOKABLE bool recordOperator(const QString &name,const QString &authorization,bool confirmed);
 Q_INVOKABLE bool verifyEvidence();
 Q_INVOKABLE QString previewEvidence(int index) const;
 Q_INVOKABLE QString evidenceCustodian(int index) const;
 Q_INVOKABLE bool transferCustody(int index,const QString &recipient,const QString &reason,bool confirmed);
 Q_INVOKABLE bool exportFormattedReport(const QString &path,const QString &format,bool arabic,bool confirmed);
 Q_INVOKABLE bool analyzeCapture(const QString &path,bool consent);
 Q_INVOKABLE bool analyzeExif(const QString &path,bool consent);
 Q_INVOKABLE bool analyzePartitions(const QString &image,const QString &mmlsExecutable,bool consent);
 Q_INVOKABLE bool exportTimeline(const QString &path,bool confirmed);
 Q_INVOKABLE bool deleteCase(bool confirmed);
 Q_INVOKABLE bool closeCase();
 Q_INVOKABLE void cancel();
signals:
 void changed();
private:
 struct JobResult {QJsonObject document;QString error;};
 using Job=std::function<JobResult(const QJsonObject &,const std::shared_ptr<std::atomic_bool> &)>;
 bool start(Job job,const QString &destination=QString());
 bool fail(const QString &message);
 bool allowedOutput(const QString &path) const;
 static bool validate(const QJsonObject &document,QString &error);
 static bool persist(const QJsonObject &document,const QString &path,bool synthetic,QString &error);
 static void addAudit(QJsonObject &document,const QString &operation,const QString &outcome);
 QString directory_,path_,pendingPath_,error_,status_;
 QJsonObject document_;
 bool synthetic_=false,running_=false;
 double progress_=0;
 std::shared_ptr<std::atomic_bool> cancelled_;
 QFutureWatcher<JobResult> watcher_;
};

