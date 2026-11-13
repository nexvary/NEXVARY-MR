#include "LampyreConnection.hpp"
#include "IdentityService.hpp"
#include "osint/CommercialProviderContract.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "CaseVault.hpp"
#include "RecoveryFile.hpp"
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSaveFile>
#include <QLockFile>
#include <QFileInfo>
#include <QFile>
#include <QRegularExpression>
#include <QDir>
#include <QHostAddress>
#include <QCryptographicHash>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <cmath>
namespace {
constexpr qsizetype responseLimit=2*1024*1024;
const QString officialLookupBase=QStringLiteral("https://account.lampyre.io/api/1.6");
const QString officialEndpoint=QStringLiteral("https://account.lampyre.io/api/v2/mcp");
QVariantMap publicInvoice(const QVariantMap &row){
 QVariantMap customer;for(const auto *field:{"id","operationId","createdUtc","updatedUtc","category","service","currency","quotedMinorUnits","amountDueMinorUnits","status","paymentStatus"})customer.insert(field,row.value(field));return customer;
}

}
LampyreConnection::LampyreConnection(const QString &dir,IdentityService *identity,GovernmentOperationPolicy *policy,QObject *parent,bool syntheticTests,int timeoutMs)
 :QObject(parent),directory_(dir),identity_(identity),policy_(policy),synthetic_(syntheticTests),timeoutMs_(qBound(50,timeoutMs,120000)) {
 timer_.setSingleShot(true);
 connect(&timer_,&QTimer::timeout,this,[this]{fail("timeout");});
 if(identity_)connect(identity_,&IdentityService::changed,this,[this]{
  const auto owner=identity_->authenticated()?identity_->userId():QString();
  if(owner!=owner_){reset();owner_=owner;load();restoreLookupReference();emit changed();}
  else if(busy_ && (!identity_->hasPermission("external-connection") || (stage_=="call"&&!authorized())))fail("policy-revoked");
 });

 owner_=identity_&&identity_->authenticated()?identity_->userId():synthetic_?QStringLiteral("synthetic"):QString();load();restoreLookupReference();
}
bool LampyreConnection::liveProfile()const{return !synthetic_&&identity_&&!identity_->syntheticProfile();}
double LampyreConnection::retailPriceEur()const{return static_cast<double>(mr::osint::intelligenceRetailPriceMinor)/100.0;}
LampyreConnection::~LampyreConnection(){reset();}
bool LampyreConnection::validEndpoint(const QString &text,bool syntheticTests){
 const QUrl u(text,QUrl::StrictMode);
 if(text.size()>2048||!u.isValid()||u.host().isEmpty()||!u.userInfo().isEmpty()||u.hasQuery()||u.hasFragment())return false;
 if(u.scheme()=="https")return syntheticTests||text==officialEndpoint;
 return syntheticTests&&u.scheme()=="http"&&QHostAddress(u.host()).isLoopback();
}
bool LampyreConnection::validKey(const QString &key){
 return key.size()<=4096&&QRegularExpression("^[A-Za-z0-9._~+/-]+=*$").match(key).hasMatch();
}
QString LampyreConnection::credentialPath()const {
 if(owner_.isEmpty()||(!synthetic_&&QUuid(owner_).isNull()))return {};
 return QDir(directory_).filePath("lampyre-"+owner_+".dat");
}
void LampyreConnection::reset(){
 if(!invoiceId_.isEmpty())finishInvoice("review");
 pendingTasks_={};taskRows_={};
 lookupConnected_=false;lookupRequestId_.clear();lookupKind_.clear();lookupPolls_=0;
 invoiceRows_.clear();invoiceId_.clear();invoiceOperationId_.clear();resultInvoiceId_.clear();requestSecrets_.clear();invoicePrepared_=false;
 timer_.stop();if(reply_){auto *r=reply_.data();reply_.clear();r->abort();r->deleteLater();}
 finishReceipt("cancelled");connected_=false;busy_=false;tools_.clear();body_.clear();cursors_.clear();pages_=0;
 categories_.clear();services_.clear();selectedCategory_.clear();catalogCheckedAt_.clear();creditBalance_=-1;serviceCatalogAt_=0;retailQuoted_=false;
 session_.clear();version_.clear();result_.clear();sourceDigest_.clear();resultCaseId_.clear();resultOperationId_.clear();key_.fill(QChar(0));key_.clear();endpoint_.clear();state_="disconnected";error_.clear();saved_=false;requestLimit_=0;submitted_=0;
}
void LampyreConnection::load(){
 loadInvoices();
 const auto path=credentialPath();if(path.isEmpty())return;QFile f(path);const QFileInfo info(f);
 if(!info.exists()){
  if(!identity_||!identity_->directWork())return;
  const auto entries=QDir(directory_).entryList({"lampyre-*.dat"},QDir::Files);
  if(entries.size()!=1)return; // Never choose between different saved keys.
  QFile legacy(QDir(directory_).filePath(entries.first()));QString error;
  if(QFileInfo(legacy).isSymLink()||!legacy.open(QIODevice::ReadOnly)||legacy.size()>32768)return;
  auto plain=mr::recovery::openCaseBytes(legacy.readAll(),synthetic_,error);
  const auto record=QJsonDocument::fromJson(plain).object();plain.fill('\0');
  if(!validKey(record.value("key").toString()))return;
  key_=record.value("key").toString();endpoint_=officialEndpoint;
  if(save()){saved_=true;state_="configured";}else{key_.fill(QChar(0));key_.clear();}
  return;
 }
 if(info.isSymLink()||!info.isFile()||info.size()>32768||!f.open(QIODevice::ReadOnly)){error_="credential-read";return;}
 QString error;auto plain=mr::recovery::openCaseBytes(f.readAll(),synthetic_,error);
 const auto obj=QJsonDocument::fromJson(plain).object();plain.fill('\0');
 if(obj.value("owner").toString()==owner_&&obj.value("revoked").toBool()&&obj.value("key").toString().isEmpty())return;
 if(obj.value("owner").toString()!=owner_||!validEndpoint(obj.value("endpoint").toString(),true)||!validKey(obj.value("key").toString())){error_="credential-read";return;}
 endpoint_=synthetic_?obj.value("endpoint").toString():officialEndpoint;key_=obj.value("key").toString();saved_=true;state_="configured";
}
bool LampyreConnection::save(){
 const auto path=credentialPath();if(path.isEmpty())return fail("sign-in-required");
 auto plain=QJsonDocument(QJsonObject{{"owner",owner_},{"endpoint",endpoint_},{"key",key_},{"revoked",key_.isEmpty()}}).toJson(QJsonDocument::Compact);
 QString error;const auto sealed=mr::recovery::sealCase(plain,synthetic_,error);plain.fill('\0');
 if(sealed.isEmpty())return fail("protected-storage-required");
 if(!QDir().mkpath(directory_)||QFileInfo(path).isSymLink())return fail("credential-write");
 QSaveFile file(path);file.setDirectWriteFallback(false);
 if(!file.open(QIODevice::WriteOnly)||!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(sealed)!=sealed.size()||!file.commit())return fail("credential-write");
 return true;
}
bool LampyreConnection::configureCredential(const QString &key,bool remember){
 return configure(officialEndpoint,key,remember);
}
QString LampyreConnection::displayText(const QString &text)const {
 QString safe=text;
 if(!key_.isEmpty()){safe.replace(key_,"[credential redacted]");safe.replace(QString::fromLatin1(QUrl::toPercentEncoding(key_)),"[credential redacted]",Qt::CaseInsensitive);}
 for(const auto &secret:requestSecrets_)if(!secret.isEmpty())safe.replace(secret,"[credential redacted]");
 if(!endpoint_.isEmpty())safe.replace(endpoint_,"[connection redacted]",Qt::CaseInsensitive);
 safe.replace(officialEndpoint,"[connection redacted]",Qt::CaseInsensitive);
 safe.replace(officialLookupBase,"[connection redacted]",Qt::CaseInsensitive);
 safe.replace(QRegularExpression("https?://[^\\s\"<>]*lampyre\\.io[^\\s\"<>]*",QRegularExpression::CaseInsensitiveOption),"[connection redacted]");
 safe.replace(QRegularExpression("lampyre",QRegularExpression::CaseInsensitiveOption),"[external service redacted]");
 return safe;
}
bool LampyreConnection::configure(const QString &endpoint,const QString &key,bool remember){
 if(busy_)return false;
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 if(!validEndpoint(endpoint,synthetic_))return reject("invalid-endpoint");
 const auto effective=key.isEmpty()&&endpoint==endpoint_?key_:key;
 if(!validKey(effective))return reject("invalid-key");
 // Copy before reset: implicit sharing keeps the retained key alive until assigned.
 const auto nextKey=QString(effective);const auto nextEndpoint=QString(endpoint);
 reset();loadInvoices();endpoint_=nextEndpoint;key_=nextKey;
 if(remember){if(!save()){key_.fill(QChar(0));key_.clear();emit changed();return false;}}
 else if(identity_&&identity_->directWork()){
  const auto retained=key_;key_.clear();const bool ok=save();key_=retained;if(!ok)return false;
 }else {const auto path=credentialPath();if(!path.isEmpty()&&QFile::exists(path)&&!QFile::remove(path))return reject("credential-write");}
 saved_=remember;state_="configured";emit changed();return true;
}
bool LampyreConnection::connectProvider(bool consent){
 if(busy_)return false;
 if(pendingResultCount()>0)return reject("results-pending");
 if(!consent)return reject("connection-consent-required");
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 if(identity_&&!identity_->hasPermission("external-connection"))return reject("local-connection-permission-denied");
 if(!validEndpoint(endpoint_,synthetic_)||!hasCredential())return reject("configuration-required");

 connected_=false;tools_.clear();result_.clear();sourceDigest_.clear();session_.clear();version_.clear();cursors_.clear();pages_=0;error_.clear();busy_=true;state_="connecting";emit changed();
 post("initialize",{{"protocolVersion","2025-06-18"},{"capabilities",QJsonObject{}},{"clientInfo",QJsonObject{{"name","NEXVARY MR"},{"version","0.4.5"}}}},"initialize");return true;
}
QVariantMap LampyreConnection::spendingAllowances()const {
 QVariantMap out;if(!identity_||!identity_->authenticated())return out;
 if(identity_->hasPermission("user-administration"))for(const auto &v:identity_->users()){
  const auto id=v.toMap().value("id").toString();out.insert(id,remainingSubmissions(id));
 }else out.insert(owner_,remainingSubmissions(owner_));
 return out;
}
int LampyreConnection::remainingSubmissions(const QString &accountId)const {
 if(!identity_||!identity_->authenticated()||QUuid(accountId).isNull()||
    (accountId!=owner_&&!identity_->hasPermission("user-administration")))return 0;
 QFile file(QDir(directory_).filePath("intelligence-budget-"+accountId+".dat"));
 if(!QFileInfo::exists(file.fileName()))return -1; // No administrator ceiling configured.
 if(QFileInfo(file).isSymLink()||!file.open(QIODevice::ReadOnly)||file.size()>4096)return 0;
 QString error;auto plain=mr::recovery::openCaseBytes(file.readAll(),synthetic_,error);
 const auto obj=QJsonDocument::fromJson(plain).object();plain.fill('\0');
 if(obj.value("owner").toString()!=accountId)return 0;
 return qBound(0,obj.value("remaining").toInt(),100);
}
bool LampyreConnection::setSpendingLimit(const QString &accountId,int submissions){
 if(!identity_||!identity_->checkSession()||!identity_->hasPermission("user-administration"))return reject("administrator-limit-required");
 bool found=false;for(const auto &v:identity_->users())if(v.toMap().value("id").toString()==accountId)found=true;
 if(!found||submissions<0||submissions>100)return reject("invalid-request-limit");
 if(!QDir().mkpath(directory_))return reject("credential-write");
 const auto path=QDir(directory_).filePath("intelligence-budget-"+accountId+".dat");
 QLockFile lock(path+".lock");if(!lock.tryLock(0)||QFileInfo(path).isSymLink())return reject("budget-storage-error");
 QString error;auto plain=QJsonDocument(QJsonObject{{"owner",accountId},{"remaining",submissions}}).toJson(QJsonDocument::Compact);
 const auto sealed=mr::recovery::sealCase(plain,synthetic_,error);plain.fill('\0');
 QSaveFile file(path);file.setDirectWriteFallback(false);
 if(sealed.isEmpty()||!file.open(QIODevice::WriteOnly)||!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(sealed)!=sealed.size()||!file.commit())return reject("budget-storage-error");
 error_.clear();emit changed();return true;
}
bool LampyreConnection::consumeSubmission(){
 if(!identity_)return synthetic_;
 if(identity_->directWork())return true; // Direct work has no application-account budget.
 const auto path=QDir(directory_).filePath("intelligence-budget-"+owner_+".dat");
 QLockFile lock(path+".lock");if(!lock.tryLock(0))return reject("budget-storage-error");
 const int remaining=remainingSubmissions(owner_);if(remaining==-1)return true;if(remaining<=0)return reject("spending-limit-reached");
 QString error;auto plain=QJsonDocument(QJsonObject{{"owner",owner_},{"remaining",remaining-1}}).toJson(QJsonDocument::Compact);
 const auto sealed=mr::recovery::sealCase(plain,synthetic_,error);plain.fill('\0');
 QSaveFile file(path);file.setDirectWriteFallback(false);
 if(QFileInfo(path).isSymLink()||sealed.isEmpty()||!file.open(QIODevice::WriteOnly)||!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(sealed)!=sealed.size()||!file.commit())return reject("budget-storage-error");
 return true;
}
bool LampyreConnection::setRequestLimit(int limit){
 if(busy_||limit<0||limit>100)return reject("invalid-request-limit");
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 requestLimit_=limit;error_.clear();emit changed();return true;
}
bool LampyreConnection::authorized(){
 return synthetic_&& !identity_ && !policy_ ? true : identity_&&identity_->checkSession()&&identity_->hasPermission("paid-search")&&identity_->hasPermission("osint-investigation")&&nexvary::law::allows(identity_->lawRole(),nexvary::law::Action::AnalyzeEvidence)&&identity_->hasPermission("external-connection");
}
// Only tools explicitly documented by this provider as free are used for
// metadata. Unknown/new tools remain on the explicit paid-consent path.
qint64 LampyreConnection::totalDueMinorUnits()const {
 qint64 total=0;for(const auto &value:invoiceRows_){const auto row=value.toMap();if(row.value("status")=="due"&&row.value("paymentStatus")=="unpaid")total+=row.value("amountDueMinorUnits").toLongLong();}return total;
}
QVariantList LampyreConnection::invoices()const {
 QVariantList result;
 for(auto it=invoiceRows_.crbegin();it!=invoiceRows_.crend()&&result.size()<64;++it){
  result.append(publicInvoice(it->toMap()));
 }
 return result;
}
QJsonObject LampyreConnection::resultInvoice()const {
 if(resultInvoiceId_.isEmpty())return {};
 for(const auto &value:invoiceRows_){const auto row=value.toMap();if(row.value("id")==resultInvoiceId_)return QJsonObject::fromVariantMap(publicInvoice(row));}
 return {};
}
void LampyreConnection::loadInvoices(){
 invoiceRows_.clear();if(owner_.isEmpty())return;
 QFile file(QDir(directory_).filePath("intelligence-invoices-"+owner_+".dat"));if(!file.exists())return;
 if(QFileInfo(file).isSymLink()||!file.open(QIODevice::ReadOnly)||file.size()>2*1024*1024){error_="invoice-storage-error";return;}
 QString error;auto bytes=mr::recovery::openCaseBytes(file.readAll(),synthetic_,error);
 const auto document=QJsonDocument::fromJson(bytes).object();bytes.fill('\0');
 if(document.value("schema")!="nexvary-intelligence-invoices/v1"||document.value("owner").toString()!=owner_||!document.value("rows").isArray()||document.value("rows").toArray().size()>1000){error_="invoice-storage-error";return;}
 QSet<QString> ids;
 for(const auto &value:document.value("rows").toArray()){
  const auto row=value.toObject();const auto id=row.value("id").toString(),status=row.value("status").toString();
  const auto quoted=row.value("quotedMinorUnits"),due=row.value("amountDueMinorUnits");
  if(QUuid(id).isNull()||ids.contains(id)||row.value("currency")!="EUR"||row.value("paymentStatus")!="unpaid"||
     !QStringList{"pending","due","void","review"}.contains(status)||!quoted.isDouble()||!due.isDouble()||
     quoted.toInteger()<1||quoted.toInteger()>100000000||quoted.toDouble()!=static_cast<double>(quoted.toInteger())||
     due.toDouble()!=(status=="due"?quoted.toDouble():0.0)){error_="invoice-storage-error";return;}
  ids.insert(id);
 }
 invoiceRows_=document.value("rows").toArray().toVariantList();
}
bool LampyreConnection::writeInvoices(const QVariantList &rows){
 if(owner_.isEmpty()||rows.size()>1000||!QDir().mkpath(directory_))return reject("invoice-storage-error");
 const auto path=QDir(directory_).filePath("intelligence-invoices-"+owner_+".dat");
 QLockFile lock(path+".lock");if(!lock.tryLock(0)||QFileInfo(path).isSymLink())return reject("invoice-storage-error");
 // Reject concurrent stale writes, including a second instance of the application.
 const auto expected=invoiceRows_;loadInvoices();if(error_=="invoice-storage-error"||QJsonDocument(QJsonArray::fromVariantList(invoiceRows_)).toJson(QJsonDocument::Compact)!=QJsonDocument(QJsonArray::fromVariantList(expected)).toJson(QJsonDocument::Compact))return reject("invoice-storage-error");
 QString error;auto bytes=QJsonDocument(QJsonObject{{"schema","nexvary-intelligence-invoices/v1"},{"owner",owner_},{"rows",QJsonArray::fromVariantList(rows)}}).toJson(QJsonDocument::Compact);
 if(bytes.size()>2*1024*1024){bytes.fill('\0');return reject("invoice-storage-error");}
 const auto sealed=mr::recovery::sealCase(bytes,synthetic_,error);bytes.fill('\0');QSaveFile file(path);file.setDirectWriteFallback(false);
 if(sealed.isEmpty()||!file.open(QIODevice::WriteOnly)||!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(sealed)!=sealed.size()||!file.commit())return reject("invoice-storage-error");
 invoiceRows_=rows;return true;
}
bool LampyreConnection::prepareInvoice(const QVariantMap &service){
 if(error_=="invoice-storage-error")return false;
 const auto id=QUuid::createUuid().toString(QUuid::WithoutBraces),now=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
 QVariantMap row{{"id",id},{"operationId",QString()},{"createdUtc",now},{"updatedUtc",now},{"category",selectedCategory_.left(128)},
  {"service",displayText(service.value("title").toString()).left(256)},{"currency","EUR"},{"quotedMinorUnits",static_cast<qint64>(mr::osint::intelligenceRetailPriceMinor)},
  {"amountDueMinorUnits",0},{"status","pending"},{"paymentStatus","unpaid"},
  {"internalQuotedPhotons",service.value("photons")},{"internalChargedPhotons",QVariant()},
  {"internalChargeVerification","not-reported-by-provider"},{"internalBalanceBefore",creditBalance_}};
 auto rows=invoiceRows_;rows.append(row);if(!writeInvoices(rows))return false;invoiceId_=id;invoiceOperationId_.clear();invoicePrepared_=true;return true;
}
bool LampyreConnection::finishInvoice(const QString &status){
 if(invoiceId_.isEmpty())return true;
 auto rows=invoiceRows_;for(auto &value:rows){auto row=value.toMap();if(row.value("id")!=invoiceId_)continue;
  row.insert("status",status);row.insert("amountDueMinorUnits",status=="due"?static_cast<qint64>(mr::osint::intelligenceRetailPriceMinor):0);
  row.insert("operationId",invoiceOperationId_);row.insert("internalResponseSha256",sourceDigest_);row.insert("updatedUtc",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));value=row;
 }
 const auto id=invoiceId_;invoiceId_.clear();invoicePrepared_=false;if(!writeInvoices(rows)){invoiceId_=id;return false;}invoiceOperationId_.clear();emit changed();return true;
}
bool LampyreConnection::freeMetadataTool(const QString &name)const {
 if(name!="lampyre_balance"&&name!="lampyre_categories_list"&&name!="lampyre_endpoints_from_categories")return false;
 for(const auto &v:tools_)if(v.toMap().value("name")==name)
  return v.toMap().value("description").toString().contains("Free.",Qt::CaseInsensitive);
 return false;
}
bool LampyreConnection::metadataCall(const QString &name,const QJsonObject &args,const QString &stage){
 if(busy_)return false;
 if(!connected_)return reject("not-connected");
 if(identity_&&(!identity_->checkSession()||!identity_->hasPermission("external-connection")))return reject("local-connection-permission-denied");
 if(!freeMetadataTool(name))return reject("metadata-not-free");
 busy_=true;state_="loading-catalog";error_.clear();emit changed();
 post("tools/call",{{"name",name},{"arguments",args}},stage);return true;
}
bool LampyreConnection::refreshCatalog(){
 if(busy_)return false;
 if(pendingResultCount()>0)return reject("results-pending");
 if(!freeMetadataTool("lampyre_categories_list"))return reject("metadata-not-free");
 creditBalance_=-1;categories_.clear();services_.clear();selectedCategory_.clear();catalogCheckedAt_.clear();serviceCatalogAt_=0;
 return metadataCall("lampyre_balance",{},"metadata-balance");
}
bool LampyreConnection::discoverServices(const QString &category){
 if(busy_)return false;
 bool found=false;for(const auto &v:categories_)if(v.toMap().value("name")==category)found=true;
 if(!found)return reject("invalid-category");
 services_.clear();selectedCategory_=category;serviceCatalogAt_=0;
 return metadataCall("lampyre_endpoints_from_categories",{{"categories",QJsonArray{category}}},"metadata-services");
}
bool LampyreConnection::runService(const QString &name,const QString &arguments,double approvedPhotons,bool consent){
 if(busy_)return false;
 if(pendingResultCount()>0)return reject("results-pending");
 if(!connected_)return reject("not-connected");
 if(!authorized())return reject("investigator-required");
 if(!consent)return reject("request-consent-required");
 QVariantMap service;for(const auto &v:services_)if(v.toMap().value("tool_name")==name)service=v.toMap();
 const auto cost=service.value("photons");
 if(service.isEmpty()||(cost.metaType().id()!=QMetaType::Double&&cost.metaType().id()!=QMetaType::LongLong&&cost.metaType().id()!=QMetaType::Int)||!std::isfinite(cost.toDouble())||cost.toDouble()<0||
    !std::isfinite(approvedPhotons)||approvedPhotons!=cost.toDouble()||
    QDateTime::currentMSecsSinceEpoch()-serviceCatalogAt_>5*60*1000)return reject("quote-expired");
 if(creditBalance_>=0&&approvedPhotons>creditBalance_)return reject("payment-required");
 QJsonParseError error;const auto bytes=arguments.toUtf8();const auto doc=QJsonDocument::fromJson(bytes,&error);
 if(bytes.size()>16384||error.error!=QJsonParseError::NoError||!doc.isObject()||doc.object().isEmpty())return reject("invalid-arguments");
 const auto args=doc.object();for(auto it=args.begin();it!=args.end();++it)if(!it.value().isString()||it.value().toString().trimmed().isEmpty())return reject("invalid-arguments");
 const auto specification=service.value("args").toString();QSet<QString> allowed;
 auto matches=QRegularExpression("(?:^|[,;]\\s*)([A-Za-z_][A-Za-z0-9_]*):\\s*(?:string|integer|number|boolean|object|array)").globalMatch(specification);
 while(matches.hasNext())allowed.insert(matches.next().captured(1));
 if(allowed.isEmpty())return reject("unsupported-service-form");
 for(auto it=args.begin();it!=args.end();++it)if(!allowed.contains(it.key())||it.value().toString().size()>8192)return reject("invalid-arguments");
 if(specification.contains("(all required)"))for(const auto &field:allowed)if(!args.contains(field))return reject("invalid-arguments");
 const QJsonObject task{{"tool_name",name},{"args",args}};
 if(!prepareInvoice(service))return false;
 const bool started=callTool("lampyre_run_crawlers",QString::fromUtf8(QJsonDocument(QJsonObject{{"tasks",QJsonArray{task}}}).toJson(QJsonDocument::Compact)),true);
 if(started){invoiceOperationId_=resultOperationId_;resultInvoiceId_=invoiceId_;retailQuoted_=true;creditBalance_=-1;emit changed();}else finishInvoice("void");return started;
}
bool LampyreConnection::callTool(const QString &name,const QString &arguments,bool consent){
 if(busy_)return false;
 if(pendingResultCount()>0)return reject("results-pending");
 if(!consent)return reject("request-consent-required");
 if(name=="lampyre_run_crawlers"&&!synthetic_&&!invoicePrepared_)return reject("retail-invoice-required");
 if(!connected_)return reject("not-connected");
 if(identity_&&(!identity_->checkSession()||!nexvary::law::allows(identity_->lawRole(),nexvary::law::Action::AnalyzeEvidence)))return reject("investigator-required");
 if(identity_&&(!identity_->hasPermission("paid-search")||!identity_->hasPermission("osint-investigation")))return reject("local-search-permission-denied");
 if(!authorized())return reject("external-analysis-approval-required");
 if(!synthetic_ && (!identity_||!identity_->directWork()) && (!caseAuthorization_ || !caseAuthorization_()))return reject("active-case-required");
 if(requestLimit_>0&&submitted_>=requestLimit_)return reject("spending-limit-reached");
 QVariantMap tool;for(const auto &v:tools_)if(v.toMap().value("name").toString()==name)tool=v.toMap();
 QJsonParseError err;const auto bytes=arguments.toUtf8();const auto doc=QJsonDocument::fromJson(bytes,&err);
 if(tool.isEmpty()||bytes.size()>32768||err.error!=QJsonParseError::NoError||!doc.isObject())return reject("invalid-arguments");
 const auto schema=tool.value("inputSchema").toMap();const auto args=doc.object();
 for(const auto &field:schema.value("required").toList())if(!args.contains(field.toString()))return reject("invalid-arguments");
 const auto fields=schema.value("properties").toMap();
 for(auto it=args.begin();it!=args.end();++it){
  if(!fields.contains(it.key())){if(schema.value("additionalProperties").metaType().id()==QMetaType::Bool&&!schema.value("additionalProperties").toBool())return reject("invalid-arguments");continue;}
  const auto type=fields.value(it.key()).toMap().value("type").toString();const auto value=it.value();
  if((type=="string"&&!value.isString())||(type=="boolean"&&!value.isBool())||(type=="object"&&!value.isObject())||(type=="array"&&!value.isArray())||((type=="number"||type=="integer")&&!value.isDouble()))return reject("invalid-arguments");
  if(type=="integer"&&std::floor(value.toDouble())!=value.toDouble())return reject("invalid-arguments");
  const auto allowed=fields.value(it.key()).toMap().value("enum");if(allowed.isValid()&&!allowed.toList().contains(value.toVariant()))return reject("invalid-arguments");
 }
 if(!beginSubmission(name,doc.object()))return false;
 post("tools/call",{{"name",name},{"arguments",doc.object()}},"call");return true;
}
bool LampyreConnection::beginSubmission(const QString &name,const QJsonObject &args){
 if(!authorized())return reject("investigator-required");
 if(requestLimit_>0&&submitted_>=requestLimit_)return reject("spending-limit-reached");
 const auto caseId=caseIdentity_?caseIdentity_():QString();
 if(!synthetic_&&(!identity_||!identity_->directWork())&&caseId.isEmpty()){finishReceipt("failed");return reject("active-case-required");}
 if(policy_){operation_=policy_->beginInvestigation(caseId,"intelligence-mcp");if(operation_.isEmpty())return reject("audit-unavailable");}
 if(!consumeSubmission()){finishReceipt("failed");return false;}
 resultCaseId_=caseId;resultOperationId_=operation_;
 requestSecrets_.clear();
 std::function<void(const QJsonValue &)> redactInputs=[&](const QJsonValue &value){
  if(value.isArray()){for(const auto &item:value.toArray())redactInputs(item);}
  else if(value.isObject()){
   const auto object=value.toObject();for(auto it=object.begin();it!=object.end();++it){
    if(QRegularExpression("api[_-]?key|password|token|secret",QRegularExpression::CaseInsensitiveOption).match(it.key()).hasMatch()&&it.value().isString()&&!it.value().toString().isEmpty())requestSecrets_.append(it.value().toString());
    else redactInputs(it.value());
   }
  }
 };
 redactInputs(args);
 pendingTasks_={};taskRows_={};
 ++submitted_;resultInvoiceId_.clear();invoicePrepared_=false;retailQuoted_=false;resultTool_=name;resultAt_.clear();busy_=true;state_="running";result_.clear();sourceDigest_.clear();error_.clear();emit changed();
 return true;
}
void LampyreConnection::post(const QString &method,const QJsonObject &params,const QString &stage,bool notification){
 stage_=stage;body_.clear();QJsonObject message{{"jsonrpc","2.0"},{"method",method}};
 if(!params.isEmpty())message.insert("params",params);
 if(!notification){pendingId_=++nextId_;message.insert("id",pendingId_);}
 QNetworkRequest req{QUrl(endpoint_)};req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
 req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");req.setRawHeader("Accept","application/json, text/event-stream");req.setRawHeader("Authorization","Bearer "+key_.toUtf8());
 if(!session_.isEmpty())req.setRawHeader("Mcp-Session-Id",session_.toLatin1());
 if(!version_.isEmpty())req.setRawHeader("MCP-Protocol-Version",version_.toLatin1());
 auto *reply=manager_.post(req,QJsonDocument(message).toJson(QJsonDocument::Compact));reply->setReadBufferSize(responseLimit+1);reply_=reply;
 connect(reply,&QNetworkReply::readyRead,this,[this,reply]{read(reply,false);});
 connect(reply,&QNetworkReply::finished,this,[this,reply]{read(reply,true);reply->deleteLater();});
 timer_.start(timeoutMs_);
}
void LampyreConnection::read(QNetworkReply *reply,bool ended){
 if(reply_!=reply)return;
 body_+=reply->readAll();if(body_.size()>responseLimit){fail("response-too-large");return;}
 if(ended&&reply->error()==QNetworkReply::SslHandshakeFailedError){fail("tls-failure");return;}
 const int status=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
 if(status>=300||(ended&&status<200)){
  fail(status==401?"authentication-failed":status==403?"provider-authorization-denied":status>=500?"provider-service-error":status==429?"rate-limited":status==402?"payment-required":status>=300&&status<400?"redirect-refused":status==404?"endpoint-or-session-expired":"network-error");return;
 }
 if(stage_.startsWith("lookup-")){
  if(!ended)return;
  if(reply->error()!=QNetworkReply::NoError){fail("network-error");return;}
  QJsonParseError error;const auto document=body_.isEmpty()?QJsonDocument():QJsonDocument::fromJson(body_,&error);
  if(status!=204&&(error.error!=QJsonParseError::NoError||document.isNull())){fail("invalid-lookup-response");return;}
  timer_.stop();reply_.clear();reply->deleteLater();acceptLookup(status,document);return;
 }
 if(stage_=="initialized"){
  if(ended){if(reply->error()!=QNetworkReply::NoError){fail("network-error");return;}timer_.stop();reply_.clear();post("tools/list",{},"tools");}return;
 }
 const bool sse=reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith("text/event-stream");
 if(!sse&&!ended)return;
 QList<QByteArray> candidates;
 if(sse){auto text=body_;text.replace("\r\n","\n");const auto events=text.split('\n');QByteArray data;
  for(const auto &line:events){if(line.isEmpty()){if(!data.isEmpty()){candidates.append(data);data.clear();}}else if(line.startsWith("data:")){if(!data.isEmpty())data+='\n';data+=line.mid(5).trimmed();}}
 }else candidates.append(body_);
 for(const auto &bytes:candidates){QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
  if(error.error!=QJsonParseError::NoError||!doc.isObject()){if(!sse){fail("invalid-response");return;}continue;}
  const auto obj=doc.object();if(obj.value("jsonrpc")!="2.0"){fail("invalid-response");return;}
  if(obj.value("id").isDouble()&&obj.value("id").toInteger()==pendingId_){accept(reply,obj);return;}
 }
 if(ended)fail("invalid-response");
}
void LampyreConnection::accept(QNetworkReply *reply,const QJsonObject &message){
 const auto stage=stage_;timer_.stop();reply_.clear();
 const auto session=reply->rawHeader("Mcp-Session-Id");reply->abort();reply->deleteLater();
 if(message.contains("error")){fail("provider-rpc-error");return;}
 if(!message.value("result").isObject()){fail("invalid-response");return;}
 const auto result=message.value("result").toObject();
 if(stage=="initialize"){
  version_=result.value("protocolVersion").toString();
  if(version_!="2025-03-26"&&version_!="2025-06-18"&&version_!="2025-11-25"){fail("unsupported-protocol");return;}
  if(session.size()>4096){fail("invalid-session");return;}
  for(const auto ch:session)if(static_cast<unsigned char>(ch)<0x21||static_cast<unsigned char>(ch)>0x7e){fail("invalid-session");return;}
  session_=QString::fromLatin1(session);post("notifications/initialized",{},"initialized",true);return;
 }
 if(stage=="tools"){
  if(!result.value("tools").isArray()){fail("invalid-response");return;}
  for(const auto &v:result.value("tools").toArray()){
   const auto t=v.toObject();const auto name=t.value("name").toString();if(name.isEmpty()||name.size()>256||!t.value("inputSchema").isObject()||tools_.size()>=256){fail("invalid-catalog");return;}
   for(const auto &existing:tools_)if(existing.toMap().value("name").toString()==name){fail("invalid-catalog");return;}
   tools_.append(t.toVariantMap());
  }
  const auto cursor=result.value("nextCursor").toString();
  if(!cursor.isEmpty()){if(++pages_>=10||cursor.size()>4096||cursors_.contains(cursor)){fail("catalog-limit");return;}cursors_.insert(cursor);post("tools/list",{{"cursor",cursor}},"tools");return;}
  finishReceipt("completed");busy_=false;connected_=true;state_="connected";error_.clear();emit changed();return;
 }
 if(stage.startsWith("metadata-")){
  if(result.value("isError").toBool()){fail("metadata-response-error");return;}
  QJsonObject payload=result.value("structuredContent").toObject();
  if(payload.isEmpty())for(const auto &v:result.value("content").toArray()){
   const auto item=v.toObject();if(item.value("type")!="text")continue;
   const auto parsed=QJsonDocument::fromJson(item.value("text").toString().toUtf8());
   if(parsed.isObject()){payload=parsed.object();break;}
  }
  if(stage=="metadata-balance"){
   const auto value=payload.value("photons");
   if(!value.isDouble()||!std::isfinite(value.toDouble())||value.toDouble()<0){fail("metadata-response-error");return;}
   creditBalance_=value.toDouble();busy_=false;
   metadataCall("lampyre_categories_list",{},"metadata-categories");return;
  }
  if(stage=="metadata-categories"){
   const auto rows=payload.value("categories");if(!rows.isArray()||rows.toArray().size()>256){fail("metadata-response-error");return;}
   QVariantList next;QSet<QString> names;
   for(const auto &v:rows.toArray()){
    const auto row=v.toObject();const auto name=row.value("name").toString();
    if(name.isEmpty()||name.size()>256||names.contains(name)){fail("metadata-response-error");return;}
    names.insert(name);next.append(row.toVariantMap());
   }
   categories_=next;catalogCheckedAt_=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
  }else if(stage=="metadata-services"){
   const auto rows=payload.value("crawlers").toObject().value(selectedCategory_);
   if(!rows.isArray()||rows.toArray().size()>512){fail("metadata-response-error");return;}
   QVariantList next;QSet<QString> names;
   for(const auto &v:rows.toArray()){
    const auto row=v.toObject();const auto name=row.value("tool_name").toString();
    if(name.isEmpty()||name.size()>256||names.contains(name)){fail("metadata-response-error");return;}
    names.insert(name);next.append(row.toVariantMap());
   }
   services_=next;serviceCatalogAt_=QDateTime::currentMSecsSinceEpoch();
  }
  busy_=false;state_="connected";error_.clear();emit changed();return;
 }
 if(stage=="call"||stage=="collect"){
  if(!authorized()||(!synthetic_&&(!identity_||!identity_->directWork())&&(!caseAuthorization_||!caseAuthorization_()||!caseIdentity_||caseIdentity_()!=resultCaseId_))){fail("policy-revoked");return;}
  sourceDigest_=QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(result).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
  resultAt_=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);result_=QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Indented));result_=displayText(result_);
  if(stage=="collect"&&result.value("isError").toBool()){fail("result-collection-error");return;}
  if(resultTool_=="lampyre_run_crawlers"&&!result.value("isError").toBool()){acceptTasks(result,stage=="collect");return;}
  const bool failed=result.value("isError").toBool();if(!invoiceId_.isEmpty()&&!finishInvoice(failed?"void":"due")){fail("invoice-storage-error");return;}finishReceipt(failed?"failed":"completed");busy_=false;state_=failed?"provider-tool-error":"completed";emit changed();return;
 }
 fail("invalid-response");
}
double LampyreConnection::lookupPhotons(const QString &kind){
 // Owner-reviewed provider web display (2026-11-14); not a confirmed debit receipt.
 return kind=="phone"?35.0:kind=="email"?30.0:kind=="username"?15.0:-1.0;
}
bool LampyreConnection::verifyLookupConnection(bool consent){
 if(busy_)return false;
 if(!consent)return reject("connection-consent-required");
 if(!hasCredential())return reject("configuration-required");
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 if(identity_&&!identity_->hasPermission("external-connection"))return reject("local-connection-permission-denied");
 lookupConnected_=false;busy_=true;state_="checking-unified-search";error_.clear();emit changed();
 // This history GET is read-only. Never call the side-effecting search GET here.
 getLookup("/find/history/by-token",{},"lookup-check");return true;
}
void LampyreConnection::getLookup(const QString &path,const QJsonObject &parameters,const QString &stage){
 stage_=stage;body_.clear();QUrl url(officialLookupBase+path);
 if(synthetic_&&QHostAddress(QUrl(endpoint_).host()).isLoopback()){
  url=QUrl(endpoint_);url.setPath("/api/1.6"+path);url.setQuery(QString());
 }
 QByteArray query="token="+QUrl::toPercentEncoding(key_);
 for(auto i=parameters.begin();i!=parameters.end();++i)query+='&'+QUrl::toPercentEncoding(i.key())+'='+QUrl::toPercentEncoding(i.value().toString());
 url.setQuery(QString::fromLatin1(query),QUrl::StrictMode);
 QNetworkRequest request(url);request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
 request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);request.setAttribute(QNetworkRequest::CacheSaveControlAttribute,false);
 request.setAttribute(QNetworkRequest::Http2AllowedAttribute,false);request.setRawHeader("Connection","close");request.setRawHeader("Accept","application/json");
 // Token-in-query is required by this published API. Never log the URL.
 auto *reply=manager_.get(request);reply->setReadBufferSize(responseLimit+1);reply_=reply;
 connect(reply,&QNetworkReply::readyRead,this,[this,reply]{read(reply,false);});
 connect(reply,&QNetworkReply::finished,this,[this,reply]{read(reply,true);reply->deleteLater();});timer_.start(timeoutMs_);
}
bool LampyreConnection::runLookup(const QString &kind,const QString &input,bool consent){
 if(busy_)return false;
 if(pendingResultCount()>0)return reject("results-pending");
 if(!consent)return reject("request-consent-required");
 if(!lookupConnected_)return reject("unified-connection-required");
 if(!authorized())return reject("investigator-required");
 if(!synthetic_&&(!identity_||!identity_->directWork())&&(!caseAuthorization_||!caseAuthorization_()))return reject("active-case-required");
 const double cost=lookupPhotons(kind);QString value=input.trimmed();
 if(kind=="phone")value.remove(QRegularExpression("[ ()-]"));
 if(cost<0||value.isEmpty()||value.size()>320||value.contains(QRegularExpression("[\\x00-\\x1f\\x7f]")))return reject("invalid-arguments");
 if(kind=="phone"&&!QRegularExpression("^\\+[1-9][0-9]{6,14}$").match(value).hasMatch())return reject("invalid-arguments");
 if(kind=="email"&&!QRegularExpression("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$").match(value).hasMatch())return reject("invalid-arguments");
 if(kind=="username"&&(value.size()>128||!QRegularExpression("^[\\p{L}\\p{N}_.@-]+$").match(value).hasMatch()))return reject("invalid-arguments");
 if(creditBalance_>=0&&cost>creditBalance_)return reject("payment-required");
 selectedCategory_=kind;
 if(!prepareInvoice({{"title","NEXVARY unified search"},{"photons",cost}}))return false;
 if(!beginSubmission("unified-search",{{kind,value}})){finishInvoice("void");return false;}
 lookupKind_=kind;lookupPolls_=0;invoiceOperationId_=resultOperationId_;resultInvoiceId_=invoiceId_;retailQuoted_=true;
 // Write an intent before the one billable GET; no retry or MCP fallback is allowed.
 if(!saveLookupReference()){fail("invoice-storage-error");return false;}
 creditBalance_=-1;getLookup("/find/by-token",{{kind,value}},"lookup-start");return true;
}
bool LampyreConnection::saveLookupReference(bool complete){
 if(invoiceId_.isEmpty())return false;
 auto rows=invoiceRows_;bool found=false;for(auto &v:rows){auto row=v.toMap();if(row.value("id")!=invoiceId_)continue;
  row.insert("internalTransport","data-lookup");row.insert("internalLookupComplete",complete);row.insert("internalProviderRequestId",lookupRequestId_);row.insert("internalLookupKind",lookupKind_);
  row.insert("internalCaseId",resultCaseId_);row.insert("operationId",resultOperationId_);row.insert("internalPricingBasis","owner-reviewed-web-display-2026-11-14");v=row;found=true;
 }
 return found&&writeInvoices(rows);
}
void LampyreConnection::restoreLookupReference(){
 for(auto it=invoiceRows_.crbegin();it!=invoiceRows_.crend();++it){const auto row=it->toMap();const auto id=row.value("internalProviderRequestId").toString();
  if(row.value("internalLookupComplete").toBool()||row.value("internalTransport")!="data-lookup"||(row.value("status")!="pending"&&row.value("status")!="review")||QUuid(id).isNull())continue;
  lookupRequestId_=id;lookupKind_=row.value("internalLookupKind").toString();selectedCategory_=lookupKind_;invoiceId_=row.value("id").toString();resultInvoiceId_=invoiceId_;
  resultCaseId_=row.value("internalCaseId").toString();resultOperationId_=row.value("operationId").toString();invoiceOperationId_=resultOperationId_;resultTool_="unified-search";retailQuoted_=true;state_="awaiting-results";break;
 }
}
bool LampyreConnection::collectLookup(){
 if(busy_)return false;
 if(!lookupConnected_)return reject("unified-connection-required");
 if(QUuid(lookupRequestId_).isNull())return reject("no-pending-results");
 if(!authorized()||(!synthetic_&&(!identity_||!identity_->directWork())&&(!caseAuthorization_||!caseAuthorization_()||!caseIdentity_||caseIdentity_()!=resultCaseId_)))return reject("policy-revoked");
 if(policy_&&operation_.isEmpty()){operation_=policy_->beginInvestigation(resultCaseId_,"intelligence-result-collection");if(operation_.isEmpty())return reject("audit-unavailable");}
 ++lookupPolls_;busy_=true;state_="collecting-results";error_.clear();emit changed();
 getLookup("/find/"+lookupRequestId_+"/by-token",{},"lookup-result");return true;
}
void LampyreConnection::acceptLookup(int status,const QJsonDocument &document){
 if(stage_=="lookup-check"){
  if(status!=204&&!document.isArray()){fail("invalid-lookup-response");return;}
  // Discard other searches' private history, even in a successful response.
  body_.clear();if(invoiceId_.isEmpty()&&!lookupRequestId_.isEmpty())restoreLookupReference();lookupConnected_=true;busy_=false;state_=lookupRequestId_.isEmpty()?"unified-connected":"awaiting-results";error_.clear();emit changed();return;
 }
 if(stage_=="lookup-start"){
  const auto id=document.object().value("request_uuid").toString();
  if(status!=200||QUuid(id).isNull()){fail("invalid-lookup-response");return;}
  lookupRequestId_=id;
  if(!saveLookupReference()){fail("invoice-storage-error");return;}
  busy_=false;collectLookup();return;
 }
 if(stage_!="lookup-result"){fail("invalid-lookup-response");return;}
 auto result=document.object();const int phase=result.value("status").toInt(-1);
 if(status==204)result={{"status",2},{"result",QJsonArray{}},{"results_count",0}};
 else if(!result.value("status").isDouble()||(phase!=1&&phase!=2&&phase!=3)){fail("invalid-lookup-response");return;}
 if(phase==1&&status!=204){
  busy_=false;state_="awaiting-results";emit changed();
  const auto id=lookupRequestId_;if(lookupPolls_<20)QTimer::singleShot(1500,this,[this,id]{if(lookupConnected_&&!busy_&&lookupRequestId_==id)collectLookup();});return;
 }
 if(phase!=3&&!result.value("result").isArray()){fail("invalid-lookup-response");return;}
 QJsonArray rows;bool locked=false;for(const auto &v:result.value("result").toArray()){
  if(!v.isObject()){fail("invalid-lookup-response");return;}const auto source=v.toObject();
  if(!source.value("source").isString()||(!source.value("data").isObject()&&!source.value("data").isArray())){fail("invalid-lookup-response");return;}
  QJsonObject row{{"tool_name",source.value("source")},{"result",source.value("data")}};
  if(source.value("payment_required").toBool()){row.insert("status","locked");locked=true;}rows.append(row);
 }
 if(rows.isEmpty())rows.append(QJsonObject{{"tool_name","NEXVARY unified search"},{"result",QJsonArray{}}});
 if(phase==3){rows=QJsonArray{QJsonObject{{"tool_name","NEXVARY unified search"},{"error","Remote search failed"}}};}
 const auto bytes=QJsonDocument(result).toJson(QJsonDocument::Compact);sourceDigest_=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
 resultAt_=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
 result_=displayText(QString::fromUtf8(QJsonDocument(QJsonObject{{"lookupResponse",result},{"requestId",lookupRequestId_},{"structuredContent",QJsonObject{{"tasks",rows}}}}).toJson(QJsonDocument::Indented)));
 if(!saveLookupReference(true)){fail("invoice-storage-error");return;}
 const bool failed=phase==3;if(!finishInvoice(failed?"void":locked?"review":"due")){fail("invoice-storage-error");return;}
 lookupRequestId_.clear();finishReceipt(failed?"failed":"completed");busy_=false;state_=failed?"provider-task-error":locked?"provider-payment-required":"completed";error_=failed?"provider-task-error":locked?"provider-payment-required":QString();emit changed();
}
bool LampyreConnection::freeResultCollector()const {
 for(const auto &v:tools_){const auto t=v.toMap();if(t.value("name")=="lampyre_check_result_endpoint"&&t.value("description").toString().contains("Charges nothing",Qt::CaseInsensitive))return true;}
 return false;
}
bool LampyreConnection::collectResults(){
 if(busy_)return false;
 if(!lookupRequestId_.isEmpty())return collectLookup();
 if(!connected_)return reject("not-connected");
 if(pendingTasks_.isEmpty())return reject("no-pending-results");
 if(!authorized()||(!synthetic_&&(!identity_||!identity_->directWork())&&(!caseAuthorization_||!caseAuthorization_()||!caseIdentity_||caseIdentity_()!=resultCaseId_)))return reject("policy-revoked");
 if(!freeResultCollector())return reject("metadata-not-free");
 busy_=true;state_="collecting-results";error_.clear();emit changed();
 // Result collection is documented as free: reuse task IDs, never submit crawlers again.
 post("tools/call",{{"name","lampyre_check_result_endpoint"},{"arguments",QJsonObject{{"tasks",pendingTasks_}}}},"collect");return true;
}
bool LampyreConnection::acceptTasks(const QJsonObject &result,bool collecting){
 QJsonObject payload=result.value("structuredContent").toObject();
 if(payload.isEmpty())for(const auto &v:result.value("content").toArray()){
  const auto item=v.toObject();if(item.value("type")!="text")continue;
  const auto parsed=QJsonDocument::fromJson(item.value("text").toString().toUtf8());if(parsed.isObject()){payload=parsed.object();break;}
 }
 if(!payload.value("tasks").isArray()||payload.value("tasks").toArray().isEmpty()||payload.value("tasks").toArray().size()>256)return fail("invalid-task-response");
 const auto incoming=payload.value("tasks").toArray();QSet<QString> seen;
 for(const auto &value:incoming){
  if(!value.isObject())return fail("invalid-task-response");
  const auto row=value.toObject();const auto name=row.value("tool_name").toString();
  if(name.isEmpty()||name.size()>256||seen.contains(name))return fail("invalid-task-response");seen.insert(name);
  if(collecting){bool known=false;for(const auto &v:pendingTasks_){const auto task=v.toObject();if(task.value("tool_name")==name){known=true;if(row.contains("task_id")&&row.value("task_id")!=task.value("task_id"))return fail("invalid-task-response");}}
   if(!known)return fail("invalid-task-response");}
  if(row.value("status")=="running"){
   const auto id=row.value("task_id").toString();if(id.isEmpty()||id.size()>256)return fail("invalid-task-response");
  }else if(!row.contains("result")&&!row.value("error").isString())return fail("invalid-task-response");
 }
 // The collector must account for every submitted pending task; omissions are not success.
 if(collecting&&incoming.size()!=pendingTasks_.size())return fail("invalid-task-response");
 if(!collecting)taskRows_=incoming;
 else for(const auto &value:incoming){const auto row=value.toObject();for(qsizetype i=0;i<taskRows_.size();++i)if(taskRows_[i].toObject().value("tool_name")==row.value("tool_name"))taskRows_[i]=row;}
 pendingTasks_={};bool failed=false,succeeded=false;
 for(const auto &v:taskRows_){const auto row=v.toObject();if(row.value("status")=="running")pendingTasks_.append(QJsonObject{{"tool_name",row.value("tool_name")},{"task_id",row.value("task_id")}});
  else if(row.contains("error"))failed=true;else succeeded=true;}
 payload.insert("tasks",taskRows_);QJsonObject display=result;display.insert("structuredContent",payload);
 display.insert("content",QJsonArray{QJsonObject{{"type","text"},{"text",QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Indented))}}});
 result_=displayText(QString::fromUtf8(QJsonDocument(display).toJson(QJsonDocument::Indented)));busy_=false;
 if(!pendingTasks_.isEmpty()){
  state_="awaiting-results";emit changed();
  // One free collection attempt after dispatch. A still-running reply stays pending.
  if(!collecting&&freeResultCollector())return collectResults();
  return true;
 }
 const auto status=failed?(succeeded?"review":"void"):"due";
 if(!invoiceId_.isEmpty()&&!finishInvoice(status))return fail("invoice-storage-error");
 finishReceipt(failed?"failed":"completed");state_=failed?"provider-task-error":"completed";error_=failed?"provider-task-error":QString();emit changed();return true;
}
QVariantList LampyreConnection::findings()const {
 const auto doc=QJsonDocument::fromJson(result_.toUtf8()).object();const auto tasks=doc.value("structuredContent").toObject().value("tasks").toArray();QVariantList rows;
 for(const auto &v:tasks){const auto row=v.toObject();const auto data=row.value("result");const bool running=row.value("status")=="running",failed=row.contains("error"),locked=row.value("status")=="locked";
  const bool empty=data.isNull()||(data.isArray()&&data.toArray().isEmpty())||(data.isObject()&&data.toObject().isEmpty());
  QString text;if(!empty&&!running&&!failed&&!locked){if(data.isArray())text=QString::fromUtf8(QJsonDocument(data.toArray()).toJson(QJsonDocument::Indented));else if(data.isObject())text=QString::fromUtf8(QJsonDocument(data.toObject()).toJson(QJsonDocument::Indented));else text=data.toVariant().toString();}
  QString title=row.value("tool_name").toString();for(const auto &s:services_)if(s.toMap().value("tool_name").toString()==title){title=s.toMap().value("title").toString();break;}
  rows.append(QVariantMap{{"service",displayText(title)},{"status",running?"running":failed?"failed":locked?"locked":empty?"empty":"results"},{"data",displayText(text)}});
 }
 return rows;
}
void LampyreConnection::finishReceipt(const QString &status){
 const auto operation=operation_;operation_.clear();
 if(policy_&&!operation.isEmpty())policy_->finishOperation(operation,status,sourceDigest_,result_.isEmpty()?QString():QString::fromLatin1(QCryptographicHash::hash(result_.toUtf8(),QCryptographicHash::Sha256).toHex()));
}
bool LampyreConnection::fail(const QString &code){
 const bool invoiceSaved=invoiceId_.isEmpty()||finishInvoice("review");
 timer_.stop();if(reply_){auto *reply=reply_.data();reply_.clear();reply->abort();reply->deleteLater();}
 pendingTasks_={};taskRows_={};
 lookupConnected_=false;
 finishReceipt("failed");creditBalance_=-1;categories_.clear();services_.clear();serviceCatalogAt_=0;busy_=false;connected_=false;session_.clear();tools_.clear();result_.clear();sourceDigest_.clear();error_=invoiceSaved?code:QStringLiteral("invoice-storage-error");state_="error";emit changed();return false;
}
void LampyreConnection::cancel(){
 const bool invoiceSaved=invoiceId_.isEmpty()||finishInvoice("review");
 timer_.stop();if(reply_){auto *reply=reply_.data();reply_.clear();reply->abort();reply->deleteLater();}
 pendingTasks_={};taskRows_={};
 lookupConnected_=false;
 finishReceipt("cancelled");busy_=false;connected_=false;tools_.clear();session_.clear();result_.clear();sourceDigest_.clear();state_=invoiceSaved?QStringLiteral("cancelled-local"):QStringLiteral("error");if(invoiceSaved)error_.clear();else error_="invoice-storage-error";emit changed();
}
void LampyreConnection::forget(){reset();loadInvoices();if(identity_&&identity_->directWork()){if(!save())return;emit changed();return;}const auto path=credentialPath();if(!path.isEmpty()&&QFile::exists(path)&&!QFile::remove(path)){fail("credential-write");return;}emit changed();}

QVariantList LampyreConnection::resultItems()const {
 QVariantList rows;const auto doc=QJsonDocument::fromJson(result_.toUtf8()).object();
 for(const auto &v:doc.value("content").toArray()){const auto item=v.toObject();if(item.value("type")=="text")rows.append(QVariantMap{{"text",item.value("text").toString()}});}
 if(rows.isEmpty()&&doc.contains("structuredContent"))rows.append(QVariantMap{{"text",QString::fromUtf8(QJsonDocument(doc.value("structuredContent").toObject()).toJson(QJsonDocument::Indented))}});
 return rows;
}

bool LampyreConnection::reject(const QString &code){error_=code;emit changed();return false;}

bool LampyreConnection::exportResult(const QString &input,bool consent){
 if(!consent)return reject("export-consent-required");
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 if(identity_&&!identity_->hasPermission("evidence-export"))return reject("local-export-permission-denied");
 if(result_.isEmpty())return reject("no-result");
 if(!synthetic_&&!identity_->directWork()&&(!identity_->hasPermission("evidence-access")||!caseIdentity_||caseIdentity_()!=resultCaseId_||!caseAuthorization_||!caseAuthorization_()))return reject("active-case-required");
 const auto path=mr::recovery::localFilePath(input);if(path.isEmpty()||QFileInfo(path).exists()||QFileInfo(path).isSymLink())return reject("choose-new-file");
 const QJsonObject envelope{{"invoice",resultInvoice()},{"retailQuote",retailQuoted_?QJsonObject{{"currency","EUR"},{"minorUnits",static_cast<qint64>(mr::osint::intelligenceRetailPriceMinor)},{"unit","search"},{"collectionStatus","not-collected-by-application"}}:QJsonObject{}},{"schema","nexvary-mcp-result/v1"},{"provider","NEXVARY Intelligence"},{"sourceType","external-service"},{"redactedDisplayCopy",true},{"originalResultSha256",sourceDigest_},{"tool",displayText(resultTool_)},{"receivedAtUtc",resultAt_},{"verification","unverified-provider-result"},{"caseEvidence",false},{"result",QJsonDocument::fromJson(result_.toUtf8()).object()}};
 const auto bytes=QJsonDocument(envelope).toJson(QJsonDocument::Indented);QFile file(path);
 if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))return reject("export-write");
 if(!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(bytes)!=bytes.size()||!file.flush()){file.close();QFile::remove(path);return reject("export-write");}
 file.close();error_.clear();emit changed();return true;
}

QString LampyreConnection::attachResult(const QString &device,bool consent){
 if(!consent){reject("attachment-consent-required");return {};}
 if(!identity_||!identity_->checkSession()||!identity_->hasPermission("case-edit")||!identity_->hasPermission("evidence-access")){reject("local-attachment-permission-denied");return {};}
 if(result_.isEmpty()||resultCaseId_.isEmpty()||!caseIdentity_||caseIdentity_()!=resultCaseId_||!caseAuthorization_||!caseAuthorization_()||!attachment_){reject("active-case-required");return {};}
 const QJsonObject envelope{{"invoice",resultInvoice()},{"retailQuote",retailQuoted_?QJsonObject{{"currency","EUR"},{"minorUnits",static_cast<qint64>(mr::osint::intelligenceRetailPriceMinor)},{"unit","search"},{"collectionStatus","not-collected-by-application"}}:QJsonObject{}},{"schema","nexvary-mcp-case-result/v1"},{"provider","NEXVARY Intelligence"},{"sourceType","external-service"},{"redactedDisplayCopy",true},{"originalResultSha256",sourceDigest_},{"caseId",resultCaseId_},{"localOperationId",resultOperationId_},{"tool",displayText(resultTool_)},{"receivedAtUtc",resultAt_},{"verification","unverified-provider-result"},{"result",QJsonDocument::fromJson(result_.toUtf8()).object()}};
 const auto id=attachment_(device,QJsonDocument(envelope).toJson(QJsonDocument::Indented));
 if(id.isEmpty()){reject("case-attachment-failed");return {};}
 error_.clear();emit changed();return id;
}
