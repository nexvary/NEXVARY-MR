#include "LawCaseBridge.hpp"
#include "LawCaseStore.hpp"
#include "CaseTimeline.hpp"
#include <QFutureWatcher>
#include <QtConcurrent>
bool LawCaseBridge::timelineBusy()const{return timelineActive_;}
void LawCaseBridge::cancelTimeline(){if(timelineCancel_)timelineCancel_->store(true);timelineContext_.clear();timelineResult_.clear();emit timelineChanged();}
void LawCaseBridge::initTimeline(){
 timeline_=std::make_unique<QFutureWatcher<QVariantMap>>();
 connect(this,&LawCaseBridge::changed,this,[this]{
  const auto retained=timelineContext_.isEmpty()?timelineResult_:timelineContext_;
  if(!retained.isEmpty()&&(retained.value("caseId")!=active_.value("id")||retained.value("caseRevision")!=active_.value("revision")))cancelTimeline();
 });
 connect(timeline_.get(),&QFutureWatcher<QVariantMap>::finished,this,[this]{
  timelineActive_=false;const auto context=timelineContext_;const auto report=timeline_->result();timelineContext_.clear();timelineResult_.clear();
  const auto finish=[this](bool ok){emit timelineChanged();emit timelineCompleted(ok);};
  if(context.isEmpty()){finish(false);return;}
  // checkSession can synchronously clear the workspace. Re-evaluate only afterwards.
  if(!permit(nexvary::law::Action::AnalyzeEvidence,false)||active_.value("id")!=context.value("caseId")||actor()!=context.value("actor")){finish(false);return;}
  const auto current=store_->get(context.value("caseId").toString());
  if(current.isEmpty()||!caseAllowed(current)||current.value("revision")!=context.value("caseRevision")||active_.value("revision")!=context.value("caseRevision")){
   fail("Timeline snapshot changed or its case grant was revoked. Reopen and rebuild.");finish(false);return;
  }
  if(report.contains("errorCode")){fail("Timeline rejected: "+report.value("errorCode").toString());finish(false);return;}
  timelineResult_=report;finish(true);
 });
}
bool LawCaseBridge::buildTimeline(const QString &query){
 if(!permit(nexvary::law::Action::AnalyzeEvidence)||active_.isEmpty()){cancelTimeline();return false;}
 if(timelineBusy())return fail("A timeline projection is already running.");
 timelineResult_.clear();emit timelineChanged();
 if(query.size()>256)return fail("Timeline search exceeds 256 characters.");
 const auto record=store_->get(active_.value("id").toString());
 if(record.isEmpty()||!caseAllowed(record)||record.value("revision")!=active_.value("revision"))return fail("Reopen the current authorized case before building its timeline.");
 timelineContext_={{"caseId",record.value("id")},{"caseRevision",record.value("revision")},{"actor",actor()}};
 timelineResult_.clear();error_.clear();timelineCancel_=std::make_shared<std::atomic_bool>(false);const auto cancelled=timelineCancel_;
 timelineActive_=true;timeline_->setFuture(QtConcurrent::run([record,query,cancelled]{return nexvary::law::projectTimeline(record,query,cancelled);}));
 emit timelineChanged();return true;
}
