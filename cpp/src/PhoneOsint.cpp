#include "PhoneOsint.hpp"
#include "OwnedProcess.hpp"
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QDateTime>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTimer>
#include <QThread>
#include <QUuid>
#include <QRegularExpression>
namespace mr::osint {
namespace {
QString utc(){return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);}
QString hash(const QByteArray &b){return QString::fromLatin1(QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex());}
struct Response {int status=0;QByteArray body,proof;QString error;};
Response request(QNetworkAccessManager &manager,quint16 port,const QString &path,const QByteArray &token,const QByteArray &body,const Cancellation &cancel,int timeout=10000,const QByteArray &challenge={}) {
 QNetworkRequest req{QUrl(QStringLiteral("http://127.0.0.1:%1").arg(port)+path)};
 req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);
 req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
 if(!token.isEmpty())req.setRawHeader("Authorization","Bearer "+token);
 if(!challenge.isEmpty())req.setRawHeader("X-Nexvary-Challenge",challenge);
 auto *reply=body.isEmpty()?manager.get(req):manager.post(req,body);
 reply->setReadBufferSize(262145);
 Response out;QEventLoop loop;QTimer deadline,poll;deadline.setSingleShot(true);poll.setInterval(25);
 const auto abort=[&](const QString &why){out.error=why;reply->abort();loop.quit();};
 QObject::connect(&deadline,&QTimer::timeout,&loop,[&]{abort("timeout");});
 QObject::connect(&poll,&QTimer::timeout,&loop,[&]{if(cancel->load())abort("cancelled");});
 QObject::connect(reply,&QNetworkReply::readyRead,&loop,[&]{out.body+=reply->readAll();if(out.body.size()>262144)abort("response-too-large");});
 QObject::connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
 deadline.start(timeout);poll.start();if(!reply->isFinished())loop.exec();
 if(reply->isOpen()){out.body+=reply->readAll();}
 out.status=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
 out.proof=reply->rawHeader("X-Nexvary-Proof");
 if(out.body.size()>262144){out.body.clear();out.error="response-too-large";}
 if(out.error.isEmpty() && reply->error()!=QNetworkReply::NoError)out.error="http-or-transport-failure";
 delete reply;return out;
}
}
QString normalize(const QString &input) {
 if(input.size()>64)return {};
 QString out;
 for(const auto c:input){const auto u=c.unicode();
  if(u>='0'&&u<='9')out+=c;
  else if(u>=0x660&&u<=0x669)out+=QChar('0'+u-0x660);
  else if(u>=0x6f0&&u<=0x6f9)out+=QChar('0'+u-0x6f0);
  else if(c=='+'&&out.isEmpty())out+=c;
  else if(c==' '||c=='-'||c=='('||c==')')continue;
  else return {};
 }
 if(out.startsWith("00"))out="+"+out.mid(2);
 return QRegularExpression("^\\+[1-9][0-9]{6,14}$").match(out).hasMatch()?out:QString();
}
QStringList supportedScanners(){return {"local","googlesearch","ovh","numverify","googlecse"};}
QVariantMap classifyResponse(const QString &scanner,int status,const QByteArray &body,const QString &transportError) {
 const auto document=QJsonDocument::fromJson(body);const auto object=document.object();
 QString error=transportError;
 if(body.size()>262144||error=="response-too-large")error="response-too-large";
 else if(status==404)error="scanner-unavailable";
 else if(!error.isEmpty()){}
 else if(status<200||status>=300)error="scanner-http-failure";
 else if(!document.isObject()||!object.contains("result")||object.value("result").isNull()||object.value("success")==false||object.contains("error"))error="invalid-or-failed-response";
 const bool ok=error.isEmpty();
 const QVariantMap origins{{"local","https://github.com/nyaruka/phonenumbers"},{"googlesearch","https://www.google.com/search"},{"ovh","https://api.ovh.com/1.0/telephony/number/detailedZones"},{"numverify","https://numverify.com/"},{"googlecse","https://customsearch.googleapis.com/customsearch/v1"}};
 return {{"scanner",scanner},{"source",origins.value(scanner,"https://github.com/sundowndev/phoneinfoga")},{"adapterSource","https://github.com/sundowndev/phoneinfoga"},{"providerError",object.value("error").toString().left(1024)},{"retrievedUtc",utc()},
  {"status",ok?"completed":error=="scanner-unavailable"?"unavailable":"failed"},{"error",error},
  {"classification",ok?(scanner=="googlesearch"?"generated-search-links":scanner=="local"?"numbering-plan-metadata":"unverified-public-source"):"no-finding"},
  {"httpStatus",status},{"responseSha256",hash(body)},
  {"rawResponseBase64",body.size()<=262144?QString::fromLatin1(body.toBase64()):QString()},
  {"result",ok?object.value("result").toVariant():QVariant()},
  {"verifiedIdentity",false},{"liveLocation",false},{"telecomRecord",false}};
}
QVariantMap PhoneInfoga::run(const Request &input,const Cancellation &cancel) {
 const auto failure=[](const QString &why){return QVariantMap{{"error",why}};};
 if(!cancel||cancel->load())return failure("cancelled");
 const auto number=normalize(input.number);
 if(number.isEmpty()||input.scanners.isEmpty()||input.scanners.size()>5)return failure("invalid-single-number-request");
 auto unique=input.scanners;unique.removeDuplicates();
 if(unique.size()!=input.scanners.size())return failure("duplicate-scanner");
 for(const auto &scanner:unique)if(!supportedScanners().contains(scanner))return failure("unsupported-scanner");
 QFile executable(input.executable);const QFileInfo info(executable);
 if(!info.isAbsolute()||info.isSymLink()||!info.isFile()||info.size()>128*1024*1024||!executable.open(QIODevice::ReadOnly))return failure("trusted-engine-required");
 const auto engineBytes=executable.read(128*1024*1024+1);
 if(engineBytes.size()!=info.size()||!QRegularExpression("^[0-9a-f]{64}$").match(input.executableSha256).hasMatch()||hash(engineBytes)!=input.executableSha256)return failure("engine-digest-mismatch");
 executable.close();
 QTemporaryDir cwd;if(!cwd.isValid())return failure("private-workspace-unavailable");
 if(!QFile::setPermissions(cwd.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner))return failure("private-workspace-permissions");
 QFile staged(cwd.filePath(
#ifdef Q_OS_WIN
 "engine.exe"
#else
 "engine"
#endif
 ));
 if(!staged.open(QIODevice::WriteOnly|QIODevice::NewOnly)||staged.write(engineBytes)!=engineBytes.size()||!staged.flush()||!staged.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner))return failure("engine-staging-failed");
 staged.close();
 QTcpServer reservation;if(!reservation.listen(QHostAddress::LocalHost,0))return failure("loopback-port-unavailable");
 const auto port=reservation.serverPort();
 const auto token=(QUuid::createUuid().toRfc4122()+QUuid::createUuid().toRfc4122()).toHex();
 QProcessEnvironment env;
 for(const auto &key:{"SystemRoot","WINDIR","TEMP","TMP","PATH"})if(qEnvironmentVariableIsSet(key))env.insert(key,qEnvironmentVariable(key));
 env.insert("NEXVARY_PHONEINFOGA_TOKEN",QString::fromLatin1(token));env.insert("GIN_MODE","release");
 OwnedProcess process;process.setWorkingDirectory(cwd.path());process.setProcessEnvironment(env);
 process.setStandardOutputFile(QProcess::nullDevice());process.setStandardErrorFile(QProcess::nullDevice());
 process.setProgram(staged.fileName());process.setArguments({"serve","--no-client","--port",QString::number(port)});
 reservation.close();process.start();
 const auto stop=[&](QVariantMap out){process.stop("osint-request-ended");process.waitForFinished(3000);out.insert("processCleanup",process.cancellationReceipt());if(!process.cleanupComplete())out.insert("error","engine-cleanup-unproved");return out;};
 if(!process.waitForStarted(3000))return stop(failure("engine-start-failed"));
 QNetworkAccessManager manager;manager.setProxy(QNetworkProxy::NoProxy);
 QElapsedTimer startup;startup.start();bool authenticated=false;
 while(startup.elapsed()<5000&&!cancel->load()&&process.state()!=QProcess::NotRunning){
  const auto challenge=QUuid::createUuid().toRfc4122().toHex();
  const auto health=request(manager,port,"/api/",{}, {},cancel,250,challenge);
  const auto expected=QMessageAuthenticationCode::hash(challenge,token,QCryptographicHash::Sha256).toHex();
  if(health.status==200&&health.error.isEmpty()&&health.proof==expected){authenticated=true;break;}
  QThread::msleep(25);
 }
 if(!authenticated)return stop(failure(cancel->load()?"cancelled":"engine-authentication-failed"));
 const auto started=utc();QVariantList rows;
 for(const auto &scanner:unique){
  if(cancel->load())return stop(failure("cancelled"));
  const auto body=QJsonDocument(QJsonObject{{"number",number.mid(1)},{"options",QJsonObject{}}}).toJson(QJsonDocument::Compact);
  const auto response=request(manager,port,"/api/v2/scanners/"+scanner+"/run",token,body,cancel);
  if(cancel->load())return stop(failure("cancelled"));
  rows.append(classifyResponse(scanner,response.status,response.body,response.error));
 }
 return stop({{"schema","nexvary-phone-osint/v1"},{"provider","phoneinfoga"},{"engineSha256",input.executableSha256},
  {"upstreamCommit","55807b05b753dab7bc6fe67403ca385668dda179"},{"number",number},{"startedUtc",started},{"completedUtc",utc()},
  {"scanners",rows},{"limitations","Unverified open-source observations; numbering-plan geography is not device location. Generated links are not search hits. No verified subscriber identity, live location or telecom records. Local process ownership is not OS/network sandboxing."}});
}
}
