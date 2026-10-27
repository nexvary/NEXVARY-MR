#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include "TskImage.hpp"
#include "RecoveryFile.hpp"
#include <QCryptographicHash>
#include <QFutureWatcher>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QtConcurrent>
void LawCaseBridge::cancelPartitions(){
 if(partitionsCancel_)partitionsCancel_->store(true);
 partitionsResult_.clear();emit partitionsChanged();
}
void LawCaseBridge::initPartitions(){
 partitions_=std::make_unique<QFutureWatcher<QVariantMap>>();
 connect(this,&LawCaseBridge::changed,this,[this]{
  const auto retained=partitionsActive_?partitionsContext_:partitionsResult_;
  if(!retained.isEmpty() && (retained.value("caseId")!=active_.value("id") || retained.value(partitionsActive_?"expectedRevision":"caseRevision")!=active_.value("revision")))cancelPartitions();
 });
 connect(partitions_.get(),&QFutureWatcher<QVariantMap>::finished,this,[this]{
  partitionsActive_=false;auto intent=partitionsContext_;const auto report=partitions_->result();partitionsContext_.clear();partitionsResult_.clear();
  const auto id=intent.value("caseId").toString(),op=intent.value("operationId").toString();
  // Keep original context across synchronous logout/revocation signals. A new
  // approval cannot authorize a worker launched under an older approval.
  const auto authorized=[&]{return partitionsCancel_ && !partitionsCancel_->load() && operationPolicy_ && operationPolicy_->caseOperationAuthorized(op) && permit(nexvary::law::Action::AnalyzeEvidence) && identity_ && identity_->userId()==intent.value("requester") && role()==intent.value("requesterRole") && active_.value("id")==id && active_.value("revision")==intent.value("expectedRevision") && caseAllowed(store_->get(id)) && store_->analysisSnapshotCurrent(intent);};
  const auto finish=[&](bool ok,const QString &status){
   const auto receipt=operationPolicy_?operationPolicy_->finishOperation(op,status,intent.value("sourceSha256").toString(),ok?intent.value("reportSha256").toString():QString()):QVariantMap();
   if(ok && receipt.value("status")!="completed"){
    ok=false;partitionsResult_.clear();transactionsReady_=false;fail("Case result exists but its operation receipt requires review; success withheld.");
   }
   if(ok && (!identity_ || !identity_->checkSession() || identity_->userId()!=intent.value("requester") || active_.value("id")!=id || !caseAllowed(store_->get(id)))){ok=false;partitionsResult_.clear();fail("Partition result committed but its session/workspace ended; result withheld.");}
   if(receipt.isEmpty()){transactionsReady_=false;fail("Partition operation receipt unavailable; preserve stores for reconciliation.");}
   emit partitionsChanged();emit partitionsCompleted(ok);
  };
  const auto reject=[&](const QString &reason){
   intent.insert("recoveryState","failed");
   if(!operationPolicy_ || !operationPolicy_->recordCaseOperation(intent))transactionsReady_=false;
   fail(reason);finish(false,partitionsCancel_&&partitionsCancel_->load()?"cancelled":"failed");
  };
  if(!authorized()){reject("Partition authority, session, case or cancellation state changed; no case write.");return;}
  if(report.contains("error")){reject(report.value("error").toString());return;}
  auto candidate=store_->partitionCandidate(intent,report);
  if(candidate.isEmpty()){reject(store_->error());return;}
  if(!operationPolicy_->recordCaseOperation(candidate)){
   transactionsReady_=false;fail("Partition candidate was not acknowledged; no case write.");finish(false,"failed");return;
  }
  intent=candidate;
  if(!authorized()){reject("Partition authority ended after candidate preparation; no case write.");return;}
  const auto result=store_->recordAnalysis(id,intent.value("expectedRevision").toLongLong(),intent.value("evidenceId").toString(),"partition-analysis",report,actor(),intent);
  intent.insert("recoveryState",store_->reconcileAnalysis(intent));
  if(!operationPolicy_->recordCaseOperation(intent) || intent.value("recoveryState")!="completed" || result.isEmpty()){
   transactionsReady_=false;fail("Partition case outcome requires reconciliation/review; success withheld.");finish(false,"failed");return;
  }
  if(!refresh(id)){transactionsReady_=false;finish(false,"failed");return;}
  partitionsResult_=result;partitionsResult_.insert("caseId",id);partitionsResult_.insert("caseRevision",active_.value("revision"));
  finish(true,"completed");
 });
}
bool LawCaseBridge::analyzePartitions(const QString &evidence,const QString &executable,bool consent){
 if(partitionsActive_)return fail("A case partition analysis is already running.");
 partitionsResult_.clear();emit partitionsChanged();
 if(!consent || !identity_ || !operationPolicy_ || !permit(nexvary::law::Action::AnalyzeEvidence) || active_.isEmpty())return fail("Confirm authorized registered evidence and a trusted separate mmls tool in an authenticated case.");
 const auto id=active_.value("id").toString();const auto revision=active_.value("revision").toLongLong();
 const auto source=store_->readEvidence(id,evidence);
 if(source.size()<512 || source.size()>16*1024*1024)return fail("Case partition evidence must be a verified registered raw image, 512 bytes to 16 MiB.");
 QString why;const auto tool=mr::recovery::localFilePath(executable);
 const auto toolHash=mr::forensic::partitionToolDigest(tool,why);
 if(toolHash.isEmpty())return fail(why);
 const auto operation=operationPolicy_->beginCasePartitions(id);
 if(operation.isEmpty())return fail(operationPolicy_->error());
 const auto op=operation.value("operationId").toString();
 const auto intent=store_->preparePartitions(id,revision,evidence,operation,QString::fromLatin1(toolHash.toHex()));
 if(intent.isEmpty() || !operationPolicy_->recordCaseOperation(intent)){
  operationPolicy_->finishOperation(op,"failed");transactionsReady_=false;return fail("Prepared partition intent not acknowledged; tool was not dispatched.");
 }
 // Signals from journal writes can change identity/grants synchronously.
 if(!operationPolicy_->caseOperationAuthorized(op) || !permit(nexvary::law::Action::AnalyzeEvidence) || active_.value("id")!=id || active_.value("revision").toLongLong()!=revision || !caseAllowed(store_->get(id)) || !store_->analysisSnapshotCurrent(intent) || QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex())!=intent.value("sourceSha256")){
  auto failed=intent;failed.insert("recoveryState","failed");if(!operationPolicy_->recordCaseOperation(failed))transactionsReady_=false;
  operationPolicy_->finishOperation(op,"cancelled");return fail("Authority ended after preparation; tool was not dispatched.");
 }
 partitionsContext_=intent;
 partitionsCancel_=std::make_shared<std::atomic_bool>(false);const auto token=partitionsCancel_;
 partitionsActive_=true;error_.clear();
 partitions_->setFuture(QtConcurrent::run([source,tool,toolHash,token]{
  const auto error=[](const QString &message){return QVariantMap{{"error",message}};};
  QTemporaryDir dir(QDir::tempPath()+"/nexvary-case-partition-XXXXXX");
  if(!dir.isValid() || !QFile::setPermissions(dir.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner))return error("Private partition staging unavailable.");
  QFile image(dir.filePath("registered-evidence.img"));
  if(!image.open(QIODevice::WriteOnly|QIODevice::NewOnly) || !image.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner) || image.write(source)!=source.size() || !image.flush())return error("Registered evidence staging failed.");
  image.close();
  mr::forensic::TskLimits limits;limits.maximumImage=16*1024*1024;limits.maximumOutput=8192;
  const auto scan=mr::forensic::analyzePartitions(image.fileName(),tool,token,{},limits);
  if(!dir.remove())return error("Private partition staging cleanup failed; preserve temporary storage for review.");
  if(!scan.error.isEmpty())return error(scan.error);
  if(!scan.sourceUnchanged || scan.toolSha256!=toolHash)return error("Prepared tool/source integrity verification failed.");
  return QVariantMap{{"schema","nexvary-case-partitions/v1"},{"sourceSha256",QString::fromLatin1(scan.sourceSha256.toHex())},{"toolSha256",QString::fromLatin1(scan.toolSha256.toHex())},{"tool","mmls"},{"version",scan.version},{"output",scan.output},{"sourceUnchanged",true},{"limitations","Raw partition listing only; separate process is not an OS sandbox. No device authenticity, complete descendant containment or network isolation established."}};
 }));
 emit partitionsChanged();return true;
}
