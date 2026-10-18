#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
class ServiceCatalog final:public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList drivers READ drivers CONSTANT)
 Q_PROPERTY(QVariantList guides READ guides NOTIFY changed)
 Q_PROPERTY(QString error READ error NOTIFY changed)
public:
 explicit ServiceCatalog(const QString &storage,QObject *parent=nullptr);
 QVariantList drivers() const;
 QVariantList guides() const {return guides_;}
 QString error() const {return error_;}
 Q_INVOKABLE QVariantMap tree(const QString &brand,const QString &model,const QString &symptom,bool arabic) const;
 Q_INVOKABLE bool importGuide(const QString &path,bool rightsReviewed);
 static bool validGuide(const QVariantMap &guide);
signals:
 void changed();
private:
 QString storage_,error_;
 QVariantList guides_;
};
