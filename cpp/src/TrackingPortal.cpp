#include "TrackingPortal.hpp"
#include "GovernmentOperationPolicy.hpp"
#include "WorkshopBridge.hpp"
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QTimer>
#include <QRegularExpression>
#include <QGuiApplication>
#include <QClipboard>
#include <QVariantMap>
#include <QUrl>
namespace {
bool privateAddress(const QHostAddress &a) {
 bool ok=false;const auto n=a.toIPv4Address(&ok);
 return ok&&(a.isLoopback()||(n>>24)==10||(n>>20)==0xac1||(n>>16)==0xc0a8);
}
QString escaped(const QVariant &v){return v.toString().toHtmlEscaped();}
QString statusLabel(const QString &s){
 const QMap<QString,QString> ar={{"Received",QStringLiteral("تم الاستلام")},{"Diagnosing",QStringLiteral("التشخيص")},{"Awaiting parts",QStringLiteral("انتظار القطع")},{"Repairing",QStringLiteral("قيد الإصلاح")},{"Ready",QStringLiteral("جاهز للاستلام")},{"Delivered",QStringLiteral("تم التسليم")},{"Cancelled",QStringLiteral("ملغي")}};
 return ar.value(s,QStringLiteral("غير متاح"))+QStringLiteral(" / ")+s.toHtmlEscaped();
}
}
TrackingPortal::TrackingPortal(WorkshopBridge *w,QObject *parent):QObject(parent),workshop_(w){
 server_.setMaxPendingConnections(16);connect(&server_,&QTcpServer::newConnection,this,&TrackingPortal::accept);
}
QStringList TrackingPortal::addresses() const {
 QStringList out{QStringLiteral("127.0.0.1")};
 for(const auto &a:QNetworkInterface::allAddresses()){if(privateAddress(a)&&!out.contains(a.toString()))out.append(a.toString());}
 return out;
}
QString TrackingPortal::baseUrl() const {return running()?QStringLiteral("http://%1:%2").arg(server_.serverAddress().toString()).arg(server_.serverPort()):QString();}
bool TrackingPortal::start(const QString &address,int port,bool consent){
 if(policy_&&!policy_->permit("tracking-portal")){error_=policy_->error();emit changed();return false;}
 if(running()){error_=QStringLiteral("Stop the portal before changing its address.");emit changed();return false;}
 if(!consent||port<0||port>65535||!addresses().contains(address)||!workshop_->ready()){error_=QStringLiteral("Choose a local/private IPv4 address and explicitly approve this trusted-network HTTP portal.");emit changed();return false;}
 if(!server_.listen(QHostAddress(address),static_cast<quint16>(port))){error_=server_.errorString();emit changed();return false;}
 error_.clear();emit changed();return true;
}
void TrackingPortal::stop(){server_.close();for(auto *s:findChildren<QTcpSocket*>())s->abort();emit changed();}
QString TrackingPortal::link(int id,int days,bool consent){
 if(policy_&&!policy_->permit("tracking-portal")){error_=policy_->error();emit changed();return {};}
 if(!running()){error_=QStringLiteral("Start the portal before issuing a link.");emit changed();return {};}
 const auto token=workshop_->issueTrackingToken(id,days,consent);if(token.isEmpty()){error_=workshop_->error();emit changed();return {};}
 error_.clear();emit changed();return baseUrl()+QStringLiteral("/t/")+token;
}
bool TrackingPortal::copyLink(const QString &url){if(policy_&&!policy_->permit("tracking-portal"))return false;if(!running()||!url.startsWith(baseUrl()+QStringLiteral("/t/")))return false;QGuiApplication::clipboard()->setText(url);return true;}
QByteArray TrackingPortal::html(const QVariantMap &s){
 if(s.isEmpty())return QByteArray("<!doctype html><html lang='en'><meta charset='utf-8'><title>Unavailable</title><p>Tracking link unavailable or expired.</p></html>");
 QString out=QStringLiteral("<!doctype html><html lang='ar' dir='rtl'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><meta http-equiv='refresh' content='30'><title>NEXVARY — متابعة الصيانة</title><style>body{background:#0a0a0a;color:#eee;font:18px system-ui;margin:0;padding:24px}main{max-width:680px;margin:auto;border:1px solid #555;border-radius:18px;padding:24px;background:linear-gradient(160deg,#303030,#101010 28%,#080808)}h1{font-size:26px}h2{color:#62d5b5}p,li{overflow-wrap:anywhere;white-space:pre-wrap}li{padding:10px}small{color:#bdbdbd}</style></head><body><main><h1>NEXVARY MR — متابعة الصيانة</h1><p>أمر الصيانة / Repair order #%1</p><p>%2 %3</p><h2>%4</h2><p>%5</p><h3>مراحل الصيانة / Repair history</h3><ol>").arg(s.value("id").toInt()).arg(escaped(s.value("brand")),escaped(s.value("model")),statusLabel(s.value("status").toString()),escaped(s.value("message")));
 for(const auto &v:s.value("history").toList()){const auto m=v.toMap();out+=QStringLiteral("<li>%1<br><small dir='ltr'>%2 UTC</small></li>").arg(statusLabel(m.value("status").toString()),escaped(m.value("atUtc")));}
 out+=QStringLiteral("</ol><small>تتحدث الحالة كل 30 ثانية أثناء تشغيل بوابة المركز. احتفظ بالرابط لنفسك.<br>Updates every 30 seconds while the workshop portal is online. Keep your link private.</small></main></body></html>");return out.toUtf8();
}
void TrackingPortal::accept(){
 if(policy_&&!policy_->permit("tracking-portal")){stop();error_=policy_->error();emit changed();return;}
 while(server_.hasPendingConnections()){
  auto *socket=server_.nextPendingConnection();socket->setParent(this);
  if(clients_>=16){socket->abort();socket->deleteLater();continue;}++clients_;
  socket->setReadBufferSize(8192);socket->setProperty("request",QByteArray());
  connect(socket,&QTcpSocket::disconnected,this,[this,socket]{--clients_;socket->deleteLater();});
  QTimer::singleShot(5000,socket,[socket]{socket->abort();});
  connect(socket,&QTcpSocket::readyRead,this,[this,socket]{
   if(policy_&&!policy_->permit("tracking-portal")){stop();return;}
   if(socket->property("answered").toBool())return;
   auto request=socket->property("request").toByteArray()+socket->readAll();socket->setProperty("request",request);
   if(request.size()>4096){socket->abort();return;}
   if(!request.contains("\r\n\r\n"))return;
   socket->setProperty("answered",true);
   const auto lines=request.split('\n');const auto first=lines.value(0).trimmed();
   static const QRegularExpression route(QStringLiteral("^GET /t/([0-9a-f]{64}) HTTP/1\\.[01]$"));
   const auto match=route.match(QString::fromLatin1(first));
   const QByteArray expectedHost=QStringLiteral("%1:%2").arg(server_.serverAddress().toString()).arg(server_.serverPort()).toLatin1();
   int hosts=0;bool hostOk=false,hasBody=false;
   for(const auto &line:lines){const auto l=line.trimmed();if(l.toLower().startsWith("host:")){++hosts;hostOk=l.mid(5).trimmed()==expectedHost;}if(l.toLower().startsWith("transfer-encoding:")||l.toLower().startsWith("content-length:"))hasBody=true;}
   const auto snapshot=match.hasMatch()&&hosts==1&&hostOk&&!hasBody?workshop_->publicTracking(match.captured(1)):QVariantMap();
   const auto body=html(snapshot);const QByteArray code=snapshot.isEmpty()?QByteArray("404 Not Found"):QByteArray("200 OK");
   const QByteArray headers="HTTP/1.1 "+code+"\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: no-store\r\nReferrer-Policy: no-referrer\r\nX-Content-Type-Options: nosniff\r\nX-Robots-Tag: noindex, nofollow\r\nContent-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'\r\nConnection: close\r\nContent-Length: "+QByteArray::number(body.size())+"\r\n\r\n";
   socket->write(headers+body);socket->disconnectFromHost();
  });
 }
}

void TrackingPortal::setOperationPolicy(GovernmentOperationPolicy *policy){
 policy_=policy;
 auto apply=[this]{if(policy_&&!policy_->granted("tracking-portal")){stop();error_=QStringLiteral("Government offline policy: tracking listener and clients stopped.");emit changed();}};
 connect(policy,&GovernmentOperationPolicy::changed,this,apply);apply();
}
