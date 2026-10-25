#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "EvidenceImportRunner.hpp"
#include "ImportAdapters.hpp"
#include "nexvary/LawEnforcementCase.hpp"

bool LawCaseBridge::importBusy()const{return importer_&&importer_->busy();}
void LawCaseBridge::resetImport(){cancelTimeline();if(importer_)importer_->cancel();importContext_.clear();importResult_.clear();emit importChanged();}
void LawCaseBridge::cancelImport(){if(importer_)importer_->cancel();}
void LawCaseBridge::initImports(){
 importer_=std::make_unique<EvidenceImportRunner>();
 connect(importer_.get(),&EvidenceImportRunner::changed,this,&LawCaseBridge::importChanged);
 connect(importer_.get(),&EvidenceImportRunner::completed,this,[this](QVariantMap report,const QString &code){
  const auto context=importContext_;importContext_.clear();importResult_.clear();
  const auto abandon=[this]{emit importChanged();emit importCompleted(false);};
  // checkSession can synchronously clear the workspace through identity signals.
  // Re-evaluate the captured target only after that authorization refresh.
  if(context.isEmpty()||!permit(nexvary::law::Action::AnalyzeEvidence,false)||active_.value("id")!=context.value("caseId")||actor()!=context.value("actor")||!caseAllowed(store_->get(context.value("caseId").toString()))){abandon();return;}
  const auto id=context.value("caseId").toString(),evidence=context.value("evidenceId").toString();
  // Recheck protected bytes and authority after the untrusted child has exited.
  if(!store_->verifyEvidence(id,evidence)){fail(store_->error());abandon();return;}
  if(report.isEmpty())report={{"schema","nexvary-authorized-import/v1"},{"adapter",context.value("adapter")},{"parserVersion",1},{"status","rejected"},{"sourceSha256",context.value("sourceSha256")},{"sourceSize",context.value("sourceSize")},{"errorCode",code.isEmpty()?QString("parser-rejected"):code},{"findings",QVariantList()}};
  report.insert("provenance",QVariantMap{{"caseId",id},{"evidenceId",evidence},{"deviceId",context.value("deviceId")},{"sourceName",context.value("sourceName")},{"purpose",context.value("purpose")},{"workingCopy","private temporary copy; source SHA-256 checked before and after processing"},{"authorization","operator consent plus current native role/case grant; not independent legal approval"}});
  const auto result=store_->recordAnalysis(id,context.value("revision").toLongLong(),evidence,"authorized-import/"+context.value("adapter").toString(),report,actor());
  if(result.isEmpty()){fail(store_->error());abandon();return;}
  if(!refresh(id)){abandon();return;}
  importResult_=result;if(!code.isEmpty())error_="Import rejected safely: "+code;
  emit importChanged();emit importCompleted(code.isEmpty());
 });
}
bool LawCaseBridge::importEvidence(const QString &evidence,const QString &adapter,const QString &purpose,bool consent){
 if(!consent)return fail("Confirm authorized import, private plaintext working copy and temporary cleanup limits.");
 if(!permit(nexvary::law::Action::AnalyzeEvidence)||active_.isEmpty())return false;
 if(importBusy())return fail("An evidence import is already running.");
 if(active_.value("status")=="suspended")return fail("Resume the suspended case before importing derived findings.");
 if(!mr::forensic::importAdapterSupported(adapter)||purpose.toUtf8().size()>512||!nexvary::law_enforcement::hasText(purpose.toUtf8().toStdString()))return fail("Choose an implemented adapter and a bounded import purpose.");
 QVariantMap item;for(const auto &v:active_.value("evidence").toList())if(v.toMap().value("id")==evidence)item=v.toMap();
 if(item.isEmpty())return fail("Choose evidence from the active case.");
 if(item.value("size").toLongLong()>mr::forensic::importInputLimit)return fail("Authorized import accepts sources up to 4 MiB; the registered original is retained.");
 const auto bytes=store_->readEvidence(active_.value("id").toString(),evidence);if(bytes.isEmpty())return fail(store_->error());
 importResult_.clear();error_.clear();
 importContext_={{"caseId",active_.value("id")},{"revision",active_.value("revision")},{"actor",actor()},{"evidenceId",evidence},{"deviceId",item.value("deviceId")},{"sourceName",item.value("name")},{"sourceSha256",item.value("sha256")},{"sourceSize",item.value("size")},{"adapter",adapter},{"purpose",purpose.trimmed()}};
 if(!importer_->start(bytes,item.value("sha256").toString(),adapter)){importContext_.clear();return fail("Private working copy or verified importer could not be prepared.");}
 emit importChanged();return true;
}
