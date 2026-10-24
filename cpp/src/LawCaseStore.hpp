#pragma once
#include <QSqlDatabase>
#include <QVariantMap>
#include <QVariantList>
#include <functional>
// Internal persistence boundary; user-facing callers must use LawCaseBridge.
class LawCaseStore final {
public:
 explicit LawCaseStore(const QString &path,bool synthetic=false);
 ~LawCaseStore();
 LawCaseStore(const LawCaseStore&)=delete;
 LawCaseStore &operator=(const LawCaseStore&)=delete;
 bool ready()const{return ready_;}
 QString error()const{return error_;}
 QVariantMap get(const QString &id);
 QVariantList list(const QString &query={});
 QString create(const QVariantMap &metadata,const QString &actor,const QString &ownerId=QString());
 bool update(const QString &id,qint64 revision,const QVariantMap &metadata,const QString &actor);
 bool transition(const QString &id,qint64 revision,const QString &status,const QString &actor);
 QString addDevice(const QString &id,qint64 revision,const QVariantMap &metadata,const QString &actor);
 QVariantList devices(const QString &id);
 QVariantList audit(const QString &id);
 bool verifyAudit(const QString &id);
 QString addEvidence(const QString &id,qint64 revision,const QString &device,const QString &path,const QString &description,const QString &custodian,const QString &actor);
 bool transfer(const QString &id,qint64 revision,const QString &evidence,const QString &from,const QString &to,const QString &reason,const QString &actor);
 bool verifyEvidence(const QString &id,const QString &evidence);
private:
 bool fail(const QString &message);
 bool save(QVariantMap record,qint64 expected);
 bool mutate(const QString &id,qint64 revision,const QString &event,const QString &actor,const std::function<bool(QVariantMap&)> &change);
 void append(QVariantMap &record,const QString &event,const QString &actor);
 bool valid(const QVariantMap &record);
 QString connection_,error_;
 QSqlDatabase db_;
 bool ready_=false,synthetic_=false;
};
