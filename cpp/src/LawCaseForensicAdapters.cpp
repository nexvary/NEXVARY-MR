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
  const auto payloads=context.value("manifestPayloads").toMap();
  for(auto it=payloads.cbegin();it!=payloads.cend();++it){
   const auto binding=it.value().toMap();const auto bytes=store_->readEvidence(id,binding.value("evidenceId").toString());
   if(bytes.isEmpty()||QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())!=binding.value("sha256")){
    fail("Forensic result refused: protected auxiliary evidence changed.");finish(false);return;
   }
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
 return startForensicEvidence(evidence,adapter,{},false);
}
bool LawCaseBridge::analyzeBrowserSqliteFamily(const QString &main,const QString &wal,const QString &shm){
 QVariantMap roles;if(!wal.isEmpty())roles.insert("wal",wal);if(!shm.isEmpty())roles.insert("shm",shm);
 return startForensicEvidence(main,"browser-sqlite-family",roles,false);
}
bool LawCaseBridge::verifyManifestEvidence(const QString &evidence,const QVariantMap &payloads){
 return startForensicEvidence(evidence,"workbench-export-manifest",payloads,true);
}
bool LawCaseBridge::startForensicEvidence(const QString &evidence,const QString &adapter,const QVariantMap &payloads,bool verifyPayloads){
 if(!permit(nexvary::law::Action::AnalyzeEvidence)||active_.isEmpty()){cancelForensicAnalysis();return false;}
 if(forensicActive_)return fail("A forensic adapter is already running.");
 if(!mr::forensic::forensicAdapters().contains(adapter))return fail("Forensic adapter is not allowlisted; write/format/repair commands are forbidden.");
 const auto id=active_.value("id").toString();const auto current=store_->get(id);
 if(current.isEmpty()||!caseAllowed(current)||current.value("revision")!=active_.value("revision"))return fail("Reopen the current authorized case.");
 const auto source=store_->readEvidence(id,evidence);if(source.isEmpty())return fail(store_->error());
 if(source.size()>8*1024*1024)return fail("Forensic adapters accept at most 8 MiB of protected evidence.");
 const auto digest=QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex());
 if(payloads.size()>128)return fail("Manifest payload count exceeds 128.");
 QMap<QString,mr::forensic::ManifestEvidencePayload> protectedPayloads;QVariantMap bindings;qint64 total=adapter=="browser-sqlite-family"?source.size():0;
 for(auto it=payloads.cbegin();it!=payloads.cend();++it){
  if(!mr::forensic::safeManifestPath(it.key())||it.value().metaType().id()!=QMetaType::QString)return fail("Invalid manifest path/evidence binding.");
  const auto payloadId=it.value().toString();
  // Check declared size before materializing a protected blob. readEvidence also
  // verifies stored size/hash and exact case membership.
  qint64 size=-1;
  for(const auto &entry:current.value("evidence").toList())if(entry.toMap().value("id")==payloadId)size=entry.toMap().value("size").toLongLong();
  if(size<=0||size>8*1024*1024-total)return fail("Manifest payload is unavailable or exceeds the aggregate 8 MiB limit.");
  total+=size;
  const auto bytes=store_->readEvidence(id,payloadId);if(bytes.isEmpty())return fail(store_->error());
  const auto payloadHash=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
  protectedPayloads.insert(it.key(),{payloadId,payloadHash,bytes});
  bindings.insert(it.key(),QVariantMap{{"evidenceId",payloadId},{"sha256",payloadHash}});
 }
 forensicCancel_=std::make_shared<std::atomic_bool>(false);
 mr::forensic::ReadOnlyEvidenceRequest request{id,evidence,digest,adapter,source,forensicCancel_};
 request.verifyManifestPayloads=verifyPayloads;if(adapter=="browser-sqlite-family")request.sqliteFamilyMembers=protectedPayloads;else request.manifestPayloads=protectedPayloads;
 forensicContext_={{"caseId",id},{"caseRevision",active_.value("revision")},{"evidenceId",evidence},{"sourceSha256",digest},{"actor",actor()},{"adapter",adapter},{"manifestPayloads",bindings}};
 forensicResult_.clear();error_.clear();forensicActive_=true;
 forensic_->setFuture(QtConcurrent::run([request]{const auto r=mr::forensic::analyzeReadOnlyEvidence(request);return r.ok()?r.report:QVariantMap{{"errorCode",r.errorCode}};}));
 emit forensicChanged();return true;
}
