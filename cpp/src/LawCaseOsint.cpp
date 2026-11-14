#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "PhoneOsint.hpp"
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QTimer>
#include <QtConcurrent>
namespace {QString digest(const QVariantMap &m){return QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(QJsonObject::fromVariantMap(m)).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());}}
void LawCaseBridge::cancelOsint(){if(osintCancel_)osintCancel_->store(true);osintResult_.clear();emit osintChanged();}
void LawCaseBridge::initOsint(){
 osint_=std::make_unique<QFutureWatcher<QVariantMap>>();
 auto *monitor=new QTimer(this);monitor->setInterval(50);
 connect(monitor,&QTimer::timeout,this,[this]{
  if(!osintActive_)return;
  if(!operationPolicy_ || !identity_ || !identity_->checkSession() || identity_->userId()!=osintContext_.value("requester") || active_.value("id").toString()!=osintContext_.value("caseId").toString() || active_.value("revision")!=osintContext_.value("revision") || !operationPolicy_->caseOperationAuthorized(osintContext_.value("operationId").toString()) || (!osintContext_.value("caseId").toString().isEmpty()&&!caseAllowed(store_->get(active_.value("id").toString()))))cancelOsint();
 });monitor->start();
 connect(this,&LawCaseBridge::changed,this,[this]{
  if(osintContext_.isEmpty())return;
  if(active_.value("id").toString()!=osintContext_.value("caseId").toString() || (!osintActive_ && active_.value("revision")!=osintContext_.value("retainedRevision")))cancelOsint();
 });
 connect(osint_.get(),&QFutureWatcher<QVariantMap>::finished,this,[this]{
  osintActive_=false;osintResult_.clear();auto report=osint_->result();const auto context=osintContext_;
  const auto operation=context.value("operationId").toString();
  const auto current=[&]{return osintCancel_&&!osintCancel_->load() && identity_ && identity_->checkSession() && identity_->userId()==context.value("requester") && role()==context.value("requesterRole") && operationPolicy_ && operationPolicy_->caseOperationAuthorized(operation) && active_.value("id").toString()==context.value("caseId").toString() && active_.value("revision")==context.value("revision") && (context.value("caseId").toString().isEmpty()||caseAllowed(store_->get(context.value("caseId").toString()))) && permit(nexvary::law::Action::AnalyzeEvidence) && permit(nexvary::law::Action::AddEvidence);};
  if(!current() || report.contains("error")){
   if(operationPolicy_)operationPolicy_->finishOperation(operation,osintCancel_&&osintCancel_->load()?"cancelled":"failed",context.value("requestSha256").toString());
   fail(report.value("error","OSINT authority or case revision ended; result withheld.").toString());emit osintChanged();return;
  }
  report.insert("caseId",context.value("caseId"));report.insert("operationId",operation);report.insert("requestSha256",context.value("requestSha256"));
  const auto output=digest(report);
  if(context.value("caseId").toString().isEmpty()){
   const auto receipt=operationPolicy_->finishOperation(operation,"completed",context.value("requestSha256").toString(),output);
   if(receipt.value("status")!="completed"){fail("Could not record investigation result.");emit osintChanged();return;}
   osintContext_.clear();osintResult_={{"report",report},{"reportSha256",output},{"receipt",receipt}};emit osintChanged();return;
  }
  // Store an explicit operation link inside the protected evidence payload.
  // Case commit and policy receipt are separate stores; a missing receipt is
  // explicitly withheld for review, never represented as an accepted result.
  const auto evidence=store_->registerOsint(context.value("caseId").toString(),context.value("revision").toLongLong(),context.value("deviceId").toString(),report,actor());
  const auto receipt=operationPolicy_->finishOperation(operation,evidence.isEmpty()?"failed":"completed",context.value("requestSha256").toString(),evidence.isEmpty()?QString():output);
  if(evidence.isEmpty()||receipt.value("status")!="completed"){
   fail("OSINT evidence/operation receipt incomplete; preserve case and policy stores for review.");emit osintChanged();return;
  }
  if(!identity_->checkSession()||identity_->userId()!=context.value("requester")||active_.value("id").toString()!=context.value("caseId").toString()||!caseAllowed(store_->get(context.value("caseId").toString()))){emit osintChanged();return;}
  osintContext_.clear();refresh(context.value("caseId").toString());
  osintContext_=context;osintContext_.insert("retainedRevision",active_.value("revision"));
  osintResult_={{"report",report},{"reportSha256",output},{"evidenceId",evidence},{"receipt",receipt}};emit osintChanged();
 });
}
bool LawCaseBridge::runPhoneOsint(const QString &number,const QStringList &scanners,const QString &engine,const QString &engineSha256,const QString &device,bool consent){
 if(osintActive_)return fail("An OSINT investigation is already running.");
 osintResult_.clear();emit osintChanged();
 const auto normalized=mr::osint::normalize(number);
 if(!consent || normalized.isEmpty() || !identity_ || !operationPolicy_ || (active_.isEmpty()&&!identity_->directWork()) || !permit(nexvary::law::Action::AnalyzeEvidence) || !permit(nexvary::law::Action::AddEvidence))return fail("A single international number, authenticated case and explicit authority are required.");
 bool found=false;for(const auto &d:devices_)if(d.toMap().value("id")==device)found=true;
 if(!device.isEmpty()&&!found)return fail("Select a registered case device or leave it empty for case-level evidence.");
 auto selected=scanners;selected.removeDuplicates();
 if(selected.isEmpty()||selected.size()!=scanners.size()||selected.size()>5)return fail("Select distinct supported scanners.");
 for(const auto &s:selected)if(!mr::osint::supportedScanners().contains(s))return fail("Unsupported scanner.");
 const auto id=active_.value("id",QString());const auto revision=active_.value("revision");
 const auto operation=operationPolicy_->beginCaseOsint(id.toString(),normalized);
 if(operation.isEmpty())return fail(operationPolicy_->error());
 const auto op=operation.value("operationId").toString();
 if(!operationPolicy_->caseOperationAuthorized(op)||active_.value("id").toString()!=id.toString()||active_.value("revision")!=revision||!permit(nexvary::law::Action::AnalyzeEvidence)||!permit(nexvary::law::Action::AddEvidence)){
  operationPolicy_->finishOperation(op,"cancelled");return fail("OSINT authority ended before dispatch.");
 }
 osintContext_={{"caseId",id},{"revision",revision},{"deviceId",device},{"operationId",op},{"requester",identity_->userId()},{"requesterRole",role()},
  {"requestSha256",digest({{"number",normalized},{"scanners",scanners},{"engineSha256",engineSha256},{"caseId",id},{"deviceId",device}})}};
 osintCancel_=std::make_shared<std::atomic_bool>(false);const auto token=osintCancel_;
 osintActive_=true;const mr::osint::Request request{normalized,engine,engineSha256,scanners};
 osint_->setFuture(QtConcurrent::run([request,token]{mr::osint::PhoneInfoga provider;return provider.run(request,token);}));emit osintChanged();return true;
}
