#pragma once
#include <QObject>
#include <QVariantList>
#include <QElapsedTimer>
#include <QTimer>
#include <QMap>
class IdentityService;

// Application operation gate, not an OS network/process sandbox. Grants are
// deliberately memory-only and independent of the currently displayed workspace.
class GovernmentOperationPolicy final : public QObject {
 Q_OBJECT
 Q_PROPERTY(bool offline READ offline NOTIFY changed)
 Q_PROPERTY(QVariantList permissions READ permissions NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY errorChanged)
public:
 explicit GovernmentOperationPolicy(IdentityService *identity,QObject *parent=nullptr,int lifetimeMs=600000);
 bool offline() const;
 QVariantList permissions() const;
 QString error() const {return error_;}
 // Native consumers must use permit(), not a QML visibility/enabled flag.
 bool permit(const QString &capability);
 bool granted(const QString &capability) const;
 Q_INVOKABLE bool grant(const QString &capability,const QString &reason,bool approved);
 Q_INVOKABLE void revoke(const QString &capability);
 Q_INVOKABLE void revokeAll();
 Q_INVOKABLE bool openExternal(const QString &url);
 static QStringList capabilities();
signals:
 void changed();
 void errorChanged();
private:
 void refresh();
 IdentityService *identity_;
 struct Grant {QString reason,actor,expiresUtc;qint64 deadline=0;};
 QMap<QString,Grant> grants_;
 QElapsedTimer clock_;
 QTimer monitor_;
 int lifetimeMs_;
 bool refreshing_=false;
 QString error_;
};
