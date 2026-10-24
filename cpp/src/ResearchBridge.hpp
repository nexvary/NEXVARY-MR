#pragma once
#include <QObject>
#include <QVariantMap>
#include <QVariantList>
class ToolBridge;
class FirmwareBridge;
class IdentityService;
class ResearchBridge final:public QObject {
 Q_OBJECT
 Q_PROPERTY(bool ready READ ready NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
 Q_PROPERTY(QVariantMap catalog READ catalog CONSTANT)
 Q_PROPERTY(QVariantList results READ results NOTIFY changed)
 Q_PROPERTY(QVariantList compatibility READ compatibility NOTIFY changed)
 Q_PROPERTY(QVariantMap engineContext READ engineContext NOTIFY contextChanged)
public:
 ResearchBridge(const QString &path,ToolBridge *tools=nullptr,FirmwareBridge *firmware=nullptr,QObject *parent=nullptr,IdentityService *identity=nullptr);
 bool ready()const{return ready_;}
 QString error()const{return error_;}
 QVariantMap catalog()const{return catalog_;}
 QVariantList results()const{return state_.value("results").toList();}
 QVariantList compatibility()const{return state_.value("compatibility").toList();}
 QVariantMap engineContext()const;
 Q_INVOKABLE QVariantList search(const QString &category,const QString &query,const QString &status=QString())const;
 Q_INVOKABLE QVariantMap validation(const QString &resultId)const;
 Q_INVOKABLE QString recordResult(const QVariantMap &fields,const QString &evidencePath,bool publicSyntheticConsent);
 Q_INVOKABLE bool reviewResult(const QString &resultId,const QString &evidencePath,const QString &decision,const QString &notes);
 Q_INVOKABLE bool addCompatibility(const QVariantMap &fields);
 Q_INVOKABLE bool exportReport(const QString &path,bool consent);
 static QString validateCatalog(const QVariantMap &catalog);
signals:
 void changed();
 void contextChanged();
private:
 bool fail(const QString &error);
 bool canWrite();
 bool reviewed(const QVariantMap &row)const;
 bool load();
 bool save(const QVariantMap &state);
 QString path_,error_;
 ToolBridge *tools_;
 FirmwareBridge *firmware_;
 IdentityService *identity_;
 QVariantMap catalog_,state_;
 bool ready_=false;
};
