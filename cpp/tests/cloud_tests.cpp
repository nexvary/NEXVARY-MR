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
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
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
   connect(s,&QTcpSocket::readyRead,this,[this,s]{auto bytes=s->property("request").toByteArray()+s->readAll();s->setProperty("request",bytes);int boundary=bytes.indexOf("\r\n\r\n");if(boundary<0)return;int length=0;QByteArray authorization;
    // HTTP field names are case-insensitive; Qt 6.8 normalizes them to lower case.
    // Preserve the credential value's exact bytes instead of lowercasing the request.
    for(const auto &line:bytes.left(boundary).split('\n')){const int colon=line.indexOf(':');if(colon<0)continue;const auto name=line.left(colon).trimmed().toLower(),value=line.mid(colon+1).trimmed();if(name=="content-length")length=value.toInt();if(name=="authorization")authorization=value;}
    if(bytes.size()<boundary+4+length){return;}
    ++calls;
    if(mode=="hold")return; // Keep a real received request pending for revocation tests.
    int status=200;QJsonObject out;QByteArray extra;
    if(authorization!="Bearer "+key.toLatin1())status=401;
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
 void governmentGateBlocksSavedCredentialsAndRevokesSync(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("w.db"));const auto config=t.filePath("saved.dat"),path=credentials(t);
  // Seed the pre-existing protected settings with the component fixture, without
  // entering the event loop or permitting an initial network request.
  {CloudTracking seed(&w,config,true);QVERIFY(seed.importCredentials(path,true));}
  IdentityService identity(t.filePath("identity"),true);const QString pass="Synthetic cloud policy 2026!";
  QVERIFY(identity.bootstrap("policy-admin","Synthetic administrator",pass));QVERIFY(identity.login("policy-admin",pass));GovernmentOperationPolicy policy(&identity);
  CloudTracking cloud(&w,config,true);cloud.setOperationPolicy(&policy);QVERIFY(cloud.configured());
  QVERIFY(!cloud.syncNow());QVERIFY(cloud.issueLink(1,1,true).isEmpty());QVERIFY(!cloud.importCredentials(path,true));QTest::qWait(900);QCOMPARE(calls,0);
  QVERIFY(policy.grant("cloud-publish","synthetic loopback publishing fixture",true));QCOMPARE(calls,0);QVERIFY(cloud.syncNow());QTRY_VERIFY_WITH_TIMEOUT(cloud.inSync(),5000);QCOMPARE(calls,2);
  const auto before=calls;policy.revoke("cloud-publish");QVERIFY(!cloud.inSync());QVERIFY(!cloud.syncNow());QTest::qWait(900);QCOMPARE(calls,before);
  QVERIFY(w.createOrder("Synthetic offline customer","TEST","Lab","Fixture","synthetic","Synthetic intake","Synthetic operator","")>0);
  QVERIFY(policy.grant("cloud-publish","synthetic pending request cancellation",true));QTest::qWait(900);QCOMPARE(calls,before); // A change made while denied must not leave a debounce that publishes on regrant.
  mode="hold";QVERIFY(cloud.syncNow());QTRY_VERIFY_WITH_TIMEOUT(calls>before,3000);QVERIFY(cloud.busy());identity.logout();QVERIFY(!cloud.busy());QVERIFY(!cloud.inSync());
  // Aborted GET may already have reached the server; no POST is allowed after
  // revocation and a new grant never automatically resumes the old request.
  QTest::qWait(100);const auto cancelledCalls=calls;QVERIFY(identity.login("policy-admin",pass));QVERIFY(policy.grant("cloud-publish","synthetic regrant",true));QTest::qWait(900);QCOMPARE(calls,cancelledCalls);QVERIFY(!cloud.inSync());
 }
 void snapshotSyncAndRevocation(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("workshop.db"));const int order=w.createOrder("PRIVATE-NAME","PRIVATE-PHONE","Samsung","TEST-MODEL","PRIVATE-SERIAL","PRIVATE-FAULT","PRIVATE-TECH","PRIVATE-NOTES");QVERIFY(order>0);
  CloudTracking c(&w,t.filePath("remote.dat"),true);const auto file=credentials(t);QVERIFY(!c.importCredentials(file,false));QVERIFY(c.importCredentials(file,true));QTRY_VERIFY2_WITH_TIMEOUT(c.inSync(),qPrintable(c.error()),5000);
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
  mode="redirect";CloudTracking test(&w,t.filePath("test.dat"),true);QVERIFY(test.importCredentials(path,true));QTRY_VERIFY_WITH_TIMEOUT(!test.error().isEmpty(),5000);QVERIFY(test.error().contains("(302)"));QVERIFY(!test.inSync());QCOMPARE(calls,1);
 }
 void rejectsWrongApiKey(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("wrong.db"));const auto path=credentials(t);QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));auto data=QJsonDocument::fromJson(file.readAll()).object();file.close();data.insert("apiKey",QString(64,'c'));QVERIFY(file.open(QIODevice::WriteOnly|QIODevice::Truncate));file.write(QJsonDocument(data).toJson());file.close();
  CloudTracking cloud(&w,t.filePath("wrong.dat"),true);QVERIFY(cloud.importCredentials(path,true));QTRY_VERIFY_WITH_TIMEOUT(!cloud.error().isEmpty(),5000);QVERIFY(cloud.error().contains("(401)"));QVERIFY(!cloud.inSync());QCOMPARE(calls,1);QVERIFY(latest.isEmpty());
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
  {CloudTracking c(&w,config,true);QVERIFY(c.importCredentials(path,true));QTRY_VERIFY2_WITH_TIMEOUT(c.inSync(),qPrintable(c.error()),5000);}
  mode="identity";CloudTracking reopened(&w,config,true);QVERIFY(reopened.configured());QTRY_VERIFY_WITH_TIMEOUT(!reopened.error().isEmpty(),5000);QVERIFY(reopened.error().contains("identity"));QVERIFY(!reopened.inSync());
 }
};
QTEST_MAIN(CloudTests)
#include "cloud_tests.moc"
