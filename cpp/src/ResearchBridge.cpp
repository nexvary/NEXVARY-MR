#include "ResearchBridge.hpp"
#include "IdentityService.hpp"
#include "ToolBridge.hpp"
#include "FirmwareBridge.hpp"
#include "RecoveryFile.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QSaveFile>
#include <QLockFile>
#include <QDir>
#include <QDateTime>
#include <QUrl>
#include <QUuid>
#include <QSet>
namespace {
QByteArray json(const QVariantMap &m){return QJsonDocument(QJsonObject::fromVariantMap(m)).toJson(QJsonDocument::Compact);}
QString hash(const QByteArray &bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
QVariantMap resultPayload(QVariantMap row){row.remove("authorAttestation");row.remove("reviewAttestation");row.remove("reviewStatus");return row;}
bool bounded(const QVariantMap &m){if(json(m).size()>16384)return false;for(auto it=m.cbegin();it!=m.cend();++it){if(it.value().metaType().id()!=QMetaType::QString||it.value().toString().trimmed().isEmpty()||it.value().toString().size()>4096)return false;for(const auto c:it.value().toString())if((c.unicode()<32&&c!='\n'&&c!='\t')||(c.unicode()>=0x202a&&c.unicode()<=0x202e)||(c.unicode()>=0x2066&&c.unicode()<=0x2069))return false;}return true;}
}
ResearchBridge::ResearchBridge(const QString &path,ToolBridge *tools,FirmwareBridge *firmware,QObject *parent,IdentityService *identity):QObject(parent),path_(path),tools_(tools),firmware_(firmware),identity_(identity) {
 Q_INIT_RESOURCE(research_catalog);
 QFile file(":/research/program/catalog.json");if(!file.open(QIODevice::ReadOnly)||file.size()>1024*1024){fail("Research catalog unavailable or oversized.");return;}
 QJsonParseError parse;
 const auto doc=QJsonDocument::fromJson(file.readAll(),&parse);if(parse.error!=QJsonParseError::NoError||!doc.isObject()){fail("Invalid research catalog JSON.");return;}
 catalog_=doc.object().toVariantMap();error_=validateCatalog(catalog_);if(!error_.isEmpty())return;
 QFile shared(":/research/advanced-mobile/knowledge.json");
 if(shared.open(QIODevice::ReadOnly)&&shared.size()<1024*1024){const auto knowledge=QJsonDocument::fromJson(shared.readAll()).object();catalog_.insert("nativeCapabilities",knowledge.value("capabilities").toVariant());}
 ready_=load();
 if(identity_)connect(identity_,&IdentityService::changed,this,&ResearchBridge::changed);
 if(tools_)connect(tools_,&ToolBridge::deviceIntelligenceChanged,this,&ResearchBridge::contextChanged);
 if(firmware_){connect(firmware_,&FirmwareBridge::avbReportChanged,this,&ResearchBridge::contextChanged);connect(firmware_,&FirmwareBridge::packageReportChanged,this,&ResearchBridge::contextChanged);}
}
bool ResearchBridge::fail(const QString &e){error_=e;emit changed();return false;}
QString ResearchBridge::validateCatalog(const QVariantMap &c) {
 if(c.value("schemaVersion").toInt()!=1)return "Unknown research schema.";
 QSet<QString> sources;
 for(const auto &v:c.value("sources").toList()){const auto s=v.toMap();
 const auto id=s.value("id").toString();
 const QUrl url(s.value("url").toString());if(id.isEmpty()||sources.contains(id)||url.scheme()!="https"||url.host().isEmpty()||!url.userInfo().isEmpty()||s.value("publisher").toString().isEmpty()||s.value("rights").toString().isEmpty()||s.value("reviewedOn").toString().isEmpty())return "Invalid/duplicate source or missing rights/review date.";sources.insert(id);}
 QSet<QString> ids;
 for(const auto *kind:{"atlas","tools","experiments"}){
  const auto rows=c.value(kind).toList();if(rows.isEmpty()||rows.size()>500)return "Missing or oversized research category.";
  for(const auto &v:rows){const auto row=v.toMap();
 const auto id=row.value("id").toString();if(id.isEmpty()||ids.contains(id)||row.value("title").toString().isEmpty()||row.value("limitations").toString().isEmpty())return "Invalid or duplicate research entry.";ids.insert(id);
   if(!QStringList{"Publicly Documented","Vendor Claimed","Research Hypothesis","Historical","Unsupported"}.contains(row.value("status").toString()))return "Catalog cannot assert Lab Verified capability.";
   const auto refs=row.value("sourceIds").toStringList();if(refs.isEmpty())return "Research entry requires provenance.";for(const auto &ref:refs)if(!sources.contains(ref))return "Unknown research source reference.";
  }
 }
 return {};
}
bool ResearchBridge::load() {
 state_={{"schemaVersion",1},{"revision",0},{"results",QVariantList()},{"compatibility",QVariantList()}};
 if(!QFile::exists(path_))return true;
 QFile file(path_);if(QFileInfo(path_).isSymLink()||!mr::recovery::openRegularInput(file)||file.size()>4*1024*1024)return fail("Research ledger path or size invalid.");
 QJsonParseError parse;
 auto doc=QJsonDocument::fromJson(file.readAll(),&parse);
 auto envelope=doc.object().toVariantMap();
 const auto digest=envelope.take("sha256").toString();if(parse.error!=QJsonParseError::NoError||!doc.isObject()||envelope.value("schemaVersion").toInt()!=1||digest!=hash(json(envelope)))return fail("Research ledger integrity failed; no data overwritten.");
 for(const auto *category:{"results","compatibility"}) {
  if(envelope.value(category).metaType().id()!=QMetaType::QVariantList||envelope.value(category).toList().size()>500)return fail("Invalid research ledger rows.");
  for(const auto &value:envelope.value(category).toList()) {
   const auto row=value.toMap();
   if(row.isEmpty()||row.value("commercialSupported").metaType().id()!=QMetaType::Bool||row.value("commercialSupported").toBool())return fail("Untrusted research capability assertion rejected.");
   const auto status=row.value("status").toString();
   if(QString::fromLatin1(category)=="results") {
    if(!QStringList{"Successful","Not Reproduced","Research Hypothesis"}.contains(status)||(row.value("reviewStatus")!="Pending independent review"&&!reviewed(row)))return fail("Untrusted or unsigned result review rejected.");
   } else if(!QStringList{"Vendor Claimed","Publicly Documented","Research Hypothesis","Unsupported"}.contains(status))return fail("Unreviewed compatibility promotion rejected.");
  }
 }
 state_=envelope;
 return true;
}
bool ResearchBridge::save(const QVariantMap &next) {
 if(!QDir().mkpath(QFileInfo(path_).absolutePath()))return fail("Cannot create local research directory.");
 QLockFile lock(path_+".lock");if(!lock.tryLock(0))return fail("Research ledger is in use; retry after reopening.");
 const auto expected=state_.value("revision").toLongLong();if(!load()||state_.value("revision").toLongLong()!=expected)return fail("Research ledger changed in another session; retry with current data.");
 auto envelope=next;envelope.insert("revision",expected+1);envelope.insert("sha256",hash(json(envelope)));
 const auto bytes=json(envelope);if(bytes.size()>4*1024*1024)return fail("Research ledger size limit reached.");
 QSaveFile file(path_);file.setDirectWriteFallback(false);if(!file.open(QIODevice::WriteOnly)||!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(bytes)!=bytes.size()||!file.commit())return fail("Research ledger could not be saved atomically.");state_=envelope;state_.remove("sha256");error_.clear();emit changed();
 return true;
}
QVariantList ResearchBridge::search(const QString &category,const QString &query,const QString &status)const {
 QVariantList rows;if(!ready_||query.size()>256)return rows;
 const auto data=category=="results"?results():category=="compatibility"?compatibility():catalog_.value(category).toList();
 for(const auto &v:data){auto r=v.toMap();if(!status.isEmpty()&&r.value("status").toString()!=status)continue;if(query.isEmpty()||QString::fromUtf8(json(r)).contains(query,Qt::CaseInsensitive))rows.append(r);}return rows;
}
QVariantMap ResearchBridge::engineContext()const {
 const auto device=tools_?tools_->deviceIntelligence():QVariantMap();
 return {{"deviceIntelligence",device},{"usbDiagnosis",tools_?tools_->connectionDiagnosis():QVariantMap()},{"firmwareAvb",firmware_?firmware_->avbReport():QVariantMap()},{"firmwarePackage",firmware_?firmware_->packageReport():QVariantMap()},{"executionApproved",false},{"modelAcceptance","not-established"},{"capabilityGate","Research records never override native service policy; synthetic tests do not certify physical devices."}};
}
QString ResearchBridge::recordResult(const QVariantMap &fields,const QString &evidencePath,bool consent) {
 if(!ready_||!canWrite()||!consent||!bounded(fields)){fail("Only consented public/synthetic research observations with bounded fields can be recorded.");return {};}
 const QStringList required{"experimentId","question","hypothesis","model","osBuild","firmware","environment","equipment","scope","methodology","expected","actual","reproducibility","researcher"};for(const auto &key:required)if(!fields.contains(key)){fail("Result is missing a required experiment field.");return {};}
 for(auto it=fields.cbegin();it!=fields.cend();++it)if(!required.contains(it.key())){fail("Unsupported experiment field.");return {};}
 bool found=false;for(const auto &e:catalog_.value("experiments").toList())if(e.toMap().value("id")==fields.value("experimentId"))found=true;
 if(!found||!QStringList{"Successful","Not Reproduced","Research Hypothesis"}.contains(fields.value("reproducibility").toString())){fail("Unknown experiment or reproducibility status.");return {};}
 const auto path=mr::recovery::localFilePath(evidencePath);
 QFile file(path);if(path.isEmpty()||QFileInfo(path).isSymLink()||!mr::recovery::openRegularInput(file)||file.size()<=0||file.size()>16*1024*1024){fail("Choose a public synthetic evidence file up to 16 MiB.");return {};}
 const auto bytes=file.read(16*1024*1024+1);if(bytes.size()!=file.size()||bytes.size()>16*1024*1024){fail("Evidence changed or exceeds limit.");return {};}
 auto row=fields;
 const auto id=QUuid::createUuid().toString(QUuid::WithoutBraces);row.insert("id",id);row.insert("recordedAtUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));row.insert("evidenceSha256",hash(bytes));row.insert("evidenceSize",bytes.size());row.insert("evidenceName",QFileInfo(path).fileName());row.insert("reviewStatus","Pending independent review");row.insert("status",fields.value("reproducibility"));row.insert("commercialSupported",false);
 if(identity_){row.insert("authorUserId",identity_->userId());row.insert("verificationScope","public-or-synthetic; no physical-device acceptance");
 const auto signature=identity_->attest("research-result",resultPayload(row));if(signature.isEmpty()){fail("Authenticated result signing failed.");return {};}row.insert("authorAttestation",signature);}
 auto next=state_;
 auto rows=results();if(rows.size()>=500){fail("Research result limit reached.");return {};}rows.append(row);next.insert("results",rows);return save(next)?id:QString();
}
bool ResearchBridge::canWrite(){
 if(!identity_)return true; // Native synthetic-fixture composition only; main always supplies identity.
 if(!identity_->checkSession()||!(identity_->lawRole()==nexvary::law::Role::Investigator||identity_->lawRole()==nexvary::law::Role::Reviewer))return fail("Sign in as a local investigator or reviewer to record research.");
 return true;
}
bool ResearchBridge::reviewed(const QVariantMap &row)const{
 if(!identity_||!QStringList{"Approved","Rejected"}.contains(row.value("reviewStatus").toString()))return false;
 const auto core=resultPayload(row),author=row.value("authorAttestation").toMap(),review=row.value("reviewAttestation").toMap(),payload=review.value("payload").toMap();
 return identity_->verify(author,"research-result",core)&&author.value("signerId")==row.value("authorUserId")&&review.value("signerId")!=row.value("authorUserId")&&review.value("role")=="reviewer"&&payload.value("resultId")==row.value("id")&&payload.value("resultSha256")==hash(json(core))&&payload.value("artifactSha256")==row.value("evidenceSha256")&&payload.value("decision")==row.value("reviewStatus")&&identity_->verify(review,"research-review",payload);
}
bool ResearchBridge::reviewResult(const QString &id,const QString &evidencePath,const QString &decision,const QString &notes){
 if(!ready_||!identity_||!identity_->checkSession()||identity_->lawRole()!=nexvary::law::Role::Reviewer)return fail("A separately authenticated local reviewer is required.");
 if(!QStringList{"Approved","Rejected"}.contains(decision)||!bounded({{"notes",notes}}))return fail("Choose an approved/rejected decision and bounded review notes.");
 auto rows=results();qsizetype index=-1;for(qsizetype i=0;i<rows.size();++i)if(rows[i].toMap().value("id")==id)index=i;if(index<0)return fail("Unknown research result.");
 auto row=rows[index].toMap();
 const auto core=resultPayload(row);if(row.value("reviewStatus")!="Pending independent review"||row.value("authorUserId").toString().isEmpty()||row.value("authorUserId")==identity_->userId()||!identity_->verify(row.value("authorAttestation").toMap(),"research-result",core))return fail("Result must have a trusted author signature from a different account and no previous review.");
 const auto path=mr::recovery::localFilePath(evidencePath);
 QFile file(path);if(path.isEmpty()||QFileInfo(path).isSymLink()||!mr::recovery::openRegularInput(file)||file.size()<=0||file.size()>16*1024*1024)return fail("Choose the original public/synthetic evidence file (max 16 MiB).");
 const auto bytes=file.read(16*1024*1024+1);if(bytes.size()!=row.value("evidenceSize").toLongLong()||hash(bytes)!=row.value("evidenceSha256"))return fail("Review artifact differs from the recorded evidence.");
 const auto signature=identity_->attest("research-review",{{"resultId",id},{"resultSha256",hash(json(core))},{"artifactSha256",row.value("evidenceSha256")},{"decision",decision},{"notes",notes}});if(signature.isEmpty())return fail("Review signing failed.");row.insert("reviewStatus",decision);row.insert("reviewAttestation",signature);if(!reviewed(row))return fail("Independent review verification failed.");rows[index]=row;
 auto next=state_;next.insert("results",rows);return save(next);
}
QVariantMap ResearchBridge::validation(const QString &id)const {
 for(const auto &v:results()){const auto r=v.toMap();if(r.value("id")==id)return {{"status",r.value("status")},{"evidenceSha256",r.value("evidenceSha256")},{"review",r.value("reviewStatus")},{"signatureTrusted",reviewed(r)},{"verificationScope",r.value("verificationScope","legacy-public-or-synthetic")},{"commercialSupported",false},{"gate","Independent evidence review, exact-device repeatability, implementation tests and native service policy acceptance remain required."}};}return {{"status","Unknown result"},{"commercialSupported",false}};
}
bool ResearchBridge::addCompatibility(const QVariantMap &fields) {
 if(!ready_||!canWrite()||!bounded(fields))return fail("Invalid compatibility record.");
 const QStringList keys{"manufacturer","model","chipset","osBuild","firmware","securityPatch","connectionMode","capability","unsupported","sourceId","lastTestUtc","result","confidence","status"};for(const auto &key:keys)if(!fields.contains(key))return fail("Compatibility record is incomplete.");for(auto it=fields.cbegin();it!=fields.cend();++it)if(!keys.contains(it.key()))return fail("Unsupported compatibility field.");
 const auto status=fields.value("status").toString();if(!QStringList{"Vendor Claimed","Publicly Documented","Research Hypothesis","Unsupported"}.contains(status))return fail("Lab Verified requires an independently reviewed experiment; direct promotion denied.");
 bool source=false;for(const auto &v:catalog_.value("sources").toList())if(v.toMap().value("id")==fields.value("sourceId"))source=true;if(!source)return fail("Unknown compatibility source.");
 auto next=state_;
 auto rows=compatibility();if(rows.size()>=500)return fail("Compatibility row limit reached.");
 auto r=fields;r.insert("id",QUuid::createUuid().toString(QUuid::WithoutBraces));r.insert("commercialSupported",false);rows.append(r);next.insert("compatibility",rows);return save(next);
}
bool ResearchBridge::exportReport(const QString &path,bool consent) {
 if(!ready_||!consent)return fail("Explicit public-research export consent is required.");
 const auto local=mr::recovery::localFilePath(path);
 QFile file(local);if(local.isEmpty()||!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail("Choose a new local report filename.");
 const auto bytes=json({{"schemaVersion",1},{"catalog",catalog_},{"ledger",state_},{"generatedAtUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},{"notice","Research only; source claims and synthetic results do not grant commercial support."}});if(!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(bytes)!=bytes.size()||!file.flush()){file.close();file.remove();return fail("Research report failed.");}error_.clear();emit changed();
 return true;
}
