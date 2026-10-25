#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <memory>
#include "nexvary/LawPermissions.hpp"
class LawCaseStore;
class ToolBridge;
class IdentityService;
// Production authorization comes from authenticated local identities and case grants.
class LawCaseBridge final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool ready READ ready NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QString protection READ protection CONSTANT)
 Q_PROPERTY(QString role READ role NOTIFY changed)
 Q_PROPERTY(QVariantMap permissions READ permissions NOTIFY changed)
 Q_PROPERTY(QVariantList cases READ cases NOTIFY changed)
 Q_PROPERTY(QVariantMap activeCase READ activeCase NOTIFY changed)
 Q_PROPERTY(QVariantList devices READ devices NOTIFY changed)
 Q_PROPERTY(QVariantList audit READ audit NOTIFY changed)
 Q_PROPERTY(QVariantList scenarios READ scenarios NOTIFY changed)
 Q_PROPERTY(QVariantList trainingAttempts READ trainingAttempts NOTIFY trainingChanged)
 Q_PROPERTY(QVariantList accessAssessment READ assessDeviceAccess NOTIFY accessChanged)
public:
 explicit LawCaseBridge(const QString &path,ToolBridge *tools,bool syntheticTestProfile=false,
  nexvary::law::Role role=nexvary::law::Role::Investigator,QObject *parent=nullptr,IdentityService *identity=nullptr);
 ~LawCaseBridge() override;
 bool ready()const;
 QString error()const{return error_;}
 QString protection()const;
 QString role()const;
 QVariantMap permissions()const;
 QVariantList cases()const{return cases_;}
 QVariantMap activeCase()const{return active_;}
 QVariantList devices()const{return devices_;}
 QVariantList audit()const{return audit_;}
 QVariantList scenarios()const;
 QVariantList trainingAttempts()const{return attempts_;}
 Q_INVOKABLE QString createCase(const QVariantMap &metadata);
 Q_INVOKABLE bool updateCase(const QVariantMap &metadata);
 Q_INVOKABLE bool openCase(const QString &id);
 Q_INVOKABLE void closeWorkspace();
 Q_INVOKABLE bool searchCases(const QString &query=QString());
 Q_INVOKABLE bool transitionCase(const QString &status);
 Q_INVOKABLE QString addDevice(const QVariantMap &metadata);
 Q_INVOKABLE QVariantMap currentDeviceObservation()const;
 Q_INVOKABLE QVariantList assessDeviceAccess()const;
 Q_INVOKABLE QString addEvidence(const QString &device,const QString &path,const QString &description,const QString &custodian,bool consent);
 Q_INVOKABLE bool transferCustody(const QString &evidence,const QString &from,const QString &to,const QString &reason);
 Q_INVOKABLE QVariantMap analyzeEvidence(const QString &evidence,const QString &parser);
 Q_INVOKABLE bool verifyEvidence(const QString &evidence);
 Q_INVOKABLE QVariantMap exportWorkingCopy(const QString &evidence,const QString &directory,const QString &purpose,bool consent);
 Q_INVOKABLE bool verifyAudit();
 Q_INVOKABLE bool exportAuditAnchor(const QString &path,bool consent);
 Q_INVOKABLE bool verifyAuditAnchor(const QString &path);
 Q_INVOKABLE QVariantMap evaluateTraining(const QString &scenarioId,const QString &choiceId);
 Q_INVOKABLE bool exportTrainingReport(const QString &path,bool consent);
 Q_INVOKABLE bool exportCasePdf(const QString &path,bool arabic,bool consent);
signals:
 void changed();
 void trainingChanged();
 void accessChanged();
private:
 bool permit(nexvary::law::Action action,bool caseBound=true);
 bool caseAllowed(const QVariantMap &record)const;
 nexvary::law::Role currentRole()const;
 bool fail(const QString &message);
 bool validateMetadata(const QVariantMap &metadata);
 bool refresh(const QString &id=QString());
 QString actor()const;
 std::unique_ptr<LawCaseStore> store_;
 ToolBridge *tools_=nullptr;
 IdentityService *identity_=nullptr;
 bool synthetic_=false;
 nexvary::law::Role role_;
 QString error_;
 QVariantList cases_,devices_,audit_,attempts_;
 QVariantMap active_;
};
