#include "LawCaseStore.hpp"
#include "LawRetention.hpp"
#include "CaseVault.hpp"
#include "RecoveryFile.hpp"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QUuid>
namespace {
QString now(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString uuid(){return QUuid::createUuid().toString(QUuid::WithoutBraces);}
QByteArray encode(const QVariantMap &v){return QJsonDocument(QJsonObject::fromVariantMap(v)).toJson(QJsonDocument::Compact);}
QString digest(const QByteArray &v){return QString::fromLatin1(QCryptographicHash::hash(v,QCryptographicHash::Sha256).toHex());}
QString stateHash(QVariantMap r){r.remove("audit");return digest(encode(r));}
bool text(const QString &s,int limit=8192){if(s.trimmed().isEmpty()||s.toUtf8().size()>limit)return false;for(const auto c:s)if((c.unicode()<32&&c!='\n'&&c!='\t')||c.unicode()==127||(c.unicode()>=0x202a&&c.unicode()<=0x202e)||(c.unicode()>=0x2066&&c.unicode()<=0x2069))return false;return true;}
}
LawCaseStore::LawCaseStore(const QString &path,bool synthetic):synthetic_(synthetic) {
 if(!synthetic&&!mr::recovery::vaultAvailable()){fail("Production cases require Windows current-account DPAPI.");return;}
 const auto local=mr::recovery::localFilePath(path);
 if(local.isEmpty()||QFileInfo(local).isSymLink()||!QDir().mkpath(QFileInfo(local).absolutePath())){fail("Invalid case database path.");return;}
 connection_="law-"+uuid();db_=QSqlDatabase::addDatabase("QSQLITE",connection_);db_.setDatabaseName(local);
 db_.setConnectOptions("QSQLITE_BUSY_TIMEOUT=3000");
 if(!db_.open()){fail("Case database could not open.");return;}
 if(!QFile::setPermissions(local,QFileDevice::ReadOwner|QFileDevice::WriteOwner)){fail("Cannot restrict database file permissions.");return;}
 QSqlQuery q(db_);
 for(const auto *sql:{"PRAGMA journal_mode=DELETE","PRAGMA secure_delete=ON","PRAGMA trusted_schema=OFF","CREATE TABLE IF NOT EXISTS law_cases(id TEXT PRIMARY KEY, revision INTEGER NOT NULL, payload BLOB NOT NULL)","CREATE TABLE IF NOT EXISTS law_evidence(id TEXT PRIMARY KEY, case_id TEXT NOT NULL, payload BLOB NOT NULL)"})
  if(!q.exec(QString::fromLatin1(sql))){fail("Case database initialization failed.");return;}
 ready_=true;
}
LawCaseStore::~LawCaseStore(){const auto name=connection_;if(db_.isValid())db_.close();db_=QSqlDatabase();if(!name.isEmpty())QSqlDatabase::removeDatabase(name);}
bool LawCaseStore::fail(const QString &m){error_=m;return false;}
bool LawCaseStore::valid(const QVariantMap &r) {
 if(r.value("schema").toInt()!=1||r.value("synthetic").toBool()!=synthetic_||r.value("revision").toLongLong()<1)return fail("Case schema/profile mismatch.");
 if(!nexvary::law::validRetention(r))return fail("Invalid retention schema.");
 const auto events=r.value("audit").toList();if(events.isEmpty()||events.size()>10000)return fail("Invalid case audit size.");
 QString previous;int sequence=0;
 for(const auto &value:events){auto e=value.toMap();const auto hash=e.take("hash").toString();if(e.value("caseId")!=r.value("id")||e.value("sequence").toInt()!=++sequence||e.value("previous").toString()!=previous||hash!=digest(encode(e)))return fail("Case audit chain verification failed.");previous=hash;}
 if(events.last().toMap().value("stateSha256").toString()!=stateHash(r)||events.size()!=r.value("revision").toLongLong())return fail("Case state does not match its audit head.");
 return true;
}
QVariantMap LawCaseStore::get(const QString &id) {
 error_.clear();if(!ready_||id.size()>128){fail("Case storage unavailable or invalid identifier.");return {};}
 QSqlQuery q(db_);q.prepare("SELECT revision,payload FROM law_cases WHERE id=?");q.addBindValue(id);
 if(!q.exec()||!q.next()){fail("Case not found or database unreadable.");return {};}
 const auto blob=q.value(1).toByteArray();if(blob.size()>4*1024*1024){fail("Case record exceeds limit.");return {};}
 const auto bytes=mr::recovery::openCaseBytes(blob,synthetic_,error_);if(bytes.isEmpty())return {};
 QJsonParseError parse;const auto doc=QJsonDocument::fromJson(bytes,&parse);auto r=doc.object().toVariantMap();
 if(parse.error!=QJsonParseError::NoError||!doc.isObject()||r.value("id").toString()!=id||r.value("revision").toLongLong()!=q.value(0).toLongLong()||!valid(r)){if(error_.isEmpty())fail("Invalid protected case.");return {};}
 return r;
}
QVariantList LawCaseStore::list(const QString &query) {
 error_.clear();QVariantList rows;if(!ready_||query.size()>256){fail("Invalid case query.");return rows;}
 QSqlQuery q(db_);if(!q.exec("SELECT id FROM law_cases ORDER BY rowid DESC LIMIT 1001")){fail("Case search failed.");return {};}
 QStringList ids;while(q.next())ids.append(q.value(0).toString());if(ids.size()>1000){fail("Case list exceeds 1000; archive/export administration required.");return {};}
 for(const auto &id:ids){auto r=get(id);if(r.isEmpty())return {};const auto m=r.value("metadata").toMap();if(query.isEmpty()||QString::fromUtf8(encode(m)).contains(query,Qt::CaseInsensitive))rows.append(r);}
 return rows;
}
void LawCaseStore::append(QVariantMap &r,const QString &event,const QString &actor) {
 auto events=r.value("audit").toList();QVariantMap e{{"caseId",r.value("id")},{"sequence",events.size()+1},{"atUtc",now()},{"event",event},{"actor",actor},{"previous",events.isEmpty()?QString():events.last().toMap().value("hash").toString()},{"stateSha256",stateHash(r)}};
 e.insert("hash",digest(encode(e)));events.append(e);r.insert("audit",events);
}
bool LawCaseStore::save(QVariantMap r,qint64 expected) {
 r.insert("deviceCount",r.value("devices").toList().size());r.insert("revision",expected+1);
 // append is performed after final state fields have been set by the caller.
 const auto bytes=encode(r);if(bytes.size()>3*1024*1024)return fail("Case record exceeds bounded size.");
 const auto sealed=mr::recovery::sealCase(bytes,synthetic_,error_);if(sealed.isEmpty())return false;
 QSqlQuery q(db_);
 if(expected==0){q.prepare("INSERT INTO law_cases(id,revision,payload) VALUES(?,?,?)");q.addBindValue(r.value("id"));q.addBindValue(1);q.addBindValue(sealed);}
 else {q.prepare("UPDATE law_cases SET revision=?,payload=? WHERE id=? AND revision=?");q.addBindValue(expected+1);q.addBindValue(sealed);q.addBindValue(r.value("id"));q.addBindValue(expected);}
 if(!q.exec()||q.numRowsAffected()!=1)return fail("Case changed in another session or write failed. Reopen before editing.");
 return true;
}
QString LawCaseStore::create(const QVariantMap &metadata,const QString &actor,const QString &ownerId) {
 error_.clear();if(!ready_||!text(actor,256)||metadata.isEmpty())return {};
 const auto id=uuid();QVariantMap r{{"schema",1},{"synthetic",synthetic_},{"id",id},{"revision",1},{"metadata",metadata},{"status","draft"},{"deviceCount",0},{"devices",QVariantList()},{"evidence",QVariantList()},{"custody",QVariantList()},{"createdAtUtc",now()}};
 if(!ownerId.isEmpty())r.insert("ownerUserId",ownerId);
 append(r,"case-created",actor);return save(r,0)?id:QString();
}
bool LawCaseStore::mutate(const QString &id,qint64 revision,const QString &event,const QString &actor,const std::function<bool(QVariantMap&)> &change) {
 error_.clear();if(!ready_||!text(actor,256)||!db_.transaction())return fail("Cannot start case transaction.");
 auto r=get(id);if(r.isEmpty()||r.value("revision").toLongLong()!=revision||r.value("audit").toList().size()>=10000){db_.rollback();return fail("Invalid case, stale revision or event limit reached.");}
 const bool retention=event.startsWith("retention:");
 const bool derived=(event=="evidence-imported"||event=="evidence-import-rejected")&&(r.value("status")=="closed"||r.value("status")=="archived");
 if(!retention&&!derived&&((r.value("status")=="archived"&&event!="evidence-working-copy-exported"&&event!="evidence-analyzed"&&event!="case-backup-exported")||(r.value("status")=="closed"&&event!="case-status:archived"&&event!="evidence-working-copy-exported"&&event!="evidence-analyzed"&&event!="case-backup-exported")||(r.value("status")=="suspended"&&!event.startsWith("case-status:")&&event!="case-backup-exported"))){db_.rollback();return fail("Closed/archived cases are immutable in this prototype.");}
 if(!change(r)){db_.rollback();return false;}
 r.insert("revision",revision+1);r.insert("deviceCount",r.value("devices").toList().size());append(r,event,actor);
 if(!save(r,revision)||!db_.commit()){db_.rollback();if(error_.isEmpty())fail("Case transaction failed.");return false;}return true;
}
bool LawCaseStore::update(const QString &id,qint64 revision,const QVariantMap &m,const QString &actor){return mutate(id,revision,"case-updated",actor,[&](QVariantMap &r){r.insert("metadata",m);return true;});}
bool LawCaseStore::transition(const QString &id,qint64 revision,const QString &status,const QString &actor){if(!QStringList{"draft","suspended","closed","archived"}.contains(status))return fail("Unsupported status; legal approval is not automated.");return mutate(id,revision,"case-status:"+status,actor,[&](QVariantMap &r){r.insert("status",status);return true;});}
QString LawCaseStore::addDevice(const QString &id,qint64 revision,const QVariantMap &metadata,const QString &actor) {
 QString device;const QStringList keys{"manufacturer","model","serial","imei","condition","powerState","connectionState","lockState","receivedAtUtc","custodian","accessories","notes","observationSummary"};
 if(metadata.size()>keys.size())return {};
 for(auto it=metadata.cbegin();it!=metadata.cend();++it)if(!keys.contains(it.key())||it.value().metaType().id()!=QMetaType::QString||(!it.value().toString().isEmpty()&&!text(it.value().toString(),2048))){fail("Invalid device metadata.");return {};}
 if(!text(metadata.value("model").toString(),256)||!text(metadata.value("custodian").toString(),256)){fail("Device model and custodian are required.");return {};}
 const bool ok=mutate(id,revision,"device-registered",actor,[&](QVariantMap &r){auto rows=r.value("devices").toList();if(rows.size()>=100)return fail("Case device limit reached.");auto m=metadata;device=uuid();m.insert("id",device);m.insert("registeredAtUtc",now());rows.append(m);r.insert("devices",rows);return true;});return ok?device:QString();
}
QVariantList LawCaseStore::devices(const QString &id){return get(id).value("devices").toList();}
QVariantList LawCaseStore::audit(const QString &id){return get(id).value("audit").toList();}
bool LawCaseStore::verifyAudit(const QString &id){return !get(id).isEmpty();}
QString LawCaseStore::addEvidence(const QString &id,qint64 revision,const QString &device,const QString &path,const QString &description,const QString &custodian,const QString &actor) {
 if(!text(description,2048)||!text(custodian,256)){fail("Evidence description and custodian are required.");return {};}
 const auto local=mr::recovery::localFilePath(path);QFile input(local);
 if(local.isEmpty()||QFileInfo(local).isSymLink()||!mr::recovery::openRegularInput(input)||input.size()<=0||input.size()>16*1024*1024){fail("Choose a regular local file up to 16 MiB.");return {};}
 const auto bytes=input.read(16*1024*1024+1);if(bytes.size()!=input.size()||bytes.size()>16*1024*1024){fail("Evidence input changed or exceeds limit.");return {};}
 const auto sealed=mr::recovery::sealCase(bytes,synthetic_,error_);if(sealed.isEmpty())return {};
 QString evidence;
 const bool ok=mutate(id,revision,"evidence-registered",actor,[&](QVariantMap &r){
  bool found=false;for(const auto &d:r.value("devices").toList())if(d.toMap().value("id")==device)found=true;if(!found)return fail("Select a device from the active case.");
  auto rows=r.value("evidence").toList();if(rows.size()>=100)return fail("Evidence limit reached.");evidence=uuid();
  QSqlQuery q(db_);q.prepare("INSERT INTO law_evidence(id,case_id,payload) VALUES(?,?,?)");q.addBindValue(evidence);q.addBindValue(id);q.addBindValue(sealed);if(!q.exec())return fail("Evidence write failed.");
  rows.append(QVariantMap{{"id",evidence},{"deviceId",device},{"name",QFileInfo(local).fileName()},{"description",description},{"custodian",custodian},{"size",bytes.size()},{"sha256",digest(bytes)},{"registeredAtUtc",now()}});r.insert("evidence",rows);
  auto custody=r.value("custody").toList();custody.append(QVariantMap{{"evidenceId",evidence},{"from","source declaration"},{"to",custodian},{"reason","initial registration"},{"atUtc",now()},{"actor",actor}});r.insert("custody",custody);return true;
 });return ok?evidence:QString();
}
bool LawCaseStore::transfer(const QString &id,qint64 revision,const QString &evidence,const QString &from,const QString &to,const QString &reason,const QString &actor) {
 if(!text(from,256)||!text(to,256)||from==to||!text(reason,2048))return fail("Custody transfer needs distinct custodians and a reason.");
 return mutate(id,revision,"custody-transferred",actor,[&](QVariantMap &r){auto rows=r.value("evidence").toList();bool found=false;for(auto &v:rows){auto e=v.toMap();if(e.value("id")==evidence){if(e.value("custodian")!=from)return fail("Current custodian does not match.");e.insert("custodian",to);v=e;found=true;}}if(!found)return fail("Evidence does not belong to this case.");r.insert("evidence",rows);auto events=r.value("custody").toList();events.append(QVariantMap{{"evidenceId",evidence},{"from",from},{"to",to},{"reason",reason},{"atUtc",now()},{"actor",actor}});r.insert("custody",events);return true;});
}
QByteArray LawCaseStore::evidenceBytes(const QString &id,const QVariantMap &item) {
 QSqlQuery q(db_);q.prepare("SELECT payload FROM law_evidence WHERE id=? AND case_id=?");q.addBindValue(item.value("id"));q.addBindValue(id);
 if(!q.exec()||!q.next()){fail("Evidence payload missing.");return {};}
 const auto sealed=q.value(0).toByteArray();if(sealed.size()>18*1024*1024){fail("Evidence payload exceeds limit.");return {};}
 const auto bytes=mr::recovery::openCaseBytes(sealed,synthetic_,error_);if(bytes.isEmpty())return {};
 if(bytes.size()!=item.value("size").toLongLong()||digest(bytes)!=item.value("sha256").toString()){fail("Evidence SHA-256 mismatch.");return {};}
 return bytes;
}
QByteArray LawCaseStore::readEvidence(const QString &id,const QString &evidence) {
 const auto r=get(id);if(r.isEmpty())return {};
 for(const auto &e:r.value("evidence").toList())if(e.toMap().value("id")==evidence)return evidenceBytes(id,e.toMap());
 fail("Evidence does not belong to this case.");return {};
}
bool LawCaseStore::verifyEvidence(const QString &id,const QString &evidence) {return !readEvidence(id,evidence).isEmpty();}
QVariantMap LawCaseStore::recordAnalysis(const QString &id,qint64 revision,const QString &evidence,const QString &parser,const QVariantMap &report,const QString &actor) {
 QVariantMap result;const auto json=encode(report);
 if(json.size()>64*1024||report.value("sourceSha256").toString().size()!=64){fail("Invalid or oversized analysis report.");return {};}
 const bool imported=parser.startsWith("authorized-import/");
 const auto event=imported?(report.value("status")=="imported"?QString("evidence-imported"):QString("evidence-import-rejected")):QString("evidence-analyzed");
 const bool ok=mutate(id,revision,event,actor,[&](QVariantMap &r){
  QVariantMap item;for(const auto &value:r.value("evidence").toList())if(value.toMap().value("id")==evidence)item=value.toMap();
  if(item.isEmpty()||item.value("sha256")!=report.value("sourceSha256"))return fail("Analysis source does not match this case evidence.");
  auto runs=r.value("analyses").toList();if(runs.size()>=100)return fail("Case analysis limit reached.");
  result={{"id",uuid()},{"evidenceId",evidence},{"parser",parser},{"implementationVersion","nexvary-mr/0.4.0"},
   {"sourceSha256",item.value("sha256")},{"sourceSize",item.value("size")},{"reportSha256",digest(json)},{"reportJson",QString::fromUtf8(json)},
   {"atUtc",now()},{"actor",actor},{"synthetic",synthetic_}};
  runs.append(result);r.insert("analyses",runs);return true;
 });return ok?result:QVariantMap();
}
QVariantMap LawCaseStore::exportWorkingCopy(const QString &id,qint64 revision,const QString &evidence,const QString &directory,const QString &purpose,const QString &actor) {
 error_.clear();if(!text(purpose,2048)){fail("A bounded working-copy purpose is required.");return {};}
 const auto path=mr::recovery::localFilePath(directory);
 if(path.isEmpty()||QFileInfo(path).exists()||QFileInfo(path).isSymLink()||!QFileInfo(QFileInfo(path).absolutePath()).isDir()){fail("Choose a new package directory inside an existing local folder.");return {};}
 bool created=false;QVariantMap result;
 const bool ok=mutate(id,revision,"evidence-working-copy-exported",actor,[&](QVariantMap &r){
  QVariantMap item;for(const auto &e:r.value("evidence").toList())if(e.toMap().value("id")==evidence)item=e.toMap();
  if(item.isEmpty())return fail("Evidence does not belong to this case.");
  auto copies=r.value("workingCopies").toList();if(copies.size()>=100)return fail("Working-copy export limit reached.");
  const auto bytes=evidenceBytes(id,item);if(bytes.isEmpty())return false;
  const auto packageId=uuid();
  const QVariantMap manifest{{"schema","nexvary-evidence-package/v1"},{"packageId",packageId},{"caseId",id},{"caseRevision",revision},
   {"auditHead",r.value("audit").toList().last().toMap().value("hash")},{"synthetic",synthetic_},{"exportedAtUtc",now()},{"actor",actor},{"purpose",purpose},
   {"evidenceId",evidence},{"deviceId",item.value("deviceId")},{"sourceName",item.value("name")},
   {"files",QVariantList{QVariantMap{{"path","evidence.bin"},{"size",bytes.size()},{"sha256",digest(bytes)}}}},
   {"trustBoundary","Hash-verified working copy. Retain the manifest SHA-256 separately. No external timestamp, legal authority or original-device authenticity is established."}};
  const auto manifestBytes=encode(manifest);const auto manifestHash=digest(manifestBytes);
  if(!QDir().mkdir(path))return fail("Package directory already exists or cannot be created.");
  created=true;
  if(!QFile::setPermissions(path,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner))return fail("Cannot restrict package directory permissions.");
  const auto write=[&](const QString &name,const QByteArray &data){QFile f(QDir(path).filePath(name));if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly))return false;if(!f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||f.write(data)!=data.size()||!f.flush())return false;return true;};
  if(!write("evidence.bin",bytes)||!write("manifest.json",manifestBytes))return fail("Working-copy export failed.");
  // The package anchors the pre-export revision. The following audit event commits
  // its digest and export purpose atomically; no external path enters case metadata.
  result={{"packageId",packageId},{"evidenceId",evidence},{"manifestSha256",manifestHash},{"sourceRevision",revision},{"purpose",purpose},{"exportedAtUtc",manifest.value("exportedAtUtc")},{"actor",actor}};
  copies.append(result);r.insert("workingCopies",copies);return true;
 });
 if(!ok){if(created){QFile::remove(QDir(path).filePath("manifest.json"));QFile::remove(QDir(path).filePath("evidence.bin"));QDir().rmdir(path);}return {};}
 return result;
}
