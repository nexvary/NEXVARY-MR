#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

QStringList GovernmentOperationPolicy::capabilities(){
 return {"cloud-publish","tracking-portal","signature-update","device-tools","external-analysis","external-links"};
}
GovernmentOperationPolicy::GovernmentOperationPolicy(IdentityService *identity,QObject *parent,int lifetimeMs)
 :QObject(parent),identity_(identity),lifetimeMs_(qBound(10,lifetimeMs,600000)){
 clock_.start();monitor_.setInterval(qMin(1000,lifetimeMs_));
 connect(&monitor_,&QTimer::timeout,this,&GovernmentOperationPolicy::refresh);
 if(identity_)connect(identity_,&IdentityService::changed,this,&GovernmentOperationPolicy::revokeAll);
}
bool GovernmentOperationPolicy::granted(const QString &capability) const {
 const auto it=grants_.constFind(capability);
 return it!=grants_.cend()&&clock_.elapsed()<it->deadline;
}
bool GovernmentOperationPolicy::offline() const {
 for(const auto &name:capabilities())if(granted(name))return false;
 return true;
}
QVariantList GovernmentOperationPolicy::permissions() const {
 QVariantList rows;
 for(const auto &name:capabilities()){
  const auto g=grants_.value(name);
  rows.append(QVariantMap{{"capability",name},{"allowed",granted(name)},
   {"reason",g.reason},{"actor",g.actor},{"expiresUtc",g.expiresUtc}});
 }
 return rows;
}
void GovernmentOperationPolicy::refresh(){
 if(refreshing_||grants_.isEmpty())return;
 refreshing_=true;
 if(!identity_||!identity_->checkSession()||identity_->session().value("role")!="administrator"){
  revokeAll();refreshing_=false;return;
 }
 bool removed=false;
 for(auto it=grants_.begin();it!=grants_.end();){
  if(clock_.elapsed()>=it->deadline){it=grants_.erase(it);removed=true;}else ++it;
 }
 if(grants_.isEmpty())monitor_.stop();
 refreshing_=false;if(removed)emit changed();
}
bool GovernmentOperationPolicy::permit(const QString &capability){
 refresh();
 if(granted(capability))return true;
 error_=QStringLiteral("Government offline policy blocked %1. A local administrator must explicitly grant this category with a reason; saved settings and workspace navigation do not grant permission.").arg(capability);
 emit errorChanged();
 // Rejection feedback does not emit changed(): it must not recursively run
 // consumer revocation handlers. QML reads error after the rejected action.
 return false;
}
bool GovernmentOperationPolicy::grant(const QString &capability,const QString &reason,bool approved){
 if(!capabilities().contains(capability)||!approved||reason.trimmed().isEmpty()||reason.size()>256||
    !identity_||!identity_->checkSession()||identity_->session().value("role")!="administrator"){
  error_=QStringLiteral("An authenticated local administrator, a known category, a reason (1–256 characters) and explicit approval are required.");emit errorChanged();return false;
 }
 grants_.insert(capability,{reason.trimmed(),identity_->actor(),QDateTime::currentDateTimeUtc().addMSecs(lifetimeMs_).toString(Qt::ISODateWithMs),clock_.elapsed()+lifetimeMs_});
 error_.clear();emit errorChanged();monitor_.start();emit changed();return true;
}
void GovernmentOperationPolicy::revoke(const QString &capability){
 if(grants_.remove(capability)){if(grants_.isEmpty())monitor_.stop();emit changed();}
}
void GovernmentOperationPolicy::revokeAll(){
 if(grants_.isEmpty())return;
 grants_.clear();monitor_.stop();emit changed();
}
bool GovernmentOperationPolicy::openExternal(const QString &text){
 if(!permit("external-links"))return false;
 const QUrl url(text,QUrl::StrictMode);
 if(text.size()>2048||!url.isValid()||url.host().isEmpty()||!url.userInfo().isEmpty()||
    (url.scheme()!="https"&&url.scheme()!="http")){
  error_=QStringLiteral("Only an explicit HTTP/HTTPS link without embedded credentials may be opened.");emit errorChanged();return false;
 }
 if(!QDesktopServices::openUrl(url)){error_=QStringLiteral("The system could not open the selected link.");emit errorChanged();return false;}
 error_.clear();emit errorChanged();return true;
}
