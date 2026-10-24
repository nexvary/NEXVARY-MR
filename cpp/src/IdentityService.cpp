#include "IdentityService.hpp"
#include "CaseVault.hpp"
#include "RecoveryFile.hpp"
#include <sodium.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QSaveFile>
#include <QLockFile>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>
#include <QSet>
#include <QTimer>
#include <QUuid>
namespace {
QByteArray json(const QVariantMap &v){return QJsonDocument(QJsonObject::fromVariantMap(v)).toJson(QJsonDocument::Compact);}
QString utc(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString b64(const QByteArray &v){return QString::fromLatin1(v.toBase64());}
QByteArray un64(const QVariant &v){return QByteArray::fromBase64(v.toString().toLatin1(),QByteArray::AbortOnBase64DecodingErrors);}
QStringList roles(){return {"administrator","investigator","reviewer","auditor","trainer","repair-technician"};}
bool passwordValid(const QString &s){return s.size()>=14&&s.toUtf8().size()<=256&&!s.contains(QChar(0));}
QVariantMap publicAccount(QVariantMap a){for(const auto *key:{"passwordHash","salt","nonce","secret","failed","blockedUntil"})a.remove(key);return a;}
QByteArray signedBytes(const QVariantMap &m){return QByteArray("NEXVARY-MR/local-attestation/v1\0",32)+json(m);}
}
IdentityService::IdentityService(const QString &path,bool synthetic,QObject *parent):QObject(parent),path_(mr::recovery::localFilePath(path)),synthetic_(synthetic){
 if(sodium_init()<0||path_.isEmpty()||(!synthetic_&&!mr::recovery::vaultAvailable())){fail("Local identities require Windows protected storage (synthetic tests excepted).");return;}
 ready_=read();
 auto *timer=new QTimer(this);timer->setInterval(1000);connect(timer,&QTimer::timeout,this,[this]{if(authenticated())checkSession();});timer->start();
}
IdentityService::~IdentityService(){if(secret_)sodium_free(secret_);}
bool IdentityService::fail(const QString &m){error_=m;emit changed();return false;}
bool IdentityService::read(){
 if(!QFile::exists(path_)){if(!state_.isEmpty()&&state_.value("revision").toLongLong()>0)return fail("Identity registry disappeared; session denied.");state_={{"schema",1},{"revision",0},{"users",QVariantList()},{"grants",QVariantMap()}};
 return true;}
 QFile f(path_);if(QFileInfo(path_).isSymLink()||!mr::recovery::openRegularInput(f)||f.size()>2*1024*1024)return fail("Invalid identity registry.");
 QString error;
 auto bytes=mr::recovery::openCaseBytes(f.readAll(),synthetic_,error);if(bytes.isEmpty())return fail("Cannot open protected identity registry.");
 QJsonParseError parse;
 auto doc=QJsonDocument::fromJson(bytes,&parse);sodium_memzero(bytes.data(),static_cast<size_t>(bytes.size()));
 auto next=doc.object().toVariantMap();
 if(parse.error!=QJsonParseError::NoError||!doc.isObject()||next.value("schema").toInt()!=1||next.value("revision").toLongLong()<1||next.value("users").metaType().id()!=QMetaType::QVariantList||next.value("grants").metaType().id()!=QMetaType::QVariantMap)return fail("Identity registry schema rejected.");
 auto users=next.value("users").toList();if(users.isEmpty()||users.size()>128)return fail("Invalid identity registry size.");QSet<QString> names,ids;bool admin=false;
 for(const auto &v:users){const auto a=v.toMap();
 const auto id=a.value("id").toString(),name=a.value("username").toString();
 const auto encoded=a.value("passwordHash").toString().toLatin1();
  if(QUuid(id).isNull()||ids.contains(id)||names.contains(name)||!QRegularExpression("^[a-z0-9][a-z0-9._-]{2,63}$").match(name).hasMatch()||!roles().contains(a.value("role").toString())||a.value("enabled").metaType().id()!=QMetaType::Bool||a.value("displayName").toString().isEmpty()||a.value("displayName").toString().size()>128||encoded.size()>=crypto_pwhash_STRBYTES||crypto_pwhash_str_needs_rehash(encoded.constData(),crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE)!=0||un64(a.value("salt")).size()!=crypto_pwhash_SALTBYTES||un64(a.value("nonce")).size()!=crypto_secretbox_NONCEBYTES||un64(a.value("secret")).size()!=crypto_sign_SECRETKEYBYTES+crypto_secretbox_MACBYTES||un64(a.value("publicKey")).size()!=crypto_sign_PUBLICKEYBYTES)return fail("Identity account validation failed.");
  ids.insert(id);names.insert(name);if(a.value("role")=="administrator"&&a.value("enabled").toBool())admin=true;
 }
 if(!admin)return fail("An enabled local administrator is required.");
 const auto grants=next.value("grants").toMap();if(grants.size()>1000)return fail("Case grant limit exceeded.");
 for(auto it=grants.cbegin();it!=grants.cend();++it){if(QUuid(it.key()).isNull()||(it.value().metaType().id()!=QMetaType::QStringList&&it.value().metaType().id()!=QMetaType::QVariantList))return fail("Invalid case grant.");for(const auto &id:it.value().toStringList())if(!ids.contains(id))return fail("Unknown grant principal.");}
 state_=next;
 return true;
}
bool IdentityService::save(QVariantMap next){
 if(!QDir().mkpath(QFileInfo(path_).absolutePath()))return fail("Cannot create identity directory.");
 QLockFile lock(path_+".lock");if(!lock.tryLock(0))return fail("Identity registry busy; retry.");
 const auto expected=state_.value("revision").toLongLong();if(!read()||state_.value("revision").toLongLong()!=expected)return fail("Identity registry changed; retry with current accounts.");
 next.insert("revision",expected+1);
 auto bytes=json(next);
 QString error;
 auto sealed=mr::recovery::sealCase(bytes,synthetic_,error);sodium_memzero(bytes.data(),static_cast<size_t>(bytes.size()));if(sealed.isEmpty()||sealed.size()>2*1024*1024)return fail("Identity protection failed or registry exceeds limit.");
 QSaveFile f(path_);f.setDirectWriteFallback(false);if(!f.open(QIODevice::WriteOnly)||!f.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)||f.write(sealed)!=sealed.size()||!f.commit())return fail("Cannot save identity registry atomically.");state_=next;error_.clear();emit changed();
 return true;
}
QVariantMap IdentityService::newAccount(const QString &username,const QString &name,const QString &role,const QString &password){
 if(!QRegularExpression("^[a-z0-9][a-z0-9._-]{2,63}$").match(username).hasMatch()||name.trimmed().isEmpty()||name.size()>128||!roles().contains(role)||!passwordValid(password)){fail("Use a 3–64 character lowercase username, display name and password of at least 14 characters (max 256 UTF-8 bytes).");return {};}
 for(const auto c:name)if(c.unicode()<32||(c.unicode()>=0x202a&&c.unicode()<=0x202e)||(c.unicode()>=0x2066&&c.unicode()<=0x2069)){fail("Invalid display name controls.");return {};}
 for(const auto &v:state_.value("users").toList())if(v.toMap().value("username")==username){fail("Username already exists.");return {};}
 auto pass=password.toUtf8();char encoded[crypto_pwhash_STRBYTES];unsigned char salt[crypto_pwhash_SALTBYTES],nonce[crypto_secretbox_NONCEBYTES],key[crypto_secretbox_KEYBYTES],pk[crypto_sign_PUBLICKEYBYTES],sk[crypto_sign_SECRETKEYBYTES];
 randombytes_buf(salt,sizeof salt);randombytes_buf(nonce,sizeof nonce);
 const bool ok=crypto_pwhash_str_alg(encoded,pass.constData(),static_cast<unsigned long long>(pass.size()),crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE,crypto_pwhash_ALG_ARGON2ID13)==0&&crypto_pwhash(key,sizeof key,pass.constData(),static_cast<unsigned long long>(pass.size()),salt,crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE,crypto_pwhash_ALG_ARGON2ID13)==0;
 sodium_memzero(pass.data(),static_cast<size_t>(pass.size()));if(!ok){sodium_memzero(key,sizeof key);fail("Password derivation failed.");return {};}
 crypto_sign_keypair(pk,sk);QByteArray cipher(crypto_sign_SECRETKEYBYTES+crypto_secretbox_MACBYTES,Qt::Uninitialized);crypto_secretbox_easy(reinterpret_cast<unsigned char*>(cipher.data()),sk,sizeof sk,nonce,key);sodium_memzero(key,sizeof key);sodium_memzero(sk,sizeof sk);
 return {{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"username",username},{"displayName",name},{"role",role},{"enabled",true},{"version",1},{"passwordHash",QString::fromLatin1(encoded)},{"salt",b64(QByteArray(reinterpret_cast<const char*>(salt),sizeof salt))},{"nonce",b64(QByteArray(reinterpret_cast<const char*>(nonce),sizeof nonce))},{"secret",b64(cipher)},{"publicKey",b64(QByteArray(reinterpret_cast<const char*>(pk),sizeof pk))},{"failed",0},{"blockedUntil",0}};
}
bool IdentityService::bootstrap(const QString &username,const QString &name,const QString &password){if(!ready_||!read()||!needsSetup())return fail("Local administrator setup is already complete or unavailable.");
 auto a=newAccount(username,name,"administrator",password);if(a.isEmpty())return false;
 auto next=state_;next.insert("users",QVariantList{a});return save(next);}
QVariantMap IdentityService::account(const QString &id)const{for(const auto &v:state_.value("users").toList())if(v.toMap().value("id")==id)return v.toMap();return {};}
QVariantMap IdentityService::session()const{if(!authenticated())return {};
 auto a=publicAccount(account(userId_));a.insert("identityScope","local-credential; not agency-verified");return a;}
QVariantList IdentityService::users()const{QVariantList out;if(!authenticated()||account(userId_).value("role")!="administrator")return out;for(const auto &v:state_.value("users").toList())out.append(publicAccount(v.toMap()));return out;}
void IdentityService::logout(){if(secret_){sodium_free(secret_);secret_=nullptr;}userId_.clear();sessionAccount_.clear();lifetime_.invalidate();emit changed();}
bool IdentityService::login(const QString &username,const QString &password){
 logout();if(!ready_||!passwordValid(password)||!read())return fail("Sign-in failed.");
 auto rows=state_.value("users").toList();qsizetype index=-1;for(qsizetype i=0;i<rows.size();++i)if(rows[i].toMap().value("username")==username)index=i;
 if(index<0)return fail("Sign-in failed.");
 auto a=rows[index].toMap();
 const auto now=QDateTime::currentSecsSinceEpoch();if(!a.value("enabled").toBool()||a.value("blockedUntil").toLongLong()>now)return fail("Sign-in failed or account temporarily locked.");
 auto pass=password.toUtf8();
 const auto encoded=a.value("passwordHash").toString().toLatin1();
 const bool valid=crypto_pwhash_str_verify(encoded.constData(),pass.constData(),static_cast<unsigned long long>(pass.size()))==0;
 if(!valid){sodium_memzero(pass.data(),static_cast<size_t>(pass.size()));
 const int failed=a.value("failed").toInt()+1;a.insert("failed",failed>=5?0:failed);if(failed>=5)a.insert("blockedUntil",now+300);rows[index]=a;
 auto next=state_;next.insert("users",rows);save(next);return fail("Sign-in failed.");}
 auto salt=un64(a.value("salt")),nonce=un64(a.value("nonce")),cipher=un64(a.value("secret"));unsigned char key[crypto_secretbox_KEYBYTES];
 const bool derived=crypto_pwhash(key,sizeof key,pass.constData(),static_cast<unsigned long long>(pass.size()),reinterpret_cast<const unsigned char*>(salt.constData()),crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE,crypto_pwhash_ALG_ARGON2ID13)==0;sodium_memzero(pass.data(),static_cast<size_t>(pass.size()));
 auto *secret=static_cast<unsigned char*>(sodium_malloc(crypto_sign_SECRETKEYBYTES));if(!derived||!secret||crypto_secretbox_open_easy(secret,reinterpret_cast<const unsigned char*>(cipher.constData()),static_cast<unsigned long long>(cipher.size()),reinterpret_cast<const unsigned char*>(nonce.constData()),key)!=0){sodium_memzero(key,sizeof key);if(secret)sodium_free(secret);return fail("Signing credential could not be opened.");}sodium_memzero(key,sizeof key);
 unsigned char pk[crypto_sign_PUBLICKEYBYTES];crypto_sign_ed25519_sk_to_pk(pk,secret);if(QByteArray(reinterpret_cast<const char*>(pk),sizeof pk)!=un64(a.value("publicKey"))){sodium_free(secret);return fail("Signing credential mismatch.");}
 a.insert("failed",0);a.insert("blockedUntil",0);rows[index]=a;
 auto next=state_;next.insert("users",rows);if(!save(next)){sodium_free(secret);return false;}secret_=secret;sessionAccount_=a;userId_=a.value("id").toString();lifetime_.start();error_.clear();emit changed();
 return true;
}
bool IdentityService::changePassword(const QString &currentPassword,const QString &newPassword){
 if(!checkSession()||!passwordValid(newPassword))return fail("Sign in and choose a new password of at least 14 characters (max 256 UTF-8 bytes).");
 const auto username=account(userId_).value("username").toString();if(!login(username,currentPassword))return false;
 auto pass=newPassword.toUtf8();char encoded[crypto_pwhash_STRBYTES];unsigned char salt[crypto_pwhash_SALTBYTES],nonce[crypto_secretbox_NONCEBYTES],key[crypto_secretbox_KEYBYTES];randombytes_buf(salt,sizeof salt);randombytes_buf(nonce,sizeof nonce);
 const bool ok=crypto_pwhash_str_alg(encoded,pass.constData(),static_cast<unsigned long long>(pass.size()),crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE,crypto_pwhash_ALG_ARGON2ID13)==0&&crypto_pwhash(key,sizeof key,pass.constData(),static_cast<unsigned long long>(pass.size()),salt,crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE,crypto_pwhash_ALG_ARGON2ID13)==0;
 sodium_memzero(pass.data(),static_cast<size_t>(pass.size()));if(!ok){sodium_memzero(key,sizeof key);return fail("Password derivation failed.");}
 QByteArray cipher(crypto_sign_SECRETKEYBYTES+crypto_secretbox_MACBYTES,Qt::Uninitialized);crypto_secretbox_easy(reinterpret_cast<unsigned char*>(cipher.data()),secret_,crypto_sign_SECRETKEYBYTES,nonce,key);sodium_memzero(key,sizeof key);
 auto rows=state_.value("users").toList();for(auto &v:rows){auto a=v.toMap();if(a.value("id")==userId_){a.insert("passwordHash",QString::fromLatin1(encoded));a.insert("salt",b64(QByteArray(reinterpret_cast<const char*>(salt),sizeof salt)));a.insert("nonce",b64(QByteArray(reinterpret_cast<const char*>(nonce),sizeof nonce)));a.insert("secret",b64(cipher));a.insert("version",a.value("version").toInt()+1);v=a;}}
 auto next=state_;next.insert("users",rows);const bool saved=save(next);logout();return saved;
}
bool IdentityService::checkSession(){
 if(!authenticated())return false;
 const auto before=sessionAccount_;
 const auto oldGrants=state_.value("grants");if(!lifetime_.isValid()||lifetime_.elapsed()>15*60*1000||!read()){logout();return false;}const auto current=account(userId_);if(!current.value("enabled").toBool()||current.value("version")!=before.value("version")||current.value("publicKey")!=before.value("publicKey")||current.value("role")!=before.value("role")){logout();return false;}if(oldGrants!=state_.value("grants"))emit changed();
 return true;
}
bool IdentityService::administrator(){return checkSession()&&account(userId_).value("role")=="administrator";}
QString IdentityService::createUser(const QString &username,const QString &name,const QString &role,const QString &password){if(!administrator()) {fail("An authenticated local administrator is required.");return {};}
 auto rows=state_.value("users").toList();if(rows.size()>=128){fail("Account limit reached.");return {};}const auto a=newAccount(username,name,role,password);if(a.isEmpty())return {};rows.append(a);
 auto next=state_;next.insert("users",rows);return save(next)?a.value("id").toString():QString();}
bool IdentityService::setEnabled(const QString &id,bool enabled){if(!administrator()||id==userId_)return fail("Administrator cannot disable their own session.");
 auto rows=state_.value("users").toList();bool found=false;for(auto &v:rows){auto a=v.toMap();if(a.value("id")==id){a.insert("enabled",enabled);a.insert("version",a.value("version").toInt()+1);v=a;found=true;}}if(!found)return fail("Unknown local account.");
 auto next=state_;next.insert("users",rows);return save(next);}
bool IdentityService::setCaseGrant(const QString &caseId,const QString &id,bool granted){if(!administrator()||QUuid(caseId).isNull()||account(id).isEmpty())return fail("Administrator, case UUID and existing account are required.");
 auto grants=state_.value("grants").toMap();if(!grants.contains(caseId)&&grants.size()>=1000)return fail("Case grant limit reached.");
 auto principals=grants.value(caseId).toStringList();principals.removeAll(id);if(granted)principals.append(id);if(principals.isEmpty())grants.remove(caseId);else grants.insert(caseId,principals);
 auto next=state_;next.insert("grants",grants);return save(next);}
bool IdentityService::hasCaseGrant(const QString &id)const{return authenticated()&&state_.value("grants").toMap().value(id).toStringList().contains(userId_);}
nexvary::law::Role IdentityService::lawRole()const{using R=nexvary::law::Role;if(!authenticated())return R::Unknown;
 const auto r=account(userId_).value("role").toString();if(r=="investigator")return R::Investigator;if(r=="reviewer")return R::Reviewer;if(r=="auditor")return R::Auditor;if(r=="trainer")return R::Trainer;if(r=="repair-technician")return R::RepairTechnician;return R::Unknown;}
QString IdentityService::actor()const{return authenticated()?userId_+" / "+account(userId_).value("username").toString():QString();}
QVariantMap IdentityService::attest(const QString &purpose,const QVariantMap &payload){if(!checkSession()||!secret_||purpose.size()>64||json(payload).size()>1024*1024)return {};
 QVariantMap e{{"schema",1},{"purpose",purpose},{"payload",payload},{"signerId",userId_},{"role",account(userId_).value("role")},{"publicKey",account(userId_).value("publicKey")},{"signedAtUtc",utc()}};
 const auto message=signedBytes(e);unsigned char signature[crypto_sign_BYTES];crypto_sign_detached(signature,nullptr,reinterpret_cast<const unsigned char*>(message.constData()),static_cast<unsigned long long>(message.size()),secret_);e.insert("signature",b64(QByteArray(reinterpret_cast<const char*>(signature),sizeof signature)));return e;}
bool IdentityService::verify(const QVariantMap &envelope,const QString &purpose,const QVariantMap &payload){if(!ready_||!read())return false;auto e=envelope;
 const auto sig=un64(e.take("signature"));
 const auto a=account(e.value("signerId").toString());
 const auto pk=un64(a.value("publicKey"));if(e.value("schema").toInt()!=1||e.value("purpose")!=purpose||e.value("payload").toMap()!=payload||a.isEmpty()||!a.value("enabled").toBool()||e.value("publicKey")!=a.value("publicKey")||e.value("role")!=a.value("role")||sig.size()!=crypto_sign_BYTES||pk.size()!=crypto_sign_PUBLICKEYBYTES||json(e).size()>1024*1024)return false;
 const auto bytes=signedBytes(e);return crypto_sign_verify_detached(reinterpret_cast<const unsigned char*>(sig.constData()),reinterpret_cast<const unsigned char*>(bytes.constData()),static_cast<unsigned long long>(bytes.size()),reinterpret_cast<const unsigned char*>(pk.constData()))==0;}
