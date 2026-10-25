#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "LawRetention.hpp"
#include "IdentityService.hpp"
#include "LawTraining.hpp"
#include "DeviceAssessment.hpp"
#include "PcapSummary.hpp"
#include "ExifSummary.hpp"
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
#include <QTextCursor>
#include <QFontDatabase>
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
LawCaseBridge::LawCaseBridge(const QString &path,ToolBridge *tools,bool syntheticTestProfile,nexvary::law::Role role,QObject *parent,IdentityService *identity)
 :QObject(parent),store_(std::make_unique<LawCaseStore>(path,syntheticTestProfile)),tools_(tools),identity_(identity),synthetic_(syntheticTestProfile),role_(role) {
 if(identity_)connect(identity_,&IdentityService::changed,this,[this]{active_.clear();devices_.clear();audit_.clear();cases_.clear();attempts_.clear();if(ready())refresh();else emit changed();emit accessChanged();emit trainingChanged();});
 if(tools_)connect(tools_,&ToolBridge::deviceIntelligenceChanged,this,&LawCaseBridge::accessChanged);
 if(!store_->ready())error_=store_->error();
 else if(nexvary::law::allows(currentRole(),nexvary::law::Action::ReadCase))refresh();
}
LawCaseBridge::~LawCaseBridge()=default;
bool LawCaseBridge::ready()const{return store_->ready()&&(identity_?identity_->authenticated():synthetic_);}
nexvary::law::Role LawCaseBridge::currentRole()const{return identity_?identity_->lawRole():synthetic_?role_:nexvary::law::Role::Unknown;}
bool LawCaseBridge::caseAllowed(const QVariantMap &record)const{return !identity_?synthetic_:identity_->authenticated()&&(record.value("ownerUserId").toString()==identity_->userId()||identity_->hasCaseGrant(record.value("id").toString()));}
QString LawCaseBridge::protection()const {
 return mr::recovery::vaultAvailable()?QStringLiteral("Windows current-account DPAPI; SQLite payloads protected"):
  synthetic_?QStringLiteral("SYNTHETIC TEST DATA ONLY — NOT ENCRYPTED"):QStringLiteral("Windows protected storage required; real cases disabled");
}
QString LawCaseBridge::role()const{return QString::fromUtf8(nexvary::law::roleId(currentRole()).data(),static_cast<qsizetype>(nexvary::law::roleId(currentRole()).size()));}
QVariantMap LawCaseBridge::permissions()const {
 using A=nexvary::law::Action;QVariantMap p;
 for(const auto &entry:std::initializer_list<std::pair<const char *,A>>{{"retention",A::ManageRetention},{"preserve",A::PreserveEvidence},{"releasePreservation",A::ReleasePreservation},{"archiveEvidence",A::ArchiveEvidence},{"requestDisposal",A::RequestDisposal},{"reviewDisposal",A::ReviewDisposal},{"read",A::ReadCase},{"create",A::CreateCase},{"edit",A::EditCase},{"archive",A::ArchiveCase},{"close",A::CloseCase},{"addDevice",A::AddDevice},{"report",A::ExportReport},{"exportEvidence",A::ExportEvidence},{"analyze",A::AnalyzeEvidence},{"backup",A::BackupCase},{"restore",A::RestoreCase},{"audit",A::ReadAudit},{"training",A::AccessTraining},{"addEvidence",A::AddEvidence},{"custody",A::TransferCustody}})
  p.insert(entry.first,nexvary::law::allows(currentRole(),entry.second));
 p.insert("executeDevice",false);p.insert("enterpriseRoleProvisioningReady",false);return p;
}
QString LawCaseBridge::actor()const{if(identity_)return identity_->actor();return synthetic_?QStringLiteral("SYNTHETIC operator"):QStringLiteral("Current OS account — local operator declaration");}
bool LawCaseBridge::fail(const QString &message){error_=message;emit changed();return false;}
bool LawCaseBridge::permit(nexvary::law::Action action,bool caseBound) {
 if(identity_&&!identity_->checkSession())return fail(QStringLiteral("Sign in with a permitted local account."));
 if(caseBound&&!active_.isEmpty()&&!caseAllowed(active_))return fail(QStringLiteral("This account has no grant for the active case."));
 if(!nexvary::law::allows(currentRole(),action))return fail(QStringLiteral("This session role does not permit this action."));
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
  auto found=store_->get(id);if(found.isEmpty())return fail(store_->error());if(!caseAllowed(found))return fail(QStringLiteral("No case grant for this account."));active_=found;
 }
 if(nexvary::law::allows(currentRole(),nexvary::law::Action::ReadCase)){cases_.clear();for(const auto &row:store_->list())if(caseAllowed(row.toMap()))cases_.append(row);}else cases_.clear();
 if(!store_->error().isEmpty())return fail(store_->error());
 if(!active_.isEmpty()) {
  const auto currentId=active_.value("id").toString();devices_=nexvary::law::allows(currentRole(),nexvary::law::Action::ReadDevice)?store_->devices(currentId):QVariantList();
  audit_=nexvary::law::allows(currentRole(),nexvary::law::Action::ReadAudit)?store_->audit(currentId):QVariantList();
  if(!store_->error().isEmpty())return fail(store_->error());
 }
 error_.clear();emit changed();return true;
}
QString LawCaseBridge::createCase(const QVariantMap &metadata) {
 if(!permit(nexvary::law::Action::CreateCase,false)||!validateMetadata(metadata))return {};
 const auto id=store_->create(metadata,actor(),identity_?identity_->userId():QString());if(id.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?id:QString();
}
bool LawCaseBridge::updateCase(const QVariantMap &metadata) {
 if(!permit(nexvary::law::Action::EditCase)||active_.isEmpty()||!validateMetadata(metadata))return false;
 const auto id=active_.value("id").toString();
 if(!store_->update(id,active_.value("revision").toLongLong(),metadata,actor()))return fail(store_->error());
 return refresh(id);
}
bool LawCaseBridge::openCase(const QString &id) {return permit(nexvary::law::Action::ReadCase,false)&&refresh(id);}
void LawCaseBridge::closeWorkspace(){active_.clear();devices_.clear();audit_.clear();error_.clear();emit changed();}
bool LawCaseBridge::searchCases(const QString &query) {
 if(!permit(nexvary::law::Action::ReadCase,false))return false;
 if(query.size()>256)return fail(QStringLiteral("Search query exceeds limit."));
 cases_.clear();for(const auto &row:store_->list(query))if(caseAllowed(row.toMap()))cases_.append(row);if(!store_->error().isEmpty())return fail(store_->error());
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
 if(!tools_||!nexvary::law::allows(currentRole(),nexvary::law::Action::AddDevice))return {};
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
 if(!nexvary::law::allows(currentRole(),nexvary::law::Action::ReadDevice))return rows;
 const auto report=tools_?tools_->deviceIntelligence():QVariantMap();
 rows=nexvary::law::assessmentFields(report);
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
QVariantMap LawCaseBridge::analyzeEvidence(const QString &evidence,const QString &parser) {
 if(!permit(nexvary::law::Action::AnalyzeEvidence)||active_.isEmpty())return {};
 if(parser!="classic-pcap"&&parser!="jpeg-exif"){fail("Choose an implemented classic-PCAP or JPEG-EXIF summary parser.");return {};}
 const auto id=active_.value("id").toString();const auto source=store_->readEvidence(id,evidence);if(source.isEmpty()){fail(store_->error());return {};}
 if(source.size()>8*1024*1024){fail("Case analysis accepts sources up to 8 MiB.");return {};}
 QVariantMap report;QString parseError;
 if(parser=="classic-pcap"){const auto result=mr::forensic::summarizeCapture(source);report=result.report.toVariantMap();parseError=result.error;}
 else {const auto result=mr::forensic::summarizeExif(source);report=result.report.toVariantMap();parseError=result.error;}
 if(!parseError.isEmpty()||report.isEmpty()){fail(QStringLiteral("Analysis rejected source: ")+parseError);return {};}
 const auto result=store_->recordAnalysis(id,active_.value("revision").toLongLong(),evidence,parser,report,actor());
 if(result.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?result:QVariantMap();
}
QVariantMap LawCaseBridge::exportProtectedBackup(const QString &path,bool consent) {
 if(!consent){fail("Confirm protected case backup and its current-account recovery boundary.");return {};}
 if(!permit(nexvary::law::Action::BackupCase)||active_.isEmpty())return {};
 const auto id=active_.value("id").toString();const auto result=store_->exportProtectedBackup(id,active_.value("revision").toLongLong(),path,actor());
 if(result.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?result:QVariantMap();
}
QString LawCaseBridge::restoreProtectedBackup(const QString &path,const QString &expectedSha256,bool consent) {
 if(!consent){fail("Confirm restoration to a database without this case; originals are never overwritten.");return {};}
 if(!permit(nexvary::law::Action::RestoreCase,false))return {};
 const auto id=store_->restoreProtectedBackup(path,expectedSha256,actor(),[this](const QVariantMap &record){
  if(identity_&&!identity_->checkSession())return false;
  return nexvary::law::allows(currentRole(),nexvary::law::Action::RestoreCase)&&caseAllowed(record)&&validateMetadata(record.value("metadata").toMap());
 });
 if(id.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?id:QString();
}
bool LawCaseBridge::verifyEvidence(const QString &evidence) {
 if(!permit(nexvary::law::Action::ReadDevice)||active_.isEmpty())return false;
 if(!store_->verifyEvidence(active_.value("id").toString(),evidence))return fail(store_->error());
 error_.clear();emit changed();return true;
}
QVariantMap LawCaseBridge::exportWorkingCopy(const QString &evidence,const QString &directory,const QString &purpose,bool consent) {
 if(!consent){fail("Explicit consent is required to export unencrypted evidence.");return {};}
 if(!permit(nexvary::law::Action::ExportEvidence)||active_.isEmpty())return {};
 const auto id=active_.value("id").toString();
 const auto result=store_->exportWorkingCopy(id,active_.value("revision").toLongLong(),evidence,directory,purpose,actor());
 if(result.isEmpty()){fail(store_->error());return {};}
 return refresh(id)?result:QVariantMap();
}
bool LawCaseBridge::verifyAudit() {
 if(!permit(nexvary::law::Action::ReadAudit)||active_.isEmpty())return false;
 if(!store_->verifyAudit(active_.value("id").toString()))return fail(store_->error());
 error_.clear();emit changed();return true;
}
bool LawCaseBridge::exportAuditAnchor(const QString &path,bool consent){
 if(!consent||!identity_||!permit(nexvary::law::Action::ReadAudit)||active_.isEmpty()||!refresh(active_.value("id").toString())||!verifyAudit())return fail("A signed-in case reader, valid audit and explicit anchor export consent are required.");
 QVariantList evidence;for(const auto &value:active_.value("evidence").toList()){const auto item=value.toMap();if(!store_->verifyEvidence(active_.value("id").toString(),item.value("id").toString()))return fail(store_->error());evidence.append(QVariantMap{{"id",item.value("id")},{"sha256",item.value("sha256")},{"size",item.value("size")}});}
 const auto signature=identity_->attest("case-audit-anchor",{{"caseId",active_.value("id")},{"revision",active_.value("revision")},{"auditHead",active_.value("audit").toList().last().toMap().value("hash")},{"evidence",evidence}});if(signature.isEmpty())return fail("Anchor signing failed.");
 QFile file;if(!newOutput(path,file))return fail("Choose a new local anchor file.");const auto bytes=QJsonDocument::fromVariant(signature).toJson();if(file.write(bytes)!=bytes.size()||!file.flush()){file.close();file.remove();return fail("Anchor export failed.");}error_.clear();emit changed();return true;
}
bool LawCaseBridge::verifyAuditAnchor(const QString &path){
 if(!identity_||!permit(nexvary::law::Action::ReadAudit)||active_.isEmpty()||!refresh(active_.value("id").toString())||!verifyAudit())return false;
 const auto local=mr::recovery::localFilePath(path);QFile file(local);if(local.isEmpty()||QFileInfo(local).isSymLink()||!mr::recovery::openRegularInput(file)||file.size()>128*1024)return fail("Choose a bounded regular local anchor file.");QJsonParseError parse;const auto doc=QJsonDocument::fromJson(file.readAll(),&parse);const auto envelope=doc.object().toVariantMap(),payload=envelope.value("payload").toMap();
 if(parse.error!=QJsonParseError::NoError||!doc.isObject()||!identity_->verify(envelope,"case-audit-anchor",payload)||payload.value("caseId")!=active_.value("id"))return fail("Anchor signature, pinned local signer or case identifier did not verify.");
 const auto revision=payload.value("revision").toLongLong();const auto events=active_.value("audit").toList();if(revision<1||revision>events.size())return fail("Current case is older than the retained anchor: possible rollback.");if(events[static_cast<qsizetype>(revision-1)].toMap().value("hash")!=payload.value("auditHead"))return fail("Case history diverges from the retained signed anchor.");error_.clear();emit changed();return true;
}
QVariantList LawCaseBridge::scenarios()const{return nexvary::law::allows(currentRole(),nexvary::law::Action::AccessTraining)?nexvary::law::trainingScenarios():QVariantList();}
QVariantMap LawCaseBridge::evaluateTraining(const QString &scenarioId,const QString &choiceId) {
 if(!nexvary::law::allows(currentRole(),nexvary::law::Action::AccessTraining))return {{"valid",false}};
 auto result=nexvary::law::evaluateTraining(scenarioId,choiceId);if(!result.value("valid").toBool())return result;
 result.insert("scenarioId",scenarioId);result.insert("choiceId",choiceId);result.insert("atUtc",utc());result.insert("synthetic",true);
 attempts_.append(result);if(attempts_.size()>200)attempts_.removeFirst();emit trainingChanged();return result;
}
bool LawCaseBridge::exportTrainingReport(const QString &path,bool consent) {
 if(!consent||!nexvary::law::allows(currentRole(),nexvary::law::Action::AccessTraining))return fail(QStringLiteral("Explicit synthetic-report export approval is required."));
 QFile output;if(!newOutput(path,output))return fail(QStringLiteral("Choose a new local report file."));
 const auto bytes=QJsonDocument(QJsonObject{{"schemaVersion",1},{"synthetic",true},{"generatedAtUtc",utc()},{"attempts",QJsonArray::fromVariantList(attempts_)},{"notice","Training decisions only; no real case or device operation."}}).toJson();
 if(output.write(bytes)!=bytes.size()||!output.flush()){output.close();output.remove();return fail(QStringLiteral("Training report could not be saved."));}return true;
}
bool LawCaseBridge::exportCasePdf(const QString &path,bool arabic,bool consent) {
 if(!consent||!permit(nexvary::law::Action::ExportReport)||active_.isEmpty()||!store_->verifyAudit(active_.value("id").toString()))return fail(QStringLiteral("A readable case, verified local audit and explicit plaintext-export consent are required."));
 if(!refresh(active_.value("id").toString()))return false;
 for(const auto &entry:active_.value("evidence").toList())if(!store_->verifyEvidence(active_.value("id").toString(),entry.toMap().value("id").toString()))return fail(store_->error());
 QFile output;if(!newOutput(path,output))return fail(QStringLiteral("Choose a new local PDF file."));
 const auto tr=[arabic](const QString &ar,const QString &en){return arabic?ar:en;};
 const QMap<QString,QString> labels{
  {"label",QStringLiteral("اسم السياسة")},{"days",QStringLiteral("مدة الاحتفاظ بالأيام")},{"archiveAfterDays",QStringLiteral("الأرشفة بعد أيام")},{"policyId",QStringLiteral("معرّف السياسة")},{"kind",QStringLiteral("نوع قيد الحفظ")},{"releasedAtUtc",QStringLiteral("وقت رفع القيد UTC")},{"releasedBy",QStringLiteral("هوية رافع القيد")},{"releaseReason",QStringLiteral("سبب رفع القيد")},{"reviewedAtUtc",QStringLiteral("وقت المراجعة UTC")},{"reviewedBy",QStringLiteral("هوية المراجع")},{"reviewReason",QStringLiteral("سبب قرار المراجعة")},{"reviewAuthorityReference",QStringLiteral("مرجع سند المراجعة")},{"operation",QStringLiteral("إجراء الاحتفاظ")},{"fields",QStringLiteral("مدخلات الإجراء")},{"storage",QStringLiteral("موضع الأرشيف")},
  {"caseNumber",QStringLiteral("رقم القضية")},{"investigatingAgency",QStringLiteral("جهة الضبط")},{"investigatorName",QStringLiteral("المحقق المسؤول")},
  {"seizedDeviceDescription",QStringLiteral("وصف المضبوطات")},{"authorityReference",QStringLiteral("مرجع الإذن أو السند")},{"issuingAuthority",QStringLiteral("الجهة المصدرة")},
  {"prosecutorName",QStringLiteral("اسم وكيل النيابة")},{"prosecutorOffice",QStringLiteral("النيابة المختصة")},{"courtName",QStringLiteral("المحكمة")},
  {"scope",QStringLiteral("نطاق التفويض المسجل")},{"jurisdiction",QStringLiteral("الولاية القضائية")},{"validFromUtc",QStringLiteral("بداية السريان UTC")},
  {"validUntilUtc",QStringLiteral("نهاية السريان UTC")},{"incidentDescription",QStringLiteral("وصف الواقعة")},{"seizureAtUtc",QStringLiteral("وقت الضبط UTC")},
  {"seizurePlace",QStringLiteral("مكان الضبط")},{"notes",QStringLiteral("الملاحظات")},{"id",QStringLiteral("المعرّف الداخلي")},
  {"manufacturer",QStringLiteral("الشركة المصنعة")},{"model",QStringLiteral("الموديل")},{"serial",QStringLiteral("الرقم التسلسلي")},{"imei","IMEI"},
  {"condition",QStringLiteral("حالة الجهاز")},{"powerState",QStringLiteral("حالة التشغيل")},{"connectionState",QStringLiteral("حالة الاتصال")},
  {"lockState",QStringLiteral("حالة القفل المرصودة")},{"receivedAtUtc",QStringLiteral("وقت الاستلام UTC")},{"custodian",QStringLiteral("المسؤول عن الحيازة")},
  {"accessories",QStringLiteral("الملحقات")},{"observationSummary",QStringLiteral("ملخص المشاهدات")},{"registeredAtUtc",QStringLiteral("وقت التسجيل UTC")},
  {"deviceId",QStringLiteral("معرّف جهاز القضية")},{"evidenceId",QStringLiteral("معرّف الدليل")},{"name",QStringLiteral("اسم الملف")},
  {"description",QStringLiteral("وصف الدليل")},{"size",QStringLiteral("الحجم بالبايت")},{"sha256","SHA-256"},
  {"from",QStringLiteral("من المسؤول")},{"to",QStringLiteral("إلى المسؤول")},{"reason",QStringLiteral("سبب النقل")},{"atUtc",QStringLiteral("وقت الحدث UTC")},
  {"actor",QStringLiteral("هوية المنفّذ المسجلة")},{"event",QStringLiteral("الإجراء")},{"sequence",QStringLiteral("رقم الحدث")},
  {"backupId",QStringLiteral("معرّف النسخة المحمية")},{"backupSha256",QStringLiteral("بصمة النسخة المحمية SHA-256")},{"sourceAuditHead",QStringLiteral("مرجع تدقيق المصدر")},{"provider",QStringLiteral("مزود الحماية")},
  {"exportedAtUtc",QStringLiteral("وقت التصدير UTC")},{"manifestSha256",QStringLiteral("بصمة Manifest SHA-256")},
  {"packageId",QStringLiteral("معرّف حزمة نسخة العمل")},{"purpose",QStringLiteral("الغرض المسجل")},{"sourceRevision",QStringLiteral("مراجعة القضية المصدر")},
  {"implementationVersion",QStringLiteral("إصدار المحرك")},{"parser",QStringLiteral("محلل الأدلة")},{"reportJson",QStringLiteral("النتيجة التقنية JSON")},
  {"reportSha256",QStringLiteral("بصمة نتيجة التحليل SHA-256")},{"sourceSha256",QStringLiteral("بصمة ملف المصدر SHA-256")},
  {"status",QStringLiteral("الحالة")},{"sourceSize",QStringLiteral("حجم المصدر بالبايت")},{"synthetic",QStringLiteral("بيانات اصطناعية")},
  {"hash",QStringLiteral("بصمة الحدث SHA-256")},{"previous",QStringLiteral("بصمة الحدث السابق")},{"stateSha256",QStringLiteral("بصمة حالة القضية")},{"caseId",QStringLiteral("معرّف القضية")}
 };
 const QStringList technical{"id","deviceId","evidenceId","caseId","serial","imei","sha256","hash","previous","stateSha256","atUtc","registeredAtUtc","receivedAtUtc","validFromUtc","validUntilUtc","seizureAtUtc","manifestSha256","sourceSha256","reportSha256","packageId","exportedAtUtc","reportJson","implementationVersion","parser","backupId","backupSha256","sourceAuditHead","provider","policyId","fields","releasedAtUtc","reviewedAtUtc"};
 const auto table=[&](const QVariantMap &record,const QString &caption){
  QString out=QStringLiteral("<table border='1' cellspacing='0' cellpadding='4' width='100%'><thead><tr><th colspan='2' bgcolor='#e0e7ef'>%1</th></tr></thead>").arg(caption.toHtmlEscaped());
  for(auto it=record.cbegin();it!=record.cend();++it){
   if(it.value().toString().isEmpty()&&it.value().metaType().id()!=QMetaType::QVariantMap)continue;
   const auto key=arabic?labels.value(it.key(),it.key()):it.key();
   auto displayed=it.value().metaType().id()==QMetaType::QVariantMap?QString::fromUtf8(QJsonDocument::fromVariant(it.value()).toJson(QJsonDocument::Indented)):it.value().toString();
   if(it.key()=="reportJson"){
    const auto report=QJsonDocument::fromJson(displayed.toUtf8());
    if(report.isObject())displayed=QString::fromUtf8(report.toJson(QJsonDocument::Indented)).trimmed();
   }
   const auto value=displayed.toHtmlEscaped().replace('\n',"<br>");
   out+=QStringLiteral("<tr><td width='29%' bgcolor='#edf1f7'>%1</td><td><p dir='%2' style='font-size:9pt'>%3</p></td></tr>").arg(key.toHtmlEscaped(),technical.contains(it.key())?"ltr":arabic?"rtl":"ltr",value);
  }
  return out+"</table>";
 };
 QString html=QStringLiteral("<html dir='%1'><head><meta charset='UTF-8'></head><body><h1>NEXVARY</h1><h2>%2</h2>").arg(arabic?"rtl":"ltr",tr(QStringLiteral("تقرير مراجعة قضية"),"Case review report"));
 html+=QStringLiteral("<p dir='ltr'>%1<br>%2</p><p>%3</p>").arg(active_.value("id").toString().toHtmlEscaped(),utc(),tr(QStringLiteral("إصدار تطوير: إقرارات ومشاهدات محلية؛ لا يثبت تفويضًا قانونيًا أو تسليمًا ماديًا أو اعتمادًا قضائيًا أو استخلاص هاتف محمي. التوقيعات والأوقات محلية، ولا تمنع استعادة قاعدة قديمة دون مرجع مستقل. التحقق المؤسسي غير مكتمل."),"Development prototype: local declarations and observations, not legal approval, physical handover proof, judicial accreditation or protected-phone acquisition. Signatures and times are local; rollback detection needs a separately retained anchor. Institutional identity provisioning remains incomplete."));
 html+=QStringLiteral("<h2>%1</h2>").arg(tr(QStringLiteral("القضية والسند القانوني"),"Case and recorded authority"))+table(active_.value("metadata").toMap(),tr(QStringLiteral("بيانات القضية"),"Case metadata"));
 const auto section=[&](const QString &key,const QString &title){
  QString out;const auto rows=active_.value(key).toList();
  if(rows.isEmpty())return table({{"status",tr(QStringLiteral("لا توجد سجلات."),"No records.")}},title);
  int index=0;for(const auto &row:rows)out+=table(row.toMap(),title+QStringLiteral(" - %1 %2").arg(tr(QStringLiteral("سجل"),"Record")).arg(++index));return out;
 };
 html+=section("devices",tr(QStringLiteral("الأجهزة المسجلة"),"Registered devices"));
 html+=section("evidence",tr(QStringLiteral("الأدلة وسلامة النسخ المحفوظة"),"Evidence and stored-copy integrity"));
 html+=section("custody",tr(QStringLiteral("إقرارات سلسلة الحيازة"),"Declared chain of custody"));
 html+=section("workingCopies",tr(QStringLiteral("نسخ العمل المصدرة"),"Exported working copies"));
 if(!active_.value("backups").toList().isEmpty())html+=section("backups",tr(QStringLiteral("النسخ المحمية المسجلة"),"Protected case backups"));
 if(!active_.value("restorations").toList().isEmpty())html+=section("restorations",tr(QStringLiteral("سجل استعادة النسخ"),"Protected backup restorations"));
 html+=section("analyses",tr(QStringLiteral("تحليلات الأدلة"),"Evidence analyses"));
 if(active_.contains("retention")){const auto g=active_.value("retention").toMap();for(const auto &key:{"policies","assignments","holds","archives","disposals","events"}){for(const auto &v:g.value(key).toList())html+=table(v.toMap(),tr(QStringLiteral("الاحتفاظ والأرشفة"),"Retention & archiving")+" / "+QString::fromLatin1(key));}}
 html+=section("audit",tr(QStringLiteral("سجل التدقيق المترابط"),"Linked audit trail"));
 html+=QStringLiteral("</body></html>");
 {QPdfWriter writer(&output);writer.setTitle(QStringLiteral("NEXVARY case review"));writer.setCreator(QStringLiteral("NEXVARY-MR"));QTextDocument document;document.setDefaultFont(QFont(QStringLiteral("Segoe UI"),10));document.setHtml(html);
  // Preserve exact ASCII when copying hashes/UUIDs from a PDF. Default fonts can
  // encode an ff ligature as U+FB00 in ToUnicode instead of two ASCII characters.
  QFont fixed=QFontDatabase::systemFont(QFontDatabase::FixedFont);fixed.setPointSize(9);fixed.setStyleStrategy(QFont::PreferNoShaping);
  QTextCharFormat identifierFormat;identifierFormat.setFont(fixed);
  const QRegularExpression identifiers(QStringLiteral("\\b(?:[0-9a-f]{64}|[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})\\b"),QRegularExpression::CaseInsensitiveOption);
  for(auto cursor=document.find(identifiers);!cursor.isNull();cursor=document.find(identifiers,cursor))cursor.mergeCharFormat(identifierFormat);
  document.print(&writer);}
 if(!output.flush()||output.size()==0){output.close();output.remove();return fail(QStringLiteral("PDF report could not be finalized."));}return true;
}

bool LawCaseBridge::retentionOperation(const QString &operation,const QVariantMap &fields) {
 using A=nexvary::law::Action;
 const QMap<QString,A> actions{{"policy",A::ManageRetention},{"assign",A::ManageRetention},{"hold",A::PreserveEvidence},{"release",A::ReleasePreservation},{"archive",A::ArchiveEvidence},{"request-disposal",A::RequestDisposal},{"review-disposal",A::ReviewDisposal}};
 if(!actions.contains(operation))return fail("Unknown retention action.");
 if(!permit(actions.value(operation))||active_.isEmpty())return false;
 if(operation=="review-disposal"&&!identity_)return fail("Disposal review requires independently authenticated local accounts.");
 const auto id=active_.value("id").toString();
 if(!store_->retentionOperation(id,active_.value("revision").toLongLong(),operation,fields,actor()))return fail(store_->error());
 return refresh(id);
}
QVariantMap LawCaseBridge::retentionAssessment(const QString &evidence) {
 if(!permit(nexvary::law::Action::ReadCase)||active_.isEmpty())return {};
 const auto r=store_->get(active_.value("id").toString());if(r.isEmpty()){fail(store_->error());return {};}
 return nexvary::law::assessRetention(r,evidence,QDateTime::currentDateTimeUtc());
}
QVariantMap LawCaseBridge::verifyArchive(const QString &archiveId) {
 if(!permit(nexvary::law::Action::ReadAudit)||active_.isEmpty())return {};
 const auto result=store_->verifyArchive(active_.value("id").toString(),archiveId);
 if(result.isEmpty())fail(store_->error());
 return result;
}
