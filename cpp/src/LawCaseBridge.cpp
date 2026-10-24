#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "LawTraining.hpp"
#include "ToolBridge.hpp"
#include "CaseVault.hpp"
#include "RecoveryFile.hpp"
#include "nexvary/LawEnforcementCase.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QPdfWriter>
#include <QTextDocument>
#include <QDateTime>
#include <utility>
namespace {
QString utc(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
std::string bytes(const QVariantMap &m,const char *key){return m.value(key).toString().toUtf8().toStdString();}
bool newOutput(const QString &input,QFile &file) {
 const auto path=mr::recovery::localFilePath(input);if(path.isEmpty())return false;
 file.setFileName(path);if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))return false;
 if(file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner))return true;
 file.close();file.remove();return false;
}
}
LawCaseBridge::LawCaseBridge(const QString &path,ToolBridge *tools,bool syntheticTestProfile,nexvary::law::Role role,QObject *parent)
 :QObject(parent),store_(std::make_unique<LawCaseStore>(path,syntheticTestProfile)),tools_(tools),synthetic_(syntheticTestProfile),role_(role) {
 if(tools_)connect(tools_,&ToolBridge::deviceIntelligenceChanged,this,&LawCaseBridge::accessChanged);
 if(!store_->ready())error_=store_->error();
 else if(nexvary::law::allows(role_,nexvary::law::Action::ReadCase))refresh();
}
LawCaseBridge::~LawCaseBridge()=default;
bool LawCaseBridge::ready()const{return store_->ready();}
QString LawCaseBridge::protection()const {
 return mr::recovery::vaultAvailable()?QStringLiteral("Windows current-account DPAPI; SQLite payloads protected"):
  synthetic_?QStringLiteral("SYNTHETIC TEST DATA ONLY — NOT ENCRYPTED"):QStringLiteral("Windows protected storage required; real cases disabled");
}
QString LawCaseBridge::role()const{return QString::fromUtf8(nexvary::law::roleId(role_).data(),static_cast<qsizetype>(nexvary::law::roleId(role_).size()));}
QVariantMap LawCaseBridge::permissions()const {
 using A=nexvary::law::Action;QVariantMap p;
 for(const auto &entry:std::initializer_list<std::pair<const char *,A>>{{"read",A::ReadCase},{"create",A::CreateCase},{"edit",A::EditCase},{"archive",A::ArchiveCase},{"close",A::CloseCase},{"addDevice",A::AddDevice},{"report",A::ExportReport},{"audit",A::ReadAudit},{"training",A::AccessTraining},{"addEvidence",A::AddEvidence},{"custody",A::TransferCustody}})
  p.insert(entry.first,nexvary::law::allows(role_,entry.second));
 p.insert("executeDevice",false);p.insert("enterpriseRoleProvisioningReady",false);return p;
}
QString LawCaseBridge::actor()const{return synthetic_?QStringLiteral("SYNTHETIC operator"):QStringLiteral("Current OS account — local operator declaration");}
bool LawCaseBridge::fail(const QString &message){error_=message;emit changed();return false;}
bool LawCaseBridge::permit(nexvary::law::Action action) {
 if(!nexvary::law::allows(role_,action))return fail(QStringLiteral("This session role does not permit this action."));
 if(!ready())return fail(QStringLiteral("Protected case storage is unavailable on this host."));
 return true;
}
bool LawCaseBridge::validateMetadata(const QVariantMap &m) {
 if(m.size()>24)return fail(QStringLiteral("Unsupported case fields."));
 const QStringList keys{"caseNumber","investigatingAgency","investigatorName","seizedDeviceDescription","authorityReference","issuingAuthority","scope","prosecutorName","prosecutorOffice","courtName","validFromUtc","validUntilUtc","incidentDescription","seizureAtUtc","seizurePlace","notes","jurisdiction"};
 for(auto it=m.cbegin();it!=m.cend();++it)if(!keys.contains(it.key())||it.value().metaType().id()!=QMetaType::QString||it.value().toString().toUtf8().size()>8192)return fail(QStringLiteral("Invalid or oversized case field."));
 for(auto it=m.cbegin();it!=m.cend();++it) {
  const auto value=it.value().toString();
  if(!value.isEmpty()&&!nexvary::law_enforcement::hasText(value.toUtf8().toStdString()))return fail(QStringLiteral("Case fields contain prohibited controls or invisible-only text."));
 }
 nexvary::law_enforcement::CaseRecord r;r.caseId="pending-case";r.caseNumber=bytes(m,"caseNumber");r.investigatingAgency=bytes(m,"investigatingAgency");r.investigatorName=bytes(m,"investigatorName");r.seizedDeviceDescription=bytes(m,"seizedDeviceDescription");
 r.authorization.authorityReference=bytes(m,"authorityReference");r.authorization.issuingAuthority=bytes(m,"issuingAuthority");r.authorization.scope=bytes(m,"scope");r.authorization.prosecutorName=bytes(m,"prosecutorName");r.authorization.courtName=bytes(m,"courtName");r.authorization.validFromUtc=bytes(m,"validFromUtc");r.authorization.validUntilUtc=bytes(m,"validUntilUtc");
 if(!nexvary::law_enforcement::validateCaseForReview(r).valid)return fail(QStringLiteral("Check required case/authority fields, UTF-8 limits and UTC date order. Completeness is not legal approval."));
 if(!m.value("seizureAtUtc").toString().isEmpty()&&!nexvary::law_enforcement::isValidUtcTimestamp(bytes(m,"seizureAtUtc")))return fail(QStringLiteral("Seizure time must be a real UTC timestamp ending in Z."));
 return true;
}
bool LawCaseBridge::refresh(const QString &id) {
 if(!id.isEmpty()) {
  auto found=store_->get(id);if(found.isEmpty())return fail(store_->error());active_=found;
 }
 if(nexvary::law::allows(role_,nexvary::law::Action::ReadCase))cases_=store_->list();else cases_.clear();
 if(!store_->error().isEmpty())return fail(store_->error());
 if(!active_.isEmpty()) {
  const auto currentId=active_.value("id").toString();devices_=nexvary::law::allows(role_,nexvary::law::Action::ReadDevice)?store_->devices(currentId):QVariantList();
  audit_=nexvary::law::allows(role_,nexvary::law::Action::ReadAudit)?store_->audit(currentId):QVariantList();
  if(!store_->error().isEmpty())return fail(store_->error());
 }
 error_.clear();emit changed();return true;
}
QString LawCaseBridge::createCase(const QVariantMap &metadata) {
 if(!permit(nexvary::law::Action::CreateCase)||!validateMetadata(metadata))return {};
 const auto id=store_->create(metadata,actor());if(id.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?id:QString();
}
bool LawCaseBridge::updateCase(const QVariantMap &metadata) {
 if(!permit(nexvary::law::Action::EditCase)||active_.isEmpty()||!validateMetadata(metadata))return false;
 const auto id=active_.value("id").toString();
 if(!store_->update(id,active_.value("revision").toLongLong(),metadata,actor()))return fail(store_->error());
 return refresh(id);
}
bool LawCaseBridge::openCase(const QString &id) {return permit(nexvary::law::Action::ReadCase)&&refresh(id);}
void LawCaseBridge::closeWorkspace(){active_.clear();devices_.clear();audit_.clear();error_.clear();emit changed();}
bool LawCaseBridge::searchCases(const QString &query) {
 if(!permit(nexvary::law::Action::ReadCase))return false;
 if(query.size()>256)return fail(QStringLiteral("Search query exceeds limit."));
 cases_=store_->list(query);if(!store_->error().isEmpty())return fail(store_->error());
 error_.clear();emit changed();return true;
}
bool LawCaseBridge::transitionCase(const QString &status) {
 if(active_.isEmpty())return fail(QStringLiteral("Open a case first."));
 const auto action=status=="archived"?nexvary::law::Action::ArchiveCase:status=="closed"?nexvary::law::Action::CloseCase:nexvary::law::Action::EditCase;
 if(status=="authorized"||!QStringList{"draft","suspended","closed","archived"}.contains(status))return fail(QStringLiteral("This action cannot approve legal authority."));
 if(!permit(action))return false;
 const auto id=active_.value("id").toString();
 if(!store_->transition(id,active_.value("revision").toLongLong(),status,actor()))return fail(store_->error());
 return refresh(id);
}
QString LawCaseBridge::addDevice(const QVariantMap &metadata) {
 if(!permit(nexvary::law::Action::AddDevice)||active_.isEmpty())return {};
 if(!metadata.value("receivedAtUtc").toString().isEmpty()&&!nexvary::law_enforcement::isValidUtcTimestamp(bytes(metadata,"receivedAtUtc"))){fail(QStringLiteral("Device receipt time must be a real UTC timestamp ending in Z."));return {};}
 const auto id=active_.value("id").toString();const auto device=store_->addDevice(id,active_.value("revision").toLongLong(),metadata,actor());
 if(device.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?device:QString();
}
QVariantMap LawCaseBridge::currentDeviceObservation()const {
 if(!tools_||!nexvary::law::allows(role_,nexvary::law::Action::AddDevice))return {};
 const auto report=tools_->deviceIntelligence();QVariantMap result;
 for(const auto &entry:report.value("fields").toList()) {
  const auto f=entry.toMap();if(f.value("state").toString()!="reported")continue;
  if(f.value("id").toString()=="manufacturer"||f.value("id").toString()=="model")result.insert(f.value("id").toString(),f.value("value"));
 }
 result.insert("serial",tools_->selectedSerial());result.insert("observationSummary",QStringLiteral("Device-reported fields; no model acceptance or cryptographic attestation. USB scope: ")+report.value("usbScope").toString());
 result.insert("receivedAtUtc",utc());return result;
}
QVariantList LawCaseBridge::assessDeviceAccess()const {
 QVariantList rows;
 if(!nexvary::law::allows(role_,nexvary::law::Action::ReadDevice))return rows;
 const auto report=tools_?tools_->deviceIntelligence():QVariantMap();
 auto add=[&](const QString &id,const QString &ar,const QString &en,const QString &state,const QString &fact){rows.append(QVariantMap{{"id",id},{"ar",ar},{"en",en},{"state",state},{"fact",fact},{"confidence","device-reported-or-unknown"},{"executionApproved",false}});};
 add("frp","حالة FRP","FRP state","Research Required","Not established by the allowlisted property read; no account bypass offered.");
 add("screen-lock","قفل الشاشة وPIN","Screen lock and PIN","Unsupported","No PIN removal, password recovery or protected-device extraction operation is implemented.");
 add("fde","تشفير القرص الكامل FDE","Full-disk encryption","Research Required","A reported encryption type does not establish key availability or a supported decryption path.");
 add("fbe","تشفير الملفات FBE","File-based encryption","Research Required","A reported file encryption property does not establish access to credential-encrypted data.");
 add("hardware-security","الحماية المدعومة بالعتاد","Hardware-backed security","Research Required","No TEE/KeyMint attestation or hardware key extraction is performed.");
 add("metadata","قراءة الخصائص المفوضة مسبقًا","Already-authorized metadata reading",report.value("propertiesCurrent").toBool()?"Supported":"Research Required","Read-only device-reported properties only; not application-content acquisition. Review target and authority scope before any separate collection.");
 add("bootloader","تفسير تصريحات Bootloader","Interpret bootloader declarations",report.value("propertiesCurrent").toBool()?"Supported":"Research Required","Reported lock/boot properties are not cryptographic attestation, an unlock operation or flashing approval.");
 add("acquisition","واجهات الاستخلاص المدعومة","Supported acquisition interfaces","Research Required","Only explicitly reviewed existing local-file and metadata paths are offered. No full physical acquisition of a protected phone is inferred.");
 return rows;
}
QString LawCaseBridge::addEvidence(const QString &device,const QString &path,const QString &description,const QString &custodian,bool consent) {
 if(!consent||!permit(nexvary::law::Action::AddEvidence)||active_.isEmpty())return {};
 const auto id=active_.value("id").toString();const auto evidence=store_->addEvidence(id,active_.value("revision").toLongLong(),device,path,description,custodian,actor());
 if(evidence.isEmpty()){fail(store_->error());return {};}return refresh(id)?evidence:QString();
}
bool LawCaseBridge::transferCustody(const QString &evidence,const QString &from,const QString &to,const QString &reason) {
 if(!permit(nexvary::law::Action::TransferCustody)||active_.isEmpty())return false;
 const auto id=active_.value("id").toString();if(!store_->transfer(id,active_.value("revision").toLongLong(),evidence,from,to,reason,actor()))return fail(store_->error());return refresh(id);
}
bool LawCaseBridge::verifyEvidence(const QString &evidence) {
 if(!permit(nexvary::law::Action::ReadDevice)||active_.isEmpty())return false;
 if(!store_->verifyEvidence(active_.value("id").toString(),evidence))return fail(store_->error());
 error_.clear();emit changed();return true;
}
bool LawCaseBridge::verifyAudit() {
 if(!permit(nexvary::law::Action::ReadAudit)||active_.isEmpty())return false;
 if(!store_->verifyAudit(active_.value("id").toString()))return fail(store_->error());
 error_.clear();emit changed();return true;
}
QVariantList LawCaseBridge::scenarios()const{return nexvary::law::allows(role_,nexvary::law::Action::AccessTraining)?nexvary::law::trainingScenarios():QVariantList();}
QVariantMap LawCaseBridge::evaluateTraining(const QString &scenarioId,const QString &choiceId) {
 if(!nexvary::law::allows(role_,nexvary::law::Action::AccessTraining))return {{"valid",false}};
 auto result=nexvary::law::evaluateTraining(scenarioId,choiceId);if(!result.value("valid").toBool())return result;
 result.insert("scenarioId",scenarioId);result.insert("choiceId",choiceId);result.insert("atUtc",utc());result.insert("synthetic",true);
 attempts_.append(result);if(attempts_.size()>200)attempts_.removeFirst();emit trainingChanged();return result;
}
bool LawCaseBridge::exportTrainingReport(const QString &path,bool consent) {
 if(!consent||!nexvary::law::allows(role_,nexvary::law::Action::AccessTraining))return fail(QStringLiteral("Explicit synthetic-report export approval is required."));
 QFile output;if(!newOutput(path,output))return fail(QStringLiteral("Choose a new local report file."));
 const auto bytes=QJsonDocument(QJsonObject{{"schemaVersion",1},{"synthetic",true},{"generatedAtUtc",utc()},{"attempts",QJsonArray::fromVariantList(attempts_)},{"notice","Training decisions only; no real case or device operation."}}).toJson();
 if(output.write(bytes)!=bytes.size()||!output.flush()){output.close();output.remove();return fail(QStringLiteral("Training report could not be saved."));}return true;
}
bool LawCaseBridge::exportCasePdf(const QString &path,bool arabic,bool consent) {
 if(!consent||!permit(nexvary::law::Action::ExportReport)||active_.isEmpty()||!store_->verifyAudit(active_.value("id").toString()))return fail(QStringLiteral("A readable case, verified local audit and explicit plaintext-export consent are required."));
 if(!refresh(active_.value("id").toString()))return false;
 for(const auto &entry:active_.value("evidence").toList())if(!store_->verifyEvidence(active_.value("id").toString(),entry.toMap().value("id").toString()))return fail(store_->error());
 QFile output;if(!newOutput(path,output))return fail(QStringLiteral("Choose a new local PDF file."));
 QString html=QStringLiteral("<html dir='%1'><head><meta charset='UTF-8'></head><body><h1>NEXVARY — Law Enforcement</h1>").arg(arabic?"rtl":"ltr");
 html+=QStringLiteral("<p>%1</p><p>%2</p>").arg(active_.value("id").toString().toHtmlEscaped(),utc().toHtmlEscaped());
 html+=QStringLiteral("<p>Local prototype: recorded declarations and observations. Not automatic judicial accreditation, legal approval or extraction certification. Enterprise role provisioning remains pending.</p><h2>Case / Authority</h2><table border='1' cellspacing='0' cellpadding='5' width='100%'>");
 const auto metadata=active_.value("metadata").toMap();
 for(auto it=metadata.cbegin();it!=metadata.cend();++it)html+=QStringLiteral("<tr><td>%1</td><td>%2</td></tr>").arg(it.key().toHtmlEscaped(),it.value().toString().toHtmlEscaped().replace('\n',"<br>"));
 html+="</table><h2>Registered devices</h2>";
 for(const auto &entry:devices_)html+=QStringLiteral("<p>%1</p>").arg(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(entry.toMap())).toJson(QJsonDocument::Compact)).toHtmlEscaped());
 html+="<h2>Evidence and declared custody</h2>";
 for(const auto &key:{"evidence","custody"})for(const auto &entry:active_.value(key).toList())html+=QStringLiteral("<p>%1</p>").arg(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(entry.toMap())).toJson(QJsonDocument::Compact)).toHtmlEscaped());
 html+="<h2>Local audit trail</h2>";
 for(const auto &entry:audit_)html+=QStringLiteral("<p>%1</p>").arg(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(entry.toMap())).toJson(QJsonDocument::Compact)).toHtmlEscaped());
 html+="<p>Hash-linked protected records do not prove physical handover, independent signatures or resistance to whole-database rollback. Evidence acquisition/custody attachments are not inferred from case registration.</p></body></html>";
 {QPdfWriter writer(&output);writer.setTitle(QStringLiteral("NEXVARY case review"));writer.setCreator(QStringLiteral("NEXVARY-MR"));QTextDocument document;document.setDefaultFont(QFont(QStringLiteral("Segoe UI"),10));document.setHtml(html);document.print(&writer);}
 if(!output.flush()||output.size()==0){output.close();output.remove();return fail(QStringLiteral("PDF report could not be finalized."));}return true;
}
