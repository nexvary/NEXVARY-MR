#include "LawCaseStore.hpp"
#include "CaseVault.hpp"
#include "RecoveryFile.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSet>
#include <QUuid>
namespace {
constexpr qint64 maximumBackup=64*1024*1024,maximumPayloads=32*1024*1024,maximumBlob=18*1024*1024;
QString now(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString uuid(){return QUuid::createUuid().toString(QUuid::WithoutBraces);}
QByteArray json(const QVariantMap &m){return QJsonDocument::fromVariant(m).toJson(QJsonDocument::Compact);}
QString hash(const QByteArray &v){return QString::fromLatin1(QCryptographicHash::hash(v,QCryptographicHash::Sha256).toHex());}
QString provider(){return mr::recovery::vaultAvailable()?"windows-current-user-dpapi":"synthetic-test-only";}
bool identifier(const QString &s){return !QUuid(s).isNull()&&s==QUuid(s).toString(QUuid::WithoutBraces);}
QByteArray decode(const QVariant &value,qint64 limit){
 if(value.metaType().id()!=QMetaType::QString)return {};
 const auto text=value.toString().toLatin1();if(text.isEmpty()||text.size()>((limit+2)/3)*4)return {};
 const auto bytes=QByteArray::fromBase64(text,QByteArray::AbortOnBase64DecodingErrors);
 return bytes.size()<=limit&&bytes.toBase64()==text?bytes:QByteArray();
}
bool evidenceMatches(const QByteArray &bytes,const QVariantMap &item){const auto size=QJsonObject::fromVariantMap(item).value("size");return size.isDouble()&&!bytes.isEmpty()&&bytes.size()<=16*1024*1024&&size.toDouble()==static_cast<double>(bytes.size())&&hash(bytes)==item.value("sha256").toString();}
}
QVariantMap LawCaseStore::exportProtectedBackup(const QString &id,qint64 revision,const QString &input,const QString &actor) {
 const auto path=mr::recovery::localFilePath(input);if(path.isEmpty()||QFileInfo(path).exists()||QFileInfo(path).isSymLink()){fail("Choose a new regular local protected-backup file.");return {};}
 bool created=false;QVariantMap result;
 const bool ok=mutate(id,revision,"case-backup-exported",actor,[&](QVariantMap &r){
  auto backups=r.value("backups").toList();if(backups.size()>=100)return fail("Case backup history limit reached.");
  QSqlQuery q(db_);q.prepare("SELECT length(payload),payload FROM law_cases WHERE id=? AND revision=?");q.addBindValue(id);q.addBindValue(revision);
  if(!q.exec()||!q.next()||q.value(0).toLongLong()>4*1024*1024)return fail("Protected case snapshot unavailable.");
  const auto casePayload=q.value(1).toByteArray();qint64 total=casePayload.size();QVariantList blobs;
  for(const auto &value:r.value("evidence").toList()){
   const auto item=value.toMap();
   QSqlQuery evidence(db_);evidence.prepare("SELECT length(payload),payload FROM law_evidence WHERE id=? AND case_id=?");evidence.addBindValue(item.value("id"));evidence.addBindValue(id);
   if(!evidence.exec()||!evidence.next())return fail("Backup evidence payload missing.");
   const auto size=evidence.value(0).toLongLong();if(size<=0||size>maximumBlob||size>maximumPayloads-total)return fail("Protected backup payload limit is 32 MiB per snapshot.");
   const auto payload=evidence.value(1).toByteArray();if(payload.size()!=size)return fail("Backup payload changed.");
   const auto bytes=mr::recovery::openCaseBytes(payload,synthetic_,error_);if(bytes.isEmpty())return false;if(!evidenceMatches(bytes,item))return fail("Backup source evidence size or SHA-256 mismatch.");total+=size;
   blobs.append(QVariantMap{{"id",item.value("id")},{"payloadBase64",QString::fromLatin1(payload.toBase64())}});
  }
  const auto backupId=uuid();const auto time=now();
  const QVariantMap envelope{{"schema","nexvary-protected-case-backup/v1"},{"schemaVersion",1},{"provider",provider()},{"synthetic",synthetic_},{"backupId",backupId},{"createdAtUtc",time},{"caseId",id},{"caseRevision",revision},{"casePayloadBase64",QString::fromLatin1(casePayload.toBase64())},{"evidence",blobs}};
  const auto sealed=mr::recovery::sealCase(json(envelope),synthetic_,error_);if(sealed.isEmpty()||sealed.size()>maximumBackup)return fail("Protected backup exceeds its 64 MiB container limit.");
  QFile output(path);if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail("Protected backup output already exists or cannot be created.");created=true;
  if(!output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||output.write(sealed)!=sealed.size()||!output.flush())return fail("Protected backup write failed.");
  result={{"backupId",backupId},{"sha256",hash(sealed)},{"sourceRevision",revision},{"sourceAuditHead",r.value("audit").toList().last().toMap().value("hash")},{"atUtc",time},{"actor",actor},{"provider",provider()}};
  backups.append(result);r.insert("backups",backups);return true;
 });
 if(!ok){if(created)QFile::remove(path);return {};}
 return result;
}
bool LawCaseStore::readProtectedBackup(const QString &input,const QString &expectedSha256,BackupData &result) {
 if(!QRegularExpression("^[0-9a-fA-F]{64}$").match(expectedSha256).hasMatch())return fail("Provide the separately retained backup SHA-256 before restoration.");
 const auto path=mr::recovery::localFilePath(input);QFile file(path);
 if(path.isEmpty()||QFileInfo(path).isSymLink()||!mr::recovery::openRegularInput(file)||file.size()<=0||file.size()>maximumBackup)return fail("Choose a regular protected backup up to 64 MiB.");
 const auto sealed=file.read(maximumBackup+1);if(sealed.size()!=file.size()||sealed.size()>maximumBackup)return fail("Protected backup changed or exceeds limit.");
 if(hash(sealed)!=expectedSha256.toLower())return fail("Backup SHA-256 does not match the separately retained value.");
 const auto plain=mr::recovery::openCaseBytes(sealed,synthetic_,error_);if(plain.isEmpty())return false;
 if(plain.size()>48*1024*1024)return fail("Decoded backup exceeds limit.");
 QJsonParseError parse;const auto doc=QJsonDocument::fromJson(plain,&parse);const auto object=doc.object();const auto envelope=object.toVariantMap();
 const QSet<QString> keys{"schema","schemaVersion","provider","synthetic","backupId","createdAtUtc","caseId","caseRevision","casePayloadBase64","evidence"};
 for(auto it=envelope.cbegin();it!=envelope.cend();++it)if(!keys.contains(it.key()))return fail("Unknown protected backup fields.");
 if(!object.value("schemaVersion").isDouble()||object.value("schemaVersion").toDouble()!=1||!object.value("caseRevision").isDouble()||!object.value("evidence").isArray())return fail("Invalid protected backup field types.");
 if(parse.error!=QJsonParseError::NoError||!doc.isObject()||envelope.size()!=10||envelope.value("schema")!="nexvary-protected-case-backup/v1"||envelope.value("schemaVersion").toInt()!=1||envelope.value("provider")!=provider()||envelope.value("synthetic").metaType().id()!=QMetaType::Bool||envelope.value("synthetic").toBool()!=synthetic_||!identifier(envelope.value("backupId").toString())||!envelope.value("createdAtUtc").toString().endsWith('Z')||!QDateTime::fromString(envelope.value("createdAtUtc").toString(),Qt::ISODateWithMs).isValid())return fail("Protected backup schema, provider or profile mismatch.");
 const auto casePayload=decode(envelope.value("casePayloadBase64"),4*1024*1024);if(casePayload.isEmpty())return fail("Invalid backup case payload.");
 const auto casePlain=mr::recovery::openCaseBytes(casePayload,synthetic_,error_);if(casePlain.isEmpty())return false;
 const auto caseDoc=QJsonDocument::fromJson(casePlain,&parse);auto record=caseDoc.object().toVariantMap();
 if(!caseDoc.object().value("schema").isDouble()||caseDoc.object().value("schema").toDouble()!=1||!caseDoc.object().value("synthetic").isBool()||!caseDoc.object().value("devices").isArray()||!caseDoc.object().value("evidence").isArray()||!caseDoc.object().value("revision").isDouble()||!QStringList{"draft","suspended","closed","archived"}.contains(record.value("status").toString()))return fail("Invalid backup case structure.");
 if(parse.error!=QJsonParseError::NoError||!caseDoc.isObject()||!identifier(record.value("id").toString())||record.value("id")!=envelope.value("caseId")||record.value("revision")!=envelope.value("caseRevision")||record.value("revision").toLongLong()>=10000||!valid(record))return fail("Backup case audit or identity did not verify.");
 const auto devices=record.value("devices").toList(),items=record.value("evidence").toList(),blobs=envelope.value("evidence").toList();
 if(devices.size()>100||items.size()>100||items.size()!=blobs.size()||(!caseDoc.object().value("deviceCount").isDouble()||caseDoc.object().value("deviceCount").toDouble()!=static_cast<double>(devices.size())))return fail("Backup device/evidence inventory mismatch.");
 QSet<QString> deviceIds,evidenceIds;for(const auto &v:devices){const auto id=v.toMap().value("id").toString();if(!identifier(id)||deviceIds.contains(id))return fail("Invalid backup device identifiers.");deviceIds.insert(id);}
 qint64 total=casePayload.size();QVariantList decoded;
 for(const auto &value:blobs){
  const auto blob=value.toMap();const auto id=blob.value("id").toString();QVariantMap item;
  for(const auto &e:items)if(e.toMap().value("id").toString()==id)item=e.toMap();
  if(blob.size()!=2||!identifier(id)||evidenceIds.contains(id)||item.isEmpty()||!deviceIds.contains(item.value("deviceId").toString()))return fail("Backup evidence inventory contains missing or duplicate identifiers.");
  evidenceIds.insert(id);const auto payload=decode(blob.value("payloadBase64"),maximumBlob);if(payload.isEmpty()||payload.size()>maximumPayloads-total)return fail("Backup evidence exceeds payload limit.");total+=payload.size();
  const auto bytes=mr::recovery::openCaseBytes(payload,synthetic_,error_);if(bytes.isEmpty())return false;if(!evidenceMatches(bytes,item))return fail("Backup evidence size or SHA-256 mismatch.");
  decoded.append(QVariantMap{{"id",id},{"payload",payload}});
 }
 if(evidenceIds.size()!=items.size())return fail("Backup evidence is incomplete.");
 result={record,decoded,hash(sealed)};return true;
}
QString LawCaseStore::restoreProtectedBackup(const QString &path,const QString &expectedSha256,const QString &actor,const std::function<bool(const QVariantMap&)> &authorize) {
 error_.clear();if(!ready_||actor.trimmed().isEmpty()||actor.toUtf8().size()>256){fail("Protected restore storage/actor unavailable.");return {};}
 BackupData source;if(!readProtectedBackup(path,expectedSha256,source))return {};
 if(!authorize(source.record)){fail("Current identity has no restore permission or case grant for this backup.");return {};}
 auto record=source.record;const auto id=record.value("id").toString();const auto revision=record.value("revision").toLongLong();
 auto restores=record.value("restorations").toList();if(restores.size()>=100){fail("Case restoration history limit reached.");return {};}
 restores.append(QVariantMap{{"id",uuid()},{"backupSha256",source.sha256},{"sourceRevision",revision},{"sourceAuditHead",record.value("audit").toList().last().toMap().value("hash")},{"actor",actor},{"atUtc",now()}});
 record.insert("restorations",restores);record.insert("revision",revision+1);append(record,"case-backup-restored",actor);
 if(!valid(record))return {};
 if(json(record).size()>3*1024*1024){fail("Restored case exceeds the bounded record limit.");return {};}
 const auto protectedRecord=mr::recovery::sealCase(json(record),synthetic_,error_);if(protectedRecord.isEmpty())return {};
 if(!db_.transaction()){fail("Cannot start atomic case restoration.");return {};}
 const auto abort=[&](const QString &message){db_.rollback();fail(message);return QString();};
 QSqlQuery existing(db_);existing.prepare("SELECT 1 FROM law_cases WHERE id=?");existing.addBindValue(id);
 if(!existing.exec())return abort("Cannot inspect existing cases.");
 if(existing.next())return abort("This case already exists; restore never overwrites cases.");
 existing.finish();
 QSqlQuery insert(db_);insert.prepare("INSERT INTO law_cases(id,revision,payload) VALUES(?,?,?)");insert.addBindValue(id);insert.addBindValue(revision+1);insert.addBindValue(protectedRecord);
 if(!insert.exec())return abort("Case restoration insert failed.");
 for(const auto &v:source.blobs){const auto blob=v.toMap();QSqlQuery q(db_);q.prepare("INSERT INTO law_evidence(id,case_id,payload) VALUES(?,?,?)");q.addBindValue(blob.value("id"));q.addBindValue(id);q.addBindValue(blob.value("payload"));if(!q.exec())return abort("Evidence identifier conflict or restoration write failure.");}
 if(!db_.commit())return abort("Case restoration commit failed.");
 return id;
}
