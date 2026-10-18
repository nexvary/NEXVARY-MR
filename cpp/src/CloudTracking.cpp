#include "CloudTracking.hpp"
#include "WorkshopBridge.hpp"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QRegularExpression>
#include <QDateTime>
#include <QGuiApplication>
#include <QClipboard>
#include <memory>
#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif
namespace {
QByteArray protect(const QByteArray &bytes,bool encrypt){
#ifdef Q_OS_WIN
 DATA_BLOB in{static_cast<DWORD>(bytes.size()),reinterpret_cast<BYTE *>(const_cast<char *>(bytes.constData()))},out{};
 const bool ok=encrypt?CryptProtectData(&in,L"NEXVARY tracking credentials",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out):CryptUnprotectData(&in,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out);
 if(!ok)return {};
 const QByteArray result(reinterpret_cast<const char *>(out.pbData),static_cast<qsizetype>(out.cbData));SecureZeroMemory(out.pbData,out.cbData);LocalFree(out.pbData);return result;
#else
 Q_UNUSED(encrypt);return bytes; // Linux test/development: owner-only config, Windows uses current-user DPAPI.
#endif
}
}
CloudTracking::CloudTracking(WorkshopBridge *w,const QString &storage,bool localTest,QObject *parent):QObject(parent),workshop_(w),storage_(storage),network_(this),localTest_(localTest){
 periodic_.setInterval(30000);debounce_.setInterval(750);debounce_.setSingleShot(true);
 connect(&periodic_,&QTimer::timeout,this,&CloudTracking::syncNow);connect(&debounce_,&QTimer::timeout,this,&CloudTracking::syncNow);
 connect(w,&WorkshopBridge::changed,this,[this]{++generation_;dirty_=true;emit changed();if(configured())debounce_.start();});
 QFile file(storage_);if(file.size()<=32768&&file.open(QIODevice::ReadOnly)){const auto data=file.readAll();if(data.startsWith("MRREMOTE1\n"))applyCredentials(protect(data.mid(10),false),false);}
 if(configured()){periodic_.start();debounce_.start();}
}
CloudTracking::~CloudTracking(){if(reply_){reply_->disconnect(this);reply_->abort();}}
void CloudTracking::fail(const QString &message){busy_=false;dirty_=true;error_=message;emit changed();}
bool CloudTracking::applyCredentials(const QByteArray &bytes,bool save){
 const auto doc=QJsonDocument::fromJson(bytes);const auto o=doc.object();const QUrl url(o.value("serverUrl").toString());
 static const QRegularExpression hex64(QStringLiteral("^[a-f0-9]{64}$")),hex32(QStringLiteral("^[a-f0-9]{32}$"));
 const bool test=localTest_&&url.scheme()==QStringLiteral("http")&&url.host()==QStringLiteral("127.0.0.1");
 if(!doc.isObject()||o.size()!=4||o.value("schemaVersion").toInt()!=1||(!test&&url.scheme()!=QStringLiteral("https"))||url.host().isEmpty()||!url.userInfo().isEmpty()||!url.query().isEmpty()||!url.fragment().isEmpty()||(!url.path().isEmpty()&&url.path()!=QStringLiteral("/"))||!hex64.match(o.value("apiKey").toString()).hasMatch()||!hex32.match(o.value("sourceId").toString()).hasMatch()){
  error_=QStringLiteral("Invalid server credentials. Use the installer's credentials JSON and a valid HTTPS origin.");emit changed();return false;
 }
 if(save){const auto cipher=protect(bytes,true);QSaveFile f(storage_);if(cipher.isEmpty()||!f.open(QIODevice::WriteOnly)||f.write("MRREMOTE1\n"+cipher)!=cipher.size()+10||!f.commit()){error_=QStringLiteral("Could not protect/save server credentials.");emit changed();return false;}QFile::setPermissions(storage_,QFileDevice::ReadOwner|QFileDevice::WriteOwner);}
 url_=url.adjusted(QUrl::StripTrailingSlash).toString();key_=o.value("apiKey").toString();source_=o.value("sourceId").toString();dirty_=true;lastSync_.clear();error_.clear();return true;
}
bool CloudTracking::importCredentials(const QString &path,bool approved){
 if(busy_)return false;
 QFile file(WorkshopBridge::localPath(path));if(!approved||!QFileInfo(file).isFile()||QFileInfo(file).isSymLink()||file.size()>16384||!file.open(QIODevice::ReadOnly)){error_=QStringLiteral("Choose the server credentials file and explicitly approve uploading public tracking fields only.");emit changed();return false;}
 if(!applyCredentials(file.readAll(),true)){return false;}
 periodic_.start();emit changed();debounce_.start();return true;
}
bool CloudTracking::syncNow(){
 if(!configured()||busy_||!workshop_->ready())return false;
 busy_=true;error_.clear();emit changed();request(QStringLiteral("/api/v1/state"));return true;
}
void CloudTracking::request(const QString &endpoint,const QByteArray &body,qint64 revision){
 QNetworkRequest req{QUrl(url_+endpoint)};req.setRawHeader("Authorization","Bearer "+key_.toLatin1());req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);req.setTransferTimeout(12000);
 QNetworkReply *reply=nullptr;if(body.isEmpty())reply=network_.get(req);else{req.setHeader(QNetworkRequest::ContentTypeHeader,QStringLiteral("application/json"));reply=network_.post(req,body);}reply_=reply;reply->setReadBufferSize(16384);
 const auto captured=std::make_shared<QByteArray>();
 connect(reply,&QNetworkReply::readyRead,this,[reply,captured]{captured->append(reply->read(qMax<qsizetype>(0,8192-captured->size())+1));if(captured->size()>8192)reply->abort();});
 connect(reply,&QNetworkReply::finished,this,[this,reply,captured,revision]{
  captured->append(reply->read(qMax<qsizetype>(0,8193-captured->size())));const int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();const bool ok=reply->error()==QNetworkReply::NoError&&code==200&&captured->size()<=8192;const auto doc=QJsonDocument::fromJson(*captured);reply->deleteLater();reply_.clear();
  if(!ok||!doc.isObject()){fail(QStringLiteral("Server sync failed (%1). Remote links may still show the last published state; no TLS/redirect bypass was attempted.").arg(code));return;}
  const auto o=doc.object();
  if(revision<0){
   const auto v=o.value("revision");const double n=v.toDouble(-1);if(o.value("schemaVersion").toInt()!=1||o.value("sourceId").toString()!=source_||!v.isDouble()||n<0||n>9007199254740000.0||n!=static_cast<double>(static_cast<qint64>(n))){fail(QStringLiteral("Server publisher identity or revision is incompatible."));return;}
   const auto records=workshop_->publicTrackingSnapshot();sentGeneration_=generation_;
   const auto payload=QJsonDocument(QJsonObject{{"schemaVersion",1},{"sourceId",source_},{"expectedRevision",n},{"records",QJsonArray::fromVariantList(records)}}).toJson(QJsonDocument::Compact);
   if(records.size()>2000||payload.size()>4*1024*1024){fail(QStringLiteral("Public tracking snapshot exceeds 2000 active links or 4 MiB."));return;}
   request(QStringLiteral("/api/v1/snapshot"),payload,static_cast<qint64>(n));return;
  }
  if(o.value("revision").toDouble(-1)!=static_cast<double>(revision+1)){fail(QStringLiteral("Server did not acknowledge the expected revision."));return;}
  busy_=false;dirty_=sentGeneration_!=generation_;lastSync_=QDateTime::currentDateTimeUtc().toString(Qt::ISODate);error_.clear();emit changed();if(dirty_)debounce_.start();
 });
}
QString CloudTracking::issueLink(int id,int days,bool approved){
 if(!configured()){error_=QStringLiteral("Import server credentials first.");emit changed();return {};}
 const auto token=workshop_->issueTrackingToken(id,days,approved);if(token.isEmpty()){error_=workshop_->error();emit changed();return {};}debounce_.start();return url_+QStringLiteral("/t/")+token;
}
bool CloudTracking::copyLink(const QString &url){if(!inSync()||!url.startsWith(url_+QStringLiteral("/t/")))return false;QGuiApplication::clipboard()->setText(url);return true;}
bool CloudTracking::disconnectServer(bool confirmed){
 if(!confirmed||busy_)return false;
 if(QFileInfo::exists(storage_)&&!QFile::remove(storage_)){error_=QStringLiteral("Could not remove saved server credentials.");emit changed();return false;}
 periodic_.stop();debounce_.stop();url_.clear();key_.fill(QChar::Null);key_.clear();source_.clear();lastSync_.clear();dirty_=true;error_.clear();emit changed();return true;
}
