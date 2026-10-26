#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
#include <QElapsedTimer>
#include "nexvary/LawPermissions.hpp"
// Local credentials within the current OS account, not an agency identity provider.
// Synthetic storage is a native test fixture; production has no plaintext fallback.
class IdentityService final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool ready READ ready NOTIFY changed)
 Q_PROPERTY(bool needsSetup READ needsSetup NOTIFY changed)
 Q_PROPERTY(bool authenticated READ authenticated NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QVariantMap session READ session NOTIFY changed)
 Q_PROPERTY(QVariantList users READ users NOTIFY changed)
public:
 explicit IdentityService(const QString &path,bool synthetic=false,QObject *parent=nullptr);
 ~IdentityService() override;
 bool ready()const{return ready_;}
 bool needsSetup()const{return ready_&&state_.value("users").toList().isEmpty();}
 bool authenticated()const{return !userId_.isEmpty();}
 QString error()const{return error_;}
 QVariantMap session()const;
 QVariantList users()const;
 Q_INVOKABLE bool bootstrap(const QString &username,const QString &displayName,const QString &password);
 Q_INVOKABLE bool login(const QString &username,const QString &password);
 Q_INVOKABLE void logout();
 Q_INVOKABLE bool changePassword(const QString &currentPassword,const QString &newPassword);
 Q_INVOKABLE QString createUser(const QString &username,const QString &displayName,const QString &role,const QString &password);
 Q_INVOKABLE bool setEnabled(const QString &userId,bool enabled);
 Q_INVOKABLE bool setCaseGrant(const QString &caseId,const QString &userId,bool granted);
 bool checkSession();
 bool hasCaseGrant(const QString &caseId)const;
 nexvary::law::Role lawRole()const;
 QString userId()const{return userId_;}
 QString actor()const;
 // Native composition only. A policy journal shares the protected identity
 // directory; callers cannot select a plaintext production fallback from QML.
 QString policyJournalPath()const{return path_+".policy-audit";}
 bool syntheticProfile()const{return synthetic_;}
 // These methods are native-only, never arbitrary signing endpoints in QML.
 QVariantMap attest(const QString &purpose,const QVariantMap &payload);
 bool verify(const QVariantMap &envelope,const QString &purpose,const QVariantMap &payload);
signals:
 void changed();
private:
 bool read();
 bool save(QVariantMap next);
 bool fail(const QString &message);
 bool administrator();
 QVariantMap account(const QString &id)const;
 QVariantMap newAccount(const QString &username,const QString &name,const QString &role,const QString &password);
 QString path_,error_,userId_;
 bool synthetic_=false,ready_=false;
 QVariantMap state_;
 QElapsedTimer lifetime_;
 unsigned char *secret_=nullptr;
 QVariantMap sessionAccount_;
};
