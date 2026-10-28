#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "ForensicAdapters.hpp"
#include "IdentityService.hpp"
#include <QCryptographicHash>
#include <QFutureWatcher>
#include <QtConcurrent>

void LawCaseBridge::cancelForensicAnalysis(){
 if(forensicCancel_)forensicCancel_->store(true);
 forensicContext_.clear();forensicResult_.clear();emit forensicChanged();
}
void LawCaseBridge::initForensicAdapters(){
 forensic_=std::make_unique<QFutureWatcher<QVariantMap>>();
 connect(this,&LawCaseBridge::changed,this,[this]{
  const auto context=forensicContext_.isEmpty()?forensicResult_:forensicContext_;
  if(!context.isEmpty()&&(context.value("caseId")!=active_.value("id")||context.value("caseRevision")!=active_.value("revision")||context.value("actor")!=actor()))cancelForensicAnalysis();
 });
 connect(forensic_.get(),&QFutureWatcher<QVariantMap>::finished,this,[this]{
  forensicActive_=false;const auto context=forensicContext_;const auto report=forensic_->result();forensicContext_.clear();forensicResult_.clear();
  const auto finish=[this](bool ok){emit forensicChanged();emit forensicCompleted(ok);};
  if(context.isEmpty()||!forensicCancel_||forensicCancel_->load()){finish(false);return;}
  if(!permit(nexvary::law::Action::AnalyzeEvidence,false)||active_.value("id")!=context.value("caseId")||actor()!=context.value("actor")){finish(false);return;}
  const auto id=context.value("caseId").toString(),evidence=context.value("evidenceId").toString();
  const auto current=store_->get(id);
  if(current.isEmpty()||!caseAllowed(current)||current.value("revision")!=context.value("caseRevision")||active_.value("revision")!=context.value("caseRevision")){
   fail("Forensic result refused: case revision or authority changed.");finish(false);return;
  }
  if(report.contains("errorCode")){fail("Forensic parser rejected: "+report.value("errorCode").toString());finish(false);return;}
  const auto source=store_->readEvidence(id,evidence);
  const auto digest=QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex());
  if(source.isEmpty()||digest!=context.value("sourceSha256")||report.value("sourceSha256")!=digest||report.value("caseId")!=id||report.value("evidenceId")!=evidence){
   fail("Forensic result refused: source/provenance changed.");finish(false);return;
  }
  const auto result=retainLocalAnalysis(id,context.value("caseRevision").toLongLong(),evidence,context.value("adapter").toString(),report);
  // retainLocalAnalysis uses the existing prepared journal and SQLite transaction;
  // refresh may emit changed synchronously. Publish only after that succeeds.
  if(result.isEmpty()){finish(false);return;}
  forensicResult_=result;forensicResult_.insert("caseId",id);forensicResult_.insert("caseRevision",active_.value("revision"));forensicResult_.insert("actor",actor());
  const mr::forensic::ForensicOperationReceipt receipt{result.value("operationId").toString(),result.value("id").toString(),result.value("reportSha256").toString()};
  forensicResult_.insert("signedAuditReference",receipt.toMap());
  finish(true);
 });
}
bool LawCaseBridge::analyzeForensicEvidence(const QString &evidence,const QString &adapter){
 if(!permit(nexvary::law::Action::AnalyzeEvidence)||active_.isEmpty()){cancelForensicAnalysis();return false;}
 if(forensicActive_)return fail("A forensic adapter is already running.");
 if(!mr::forensic::forensicAdapters().contains(adapter))return fail("Forensic adapter is not allowlisted; write/format/repair commands are forbidden.");
 const auto id=active_.value("id").toString();const auto current=store_->get(id);
 if(current.isEmpty()||!caseAllowed(current)||current.value("revision")!=active_.value("revision"))return fail("Reopen the current authorized case.");
 const auto source=store_->readEvidence(id,evidence);if(source.isEmpty())return fail(store_->error());
 if(source.size()>8*1024*1024)return fail("Forensic adapters accept at most 8 MiB of protected evidence.");
 const auto digest=QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex());
 forensicCancel_=std::make_shared<std::atomic_bool>(false);
 const mr::forensic::ReadOnlyEvidenceRequest request{id,evidence,digest,adapter,source,forensicCancel_};
 forensicContext_={{"caseId",id},{"caseRevision",active_.value("revision")},{"evidenceId",evidence},{"sourceSha256",digest},{"actor",actor()},{"adapter",adapter}};
 forensicResult_.clear();error_.clear();forensicActive_=true;
 forensic_->setFuture(QtConcurrent::run([request]{const auto r=mr::forensic::analyzeReadOnlyEvidence(request);return r.ok()?r.report:QVariantMap{{"errorCode",r.errorCode}};}));
 emit forensicChanged();return true;
}
