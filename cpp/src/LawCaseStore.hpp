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
 QByteArray readEvidence(const QString &id,const QString &evidence);
 QVariantMap recordAnalysis(const QString &id,qint64 revision,const QString &evidence,const QString &parser,const QVariantMap &report,const QString &actor);
 QVariantMap exportProtectedBackup(const QString &id,qint64 revision,const QString &path,const QString &actor);
 QString restoreProtectedBackup(const QString &path,const QString &expectedSha256,const QString &actor,const std::function<bool(const QVariantMap&)> &authorize);
 bool retentionOperation(const QString &id,qint64 revision,const QString &operation,const QVariantMap &fields,const QString &actor);
 QVariantMap verifyArchive(const QString &id,const QString &archiveId);
 bool verifyEvidence(const QString &id,const QString &evidence);
 QVariantMap exportWorkingCopy(const QString &id,qint64 revision,const QString &evidence,const QString &directory,const QString &purpose,const QString &actor);
private:
 struct BackupData {QVariantMap record;QVariantList blobs;QString sha256;};
 bool readProtectedBackup(const QString &path,const QString &expectedSha256,BackupData &result);
 bool fail(const QString &message);
 QByteArray evidenceBytes(const QString &id,const QVariantMap &item);
 bool save(QVariantMap record,qint64 expected);
 bool mutate(const QString &id,qint64 revision,const QString &event,const QString &actor,const std::function<bool(QVariantMap&)> &change);
 void append(QVariantMap &record,const QString &event,const QString &actor);
 bool valid(const QVariantMap &record);
 QString connection_,error_;
 QSqlDatabase db_;
 bool ready_=false,synthetic_=false;
};
