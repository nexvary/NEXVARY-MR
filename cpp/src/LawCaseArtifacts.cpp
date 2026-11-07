#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "CaseArtifacts.hpp"
#include <QFutureWatcher>
#include <QtConcurrent>
bool LawCaseBridge::artifactsBusy()const{return artifactsActive_;}
void LawCaseBridge::cancelArtifacts(){if(artifactsCancel_)artifactsCancel_->store(true);artifactsContext_.clear();artifactsResult_.clear();emit artifactsChanged();}
void LawCaseBridge::initArtifacts(){
 artifacts_=std::make_unique<QFutureWatcher<QVariantMap>>();
 connect(this,&LawCaseBridge::changed,this,[this]{
  const auto retained=artifactsContext_.isEmpty()?artifactsResult_:artifactsContext_;
  if(!retained.isEmpty()&&(retained.value("caseId")!=active_.value("id")||retained.value("caseRevision")!=active_.value("revision")))cancelArtifacts();
 });
 connect(artifacts_.get(),&QFutureWatcher<QVariantMap>::finished,this,[this]{
  artifactsActive_=false;const auto context=artifactsContext_;const auto report=artifacts_->result();artifactsContext_.clear();artifactsResult_.clear();
  const auto finish=[this](bool ok){emit artifactsChanged();emit artifactsCompleted(ok);};
  if(context.isEmpty()){finish(false);return;}
  // checkSession can synchronously clear the workspace. Re-evaluate only afterwards.
  if(!permit(nexvary::law::Action::AnalyzeEvidence,false)||active_.value("id")!=context.value("caseId")||actor()!=context.value("actor")){finish(false);return;}
  const auto current=store_->get(context.value("caseId").toString());
  if(current.isEmpty()||!caseAllowed(current)||current.value("revision")!=context.value("caseRevision")||active_.value("revision")!=context.value("caseRevision")){
   fail("Artifacts snapshot changed or its case grant was revoked. Reopen and rebuild.");finish(false);return;
  }
  if(report.contains("errorCode")){fail("Artifacts rejected: "+report.value("errorCode").toString());finish(false);return;}
  artifactsResult_=report;finish(true);
 });
}
bool LawCaseBridge::buildArtifacts(const QString &query){
 if(!permit(nexvary::law::Action::AnalyzeEvidence)||active_.isEmpty()){cancelArtifacts();return false;}
 if(importBusy()||timelineBusy())return fail("Wait for the current import/timeline before building the artifact index.");
 if(artifactsBusy())return fail("An artifact projection is already running.");
 artifactsResult_.clear();emit artifactsChanged();
 if(query.size()>256)return fail("Artifacts search exceeds 256 characters.");
 const auto record=store_->get(active_.value("id").toString());
 if(record.isEmpty()||!caseAllowed(record)||record.value("revision")!=active_.value("revision"))return fail("Reopen the current authorized case before building its artifacts.");
 artifactsContext_={{"caseId",record.value("id")},{"caseRevision",record.value("revision")},{"actor",actor()}};
 artifactsResult_.clear();error_.clear();artifactsCancel_=std::make_shared<std::atomic_bool>(false);const auto cancelled=artifactsCancel_;
 artifactsActive_=true;artifacts_->setFuture(QtConcurrent::run([record,query,cancelled]{return nexvary::law::projectArtifacts(record,query,cancelled);}));
 emit artifactsChanged();return true;
}
