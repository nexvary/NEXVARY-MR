#include "LampyreConnection.hpp"
#include "IdentityService.hpp"
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
#include <QFileInfo>
#include <QFile>
#include <QRegularExpression>
#include <QDir>
#include <QHostAddress>
#include <QCryptographicHash>
#include <QUrl>
#include <QUuid>
#include <cmath>
namespace {constexpr qsizetype limit=2*1024*1024;}
LampyreConnection::LampyreConnection(const QString &dir,IdentityService *identity,GovernmentOperationPolicy *policy,QObject *parent,bool syntheticTests,int timeoutMs)
 :QObject(parent),directory_(dir),identity_(identity),policy_(policy),synthetic_(syntheticTests),timeoutMs_(qBound(50,timeoutMs,120000)) {
 timer_.setSingleShot(true);
 connect(&timer_,&QTimer::timeout,this,[this]{fail("timeout");});
 if(identity_)connect(identity_,&IdentityService::changed,this,[this]{
  const auto owner=identity_->authenticated()?identity_->userId():QString();
  if(owner!=owner_){reset();owner_=owner;load();emit changed();}
 });
 if(policy_)connect(policy_,&GovernmentOperationPolicy::changed,this,[this]{if(!operation_.isEmpty()&&(!policy_->granted("external-analysis")||(stage_=="call"&&!policy_->granted("paid-unknown-cost"))))fail("policy-revoked");});
 owner_=identity_&&identity_->authenticated()?identity_->userId():synthetic_?QStringLiteral("synthetic"):QString();load();
}
LampyreConnection::~LampyreConnection(){reset();}
bool LampyreConnection::validEndpoint(const QString &text,bool syntheticTests){
 const QUrl u(text,QUrl::StrictMode);
 if(text.size()>2048||!u.isValid()||u.host().isEmpty()||!u.userInfo().isEmpty()||u.hasQuery()||u.hasFragment())return false;
 if(u.scheme()=="https")return true;
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
 timer_.stop();if(reply_){auto *r=reply_.data();reply_.clear();r->abort();r->deleteLater();}
 finishReceipt("cancelled");connected_=false;busy_=false;tools_.clear();body_.clear();cursors_.clear();pages_=0;
 session_.clear();version_.clear();result_.clear();key_.fill(QChar(0));key_.clear();endpoint_.clear();state_="disconnected";error_.clear();saved_=false;requestLimit_=0;submitted_=0;
}
void LampyreConnection::load(){
 const auto path=credentialPath();if(path.isEmpty())return;QFile f(path);const QFileInfo info(f);
 if(!info.exists())return;
 if(info.isSymLink()||!info.isFile()||info.size()>32768||!f.open(QIODevice::ReadOnly)){error_="credential-read";return;}
 QString error;auto plain=mr::recovery::openCaseBytes(f.readAll(),synthetic_,error);
 const auto obj=QJsonDocument::fromJson(plain).object();plain.fill('\0');
 if(obj.value("owner").toString()!=owner_||!validEndpoint(obj.value("endpoint").toString(),synthetic_)||!validKey(obj.value("key").toString())){error_="credential-read";return;}
 endpoint_=obj.value("endpoint").toString();key_=obj.value("key").toString();saved_=true;state_="configured";
}
bool LampyreConnection::save(){
 const auto path=credentialPath();if(path.isEmpty())return fail("sign-in-required");
 auto plain=QJsonDocument(QJsonObject{{"owner",owner_},{"endpoint",endpoint_},{"key",key_}}).toJson(QJsonDocument::Compact);
 QString error;const auto sealed=mr::recovery::sealCase(plain,synthetic_,error);plain.fill('\0');
 if(sealed.isEmpty())return fail("protected-storage-required");
 if(!QDir().mkpath(directory_)||QFileInfo(path).isSymLink())return fail("credential-write");
 QSaveFile file(path);file.setDirectWriteFallback(false);
 if(!file.open(QIODevice::WriteOnly)||!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(sealed)!=sealed.size()||!file.commit())return fail("credential-write");
 return true;
}
bool LampyreConnection::configure(const QString &endpoint,const QString &key,bool remember){
 if(busy_)return false;
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 if(!validEndpoint(endpoint,synthetic_))return reject("invalid-endpoint");
 const auto effective=key.isEmpty()&&endpoint==endpoint_?key_:key;
 if(!validKey(effective))return reject("invalid-key");
 // Copy before reset: implicit sharing keeps the retained key alive until assigned.
 const auto nextKey=QString(effective);const auto nextEndpoint=QString(endpoint);
 reset();endpoint_=nextEndpoint;key_=nextKey;
 if(remember){if(!save()){key_.fill(QChar(0));key_.clear();emit changed();return false;}}
 else {const auto path=credentialPath();if(!path.isEmpty()&&QFile::exists(path)&&!QFile::remove(path))return reject("credential-write");}
 saved_=remember;state_="configured";emit changed();return true;
}
bool LampyreConnection::connectProvider(bool consent){
 if(busy_)return false;
 if(!consent)return reject("connection-consent-required");
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 if(identity_&&!identity_->hasPermission("external-connection"))return reject("local-connection-permission-denied");
 if(!validEndpoint(endpoint_,synthetic_)||!hasCredential())return reject("configuration-required");
 if((!synthetic_&&!policy_)||(policy_&&!policy_->granted("external-analysis")))return reject("external-analysis-approval-required");
 if(policy_){operation_=policy_->beginOperation("external-analysis","lampyre-mcp");if(operation_.isEmpty())return reject("external-analysis-approval-required");}
 connected_=false;tools_.clear();result_.clear();session_.clear();version_.clear();cursors_.clear();pages_=0;error_.clear();busy_=true;state_="connecting";emit changed();
 post("initialize",{{"protocolVersion","2025-06-18"},{"capabilities",QJsonObject{}},{"clientInfo",QJsonObject{{"name","NEXVARY MR"},{"version","0.4.2"}}}},"initialize");return true;
}
bool LampyreConnection::setRequestLimit(int limit){
 if(busy_||limit<0||limit>100)return reject("invalid-request-limit");
 if(!synthetic_&&(!identity_||!identity_->checkSession()))return reject("sign-in-required");
 requestLimit_=limit;error_.clear();emit changed();return true;
}
bool LampyreConnection::authorized(){
 return synthetic_&& !identity_ && !policy_ ? true : identity_&&identity_->checkSession()&&identity_->hasPermission("paid-search")&&identity_->hasPermission("osint-investigation")&&nexvary::law::allows(identity_->lawRole(),nexvary::law::Action::AnalyzeEvidence)&&policy_&&policy_->granted("external-analysis");
}
bool LampyreConnection::callTool(const QString &name,const QString &arguments,bool consent){
 if(busy_)return false;
 if(!consent)return reject("request-consent-required");
 if(!connected_)return reject("not-connected");
 if(identity_&&(!identity_->checkSession()||!nexvary::law::allows(identity_->lawRole(),nexvary::law::Action::AnalyzeEvidence)))return reject("investigator-required");
 if(identity_&&(!identity_->hasPermission("paid-search")||!identity_->hasPermission("osint-investigation")))return reject("local-search-permission-denied");
 if(!authorized())return reject("external-analysis-approval-required");
 if(!synthetic_ && (!caseAuthorization_ || !caseAuthorization_()))return reject("active-case-required");
 if(policy_&&!policy_->granted("paid-unknown-cost"))return reject("unknown-cost-policy-required");
 if((!synthetic_||policy_) && (requestLimit_<=0||submitted_>=requestLimit_))return reject("spending-limit-reached");
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
 if(policy_){operation_=policy_->beginOperation("external-analysis","lampyre-mcp");if(operation_.isEmpty())return reject("external-analysis-approval-required");}
 ++submitted_;resultTool_=name;resultAt_.clear();busy_=true;state_="running";result_.clear();error_.clear();emit changed();
 post("tools/call",{{"name",name},{"arguments",doc.object()}},"call");return true;
}
void LampyreConnection::post(const QString &method,const QJsonObject &params,const QString &stage,bool notification){
 stage_=stage;body_.clear();QJsonObject message{{"jsonrpc","2.0"},{"method",method}};
 if(!params.isEmpty())message.insert("params",params);
 if(!notification){pendingId_=++nextId_;message.insert("id",pendingId_);}
 QNetworkRequest req{QUrl(endpoint_)};req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
 req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");req.setRawHeader("Accept","application/json, text/event-stream");req.setRawHeader("Authorization","Bearer "+key_.toUtf8());
 if(!session_.isEmpty())req.setRawHeader("Mcp-Session-Id",session_.toLatin1());
 if(!version_.isEmpty())req.setRawHeader("MCP-Protocol-Version",version_.toLatin1());
 auto *reply=manager_.post(req,QJsonDocument(message).toJson(QJsonDocument::Compact));reply->setReadBufferSize(limit+1);reply_=reply;
 connect(reply,&QNetworkReply::readyRead,this,[this,reply]{read(reply,false);});
 connect(reply,&QNetworkReply::finished,this,[this,reply]{read(reply,true);reply->deleteLater();});
 timer_.start(timeoutMs_);
}
void LampyreConnection::read(QNetworkReply *reply,bool ended){
 if(reply_!=reply)return;
 body_+=reply->readAll();if(body_.size()>limit){fail("response-too-large");return;}
 if(ended&&reply->error()==QNetworkReply::SslHandshakeFailedError){fail("tls-failure");return;}
 const int status=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
 if(status>=300||(ended&&status<200)){
  fail(status==401?"authentication-failed":status==403?"provider-authorization-denied":status>=500?"provider-service-error":status==429?"rate-limited":status==402?"payment-required":status>=300&&status<400?"redirect-refused":status==404?"endpoint-or-session-expired":"network-error");return;
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
 if(stage=="call"){
  if(!authorized()||(policy_&&!policy_->granted("paid-unknown-cost"))||(!synthetic_&&(!caseAuthorization_||!caseAuthorization_()))){fail("policy-revoked");return;}
  resultAt_=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);result_=QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Indented));result_.replace(key_,"[credential redacted]");
  const bool failed=result.value("isError").toBool();finishReceipt(failed?"failed":"completed");busy_=false;state_=failed?"provider-tool-error":"completed";emit changed();return;
 }
 fail("invalid-response");
}
void LampyreConnection::finishReceipt(const QString &status){
 const auto operation=operation_;operation_.clear();
 if(policy_&&!operation.isEmpty())policy_->finishOperation(operation,status,{},result_.isEmpty()?QString():QString::fromLatin1(QCryptographicHash::hash(result_.toUtf8(),QCryptographicHash::Sha256).toHex()));
}
bool LampyreConnection::fail(const QString &code){
 timer_.stop();if(reply_){auto *reply=reply_.data();reply_.clear();reply->abort();reply->deleteLater();}
 finishReceipt("failed");busy_=false;connected_=false;session_.clear();tools_.clear();result_.clear();error_=code;state_="error";emit changed();return false;
}
void LampyreConnection::cancel(){
 timer_.stop();if(reply_){auto *reply=reply_.data();reply_.clear();reply->abort();reply->deleteLater();}
 finishReceipt("cancelled");busy_=false;connected_=false;tools_.clear();session_.clear();result_.clear();state_="cancelled-local";error_.clear();emit changed();
}
void LampyreConnection::forget(){reset();const auto path=credentialPath();if(!path.isEmpty()&&QFile::exists(path)&&!QFile::remove(path)){fail("credential-write");return;}emit changed();}

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
 if(result_.isEmpty())return reject("no-result");
 const auto path=mr::recovery::localFilePath(input);if(path.isEmpty()||QFileInfo(path).exists()||QFileInfo(path).isSymLink())return reject("choose-new-file");
 const QJsonObject envelope{{"schema","nexvary-mcp-result/v1"},{"provider","Lampyre MCP (user-configured endpoint)"},{"endpointHost",QUrl(endpoint_).host()},{"tool",resultTool_},{"receivedAtUtc",resultAt_},{"verification","unverified-provider-result"},{"caseEvidence",false},{"result",QJsonDocument::fromJson(result_.toUtf8()).object()}};
 const auto bytes=QJsonDocument(envelope).toJson(QJsonDocument::Indented);QFile file(path);
 if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))return reject("export-write");
 if(!file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||file.write(bytes)!=bytes.size()||!file.flush()){file.close();QFile::remove(path);return reject("export-write");}
 file.close();error_.clear();emit changed();return true;
}
