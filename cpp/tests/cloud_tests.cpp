#include <QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QNetworkReply>
#include <QEventLoop>
#include "WorkshopBridge.hpp"
#include "CloudTracking.hpp"
class CloudTests:public QObject {
 Q_OBJECT
 QTcpServer server;
 QString key=QString(64,'a'),source=QString(32,'b');
 QJsonObject latest;
 int revision=0,calls=0;
 QString mode;
private slots:
 void init(){
  revision=0;calls=0;latest={};mode.clear();QVERIFY(server.listen(QHostAddress::LocalHost,0));
  connect(&server,&QTcpServer::newConnection,this,[this]{while(server.hasPendingConnections()){
   auto *s=server.nextPendingConnection();s->setProperty("request",QByteArray());
   connect(s,&QTcpSocket::disconnected,s,&QTcpSocket::deleteLater);
   connect(s,&QTcpSocket::readyRead,this,[this,s]{auto bytes=s->property("request").toByteArray()+s->readAll();s->setProperty("request",bytes);int boundary=bytes.indexOf("\r\n\r\n");if(boundary<0)return;int length=0;for(const auto &line:bytes.left(boundary).split('\n'))if(line.toLower().startsWith("content-length:"))length=line.mid(15).trimmed().toInt();if(bytes.size()<boundary+4+length)return;++calls;
    int status=200;QJsonObject out;QByteArray extra;
    if(!bytes.contains("Authorization: Bearer "+key.toLatin1()))status=401;
    else if(mode=="redirect"){status=302;extra="Location: http://127.0.0.1:1/leak\r\n";}
    else if(bytes.startsWith("GET /api/v1/state "))out={{"schemaVersion",1},{"sourceId",mode=="identity"?QString(32,'c'):source},{"revision",revision}};
    else if(bytes.startsWith("POST /api/v1/snapshot ")){latest=QJsonDocument::fromJson(bytes.mid(boundary+4,length)).object();if(latest.value("expectedRevision").toInt()!=revision)status=409;else out={{"revision",++revision},{"count",latest.value("records").toArray().size()}};}
    else status=404;
    const auto body=QJsonDocument(out).toJson(QJsonDocument::Compact);s->write("HTTP/1.1 "+QByteArray::number(status)+" Result\r\nContent-Type: application/json\r\nConnection: close\r\n"+extra+"Content-Length: "+QByteArray::number(body.size())+"\r\n\r\n"+body);s->disconnectFromHost();
   });
  }});
 }
 void cleanup(){server.close();server.disconnect(this);}
 QString credentials(const QTemporaryDir &t){const auto path=t.filePath("credentials.json");QFile f(path);if(!f.open(QIODevice::WriteOnly))return {};f.write(QJsonDocument(QJsonObject{{"schemaVersion",1},{"serverUrl",QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())},{"sourceId",source},{"apiKey",key}}).toJson());return path;}
 void snapshotSyncAndRevocation(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("workshop.db"));const int order=w.createOrder("PRIVATE-NAME","PRIVATE-PHONE","Samsung","TEST-MODEL","PRIVATE-SERIAL","PRIVATE-FAULT","PRIVATE-TECH","PRIVATE-NOTES");QVERIFY(order>0);
  CloudTracking c(&w,t.filePath("remote.dat"),true);const auto file=credentials(t);QVERIFY(!c.importCredentials(file,false));QVERIFY(c.importCredentials(file,true));QTRY_VERIFY_WITH_TIMEOUT(c.inSync(),5000);
  const auto link=c.issueLink(order,1,true);QVERIFY(link.contains("/t/"));QVERIFY(!c.inSync());QTRY_VERIFY_WITH_TIMEOUT(c.inSync(),5000);
  auto records=latest.value("records").toArray();QCOMPARE(records.size(),1);const auto payload=QJsonDocument(latest).toJson();QVERIFY(!payload.contains("PRIVATE-"));QVERIFY(!payload.contains(link.section('/',-1).toLatin1()));QCOMPARE(records[0].toObject().value("status").toString(),QStringLiteral("Received"));
  QVERIFY(w.updateOrder(order,"Repairing","PRIVATE-TECH",12300,"PRIVATE-NOTES"));QTRY_VERIFY_WITH_TIMEOUT(c.inSync(),5000);QCOMPARE(latest.value("records").toArray()[0].toObject().value("status").toString(),QStringLiteral("Repairing"));
  QVERIFY(w.revokeTracking(order));QTRY_VERIFY_WITH_TIMEOUT(c.inSync(),5000);QCOMPARE(latest.value("records").toArray().size(),0);
#ifdef Q_OS_WIN
  QFile stored(t.filePath("remote.dat"));QVERIFY(stored.open(QIODevice::ReadOnly));QVERIFY(!stored.readAll().contains(key.toLatin1()));stored.close();
#endif
  QVERIFY(!c.disconnectServer(false));QVERIFY(c.disconnectServer(true));QVERIFY(!c.configured());
 }
 void rejectsPlainHttpAndRedirects(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("w.db"));const auto path=credentials(t);CloudTracking production(&w,t.filePath("prod.dat"));QVERIFY(!production.importCredentials(path,true));QVERIFY(!production.configured());
  mode="redirect";CloudTracking test(&w,t.filePath("test.dat"),true);QVERIFY(test.importCredentials(path,true));QTRY_VERIFY_WITH_TIMEOUT(!test.error().isEmpty(),5000);QVERIFY(!test.inSync());QCOMPARE(calls,1);
 }
 void realServerIntegration(){
  const auto credentialsFile=qEnvironmentVariable("MR_PORTAL_TEST_CREDENTIALS"),caFile=qEnvironmentVariable("MR_PORTAL_TEST_CA");if(credentialsFile.isEmpty()||caFile.isEmpty())QSKIP("Run server TLS integration with its temporary CA and credentials fixture.");
  QFile ca(caFile);QVERIFY(ca.open(QIODevice::ReadOnly));auto ssl=QSslConfiguration::defaultConfiguration();ssl.addCaCertificates(QSslCertificate::fromData(ca.readAll()));QSslConfiguration::setDefaultConfiguration(ssl);
  QTemporaryDir t;WorkshopBridge w(t.filePath("real.db"));const int id=w.createOrder("PRIVATE-OWNER","PRIVATE-PHONE","SYNTHETIC","NATIVE-TLS-TEST","PRIVATE-SERIAL","PRIVATE-FAULT","PRIVATE-TECH","PRIVATE-NOTES");QVERIFY(id>0);
  CloudTracking cloud(&w,t.filePath("real-remote.dat"));QVERIFY(cloud.importCredentials(credentialsFile,true));QTRY_VERIFY_WITH_TIMEOUT(cloud.inSync(),10000);
  const auto link=cloud.issueLink(id,1,true);QVERIFY(link.startsWith("https://"));QTRY_VERIFY_WITH_TIMEOUT(cloud.inSync(),10000);
  QNetworkAccessManager network;
  auto get=[&network](const QString &url){auto *reply=network.get(QNetworkRequest{QUrl(url)});QEventLoop loop;QTimer::singleShot(5000,&loop,&QEventLoop::quit);QObject::connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);loop.exec();const auto code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();const auto body=reply->readAll();reply->deleteLater();return QVariantMap{{"code",code},{"body",body}};};
  const auto visible=get(link);QCOMPARE(visible.value("code").toInt(),200);QVERIFY(visible.value("body").toByteArray().contains("NATIVE-TLS-TEST"));QVERIFY(!visible.value("body").toByteArray().contains("PRIVATE-"));
  QVERIFY(w.revokeTracking(id));QTRY_VERIFY_WITH_TIMEOUT(cloud.inSync(),10000);QCOMPARE(get(link).value("code").toInt(),404);
 }
 void identityAndPersistence(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("w.db"));const auto path=credentials(t),config=t.filePath("remote.dat");
  {CloudTracking c(&w,config,true);QVERIFY(c.importCredentials(path,true));QTRY_VERIFY_WITH_TIMEOUT(c.inSync(),5000);}
  mode="identity";CloudTracking reopened(&w,config,true);QVERIFY(reopened.configured());QTRY_VERIFY_WITH_TIMEOUT(!reopened.error().isEmpty(),5000);QVERIFY(!reopened.inSync());
 }
};
QTEST_MAIN(CloudTests)
#include "cloud_tests.moc"
