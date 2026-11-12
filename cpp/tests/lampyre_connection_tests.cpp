#include "LampyreConnection.hpp"
#include "IdentityService.hpp"
#include "GovernmentOperationPolicy.hpp"
#include <QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
class FixtureServer:public QObject {
public:
 QTcpServer server;QString mode;int requests=0,calls=0,lists=0;QList<QByteArray> headers;QJsonObject lastArguments;
 explicit FixtureServer(const QString &m={}):mode(m){
  if(!server.listen(QHostAddress::LocalHost))qFatal("Cannot bind the synthetic loopback MCP fixture");
  connect(&server,&QTcpServer::newConnection,this,[this]{while(server.hasPendingConnections()){
   auto *socket=server.nextPendingConnection();connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
   connect(socket,&QTcpSocket::readyRead,this,[this,socket]{
    auto data=socket->property("request").toByteArray()+socket->readAll();socket->setProperty("request",data);
    const auto split=data.indexOf("\r\n\r\n");if(split<0||socket->property("handled").toBool())return;
    const auto head=data.left(split);qsizetype length=-1;for(auto line:head.split('\n'))if(line.toLower().startsWith("content-length:"))length=line.mid(15).trimmed().toLongLong();
    if(length<0||data.size()<split+4+length)return;
    socket->setProperty("handled",true);headers.append(head);++requests;
    const auto msg=QJsonDocument::fromJson(data.mid(split+4,length)).object();const auto method=msg.value("method").toString();
    if(mode=="timeout")return;
    QByteArray payload,extra;int status=200;QJsonObject result;
    if(method=="initialize"){
     if(mode=="auth"||mode=="payment"||mode=="forbidden"||mode=="service"){status=mode=="auth"?401:mode=="payment"?402:mode=="forbidden"?403:503;payload="secret-reflection-must-not-display";}
     else if(mode=="redirect"){status=302;extra="Location: https://example.invalid/never-follow\r\n";}
     else {result={{"protocolVersion","2025-06-18"},{"capabilities",QJsonObject{{"tools",QJsonObject{}}}},{"serverInfo",QJsonObject{{"name","Synthetic fixture"},{"version","1"}}}};extra="Mcp-Session-Id: synthetic-session\r\n";}
    }else if(method=="notifications/initialized"){status=202;}
    else if(method=="tools/list"){
     ++lists;
     const QJsonObject schema{{"type","object"},{"required",QJsonArray{"selector"}},{"properties",QJsonObject{{"selector",QJsonObject{{"type","string"}}}}}};
     const QJsonObject tool{{"name",msg.value("params").toObject().value("cursor")=="second-page"?"second":"lookup"},{"description","Synthetic local test; not Lampyre data"},{"inputSchema",schema}};
     result={{"tools",QJsonArray{tool}}};if(mode=="pages"&&msg.value("params").toObject().value("cursor")!="second-page")result.insert("nextCursor","second-page");
    }else if(method=="tools/call"){
     ++calls;lastArguments=msg.value("params").toObject().value("arguments").toObject();
     result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text","Synthetic server reply; TEST-API-KEY"}}}}};
    }
    if(payload.isEmpty()&&status==200)payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",result}}).toJson(QJsonDocument::Compact);
    if(mode=="malformed"&&method=="initialize")payload="not JSON";
    if(mode=="oversize"&&method=="initialize")payload=QByteArray(2*1024*1024+1,'x');
    const bool sse=mode=="sse"&&status==200;
    if(sse)payload="event: message\r\ndata: "+payload+"\r\n\r\n";
    socket->write("HTTP/1.1 "+QByteArray::number(status)+" Fixture\r\nConnection: close\r\nContent-Type: "+(sse?QByteArray("text/event-stream"):QByteArray("application/json"))+"\r\n"+extra+"Content-Length: "+QByteArray::number(payload.size())+"\r\n\r\n"+payload);socket->disconnectFromHost();
   });
  }});
 }
 QString url()const{return "http://127.0.0.1:"+QString::number(server.serverPort())+"/mcp";}
};
class LampyreConnectionTests:public QObject {
 Q_OBJECT
 const QString password="Synthetic-only MCP tests 2026!";
private slots:
 void endpointAndKeyValidation(){
  QVERIFY(LampyreConnection::validEndpoint("https://provider.example/mcp"));
  for(const auto &s:{"http://provider.example/mcp","https://key:secret@provider.example/mcp","https://provider.example/mcp?key=secret","https://provider.example/mcp#secret","file:///tmp/key","https:///mcp"})QVERIFY(!LampyreConnection::validEndpoint(s));
  QVERIFY(!LampyreConnection::validEndpoint("http://127.0.0.1/mcp"));QVERIFY(LampyreConnection::validEndpoint("http://127.0.0.1/mcp",true));QVERIFY(!LampyreConnection::validEndpoint("http://not-loopback.example/mcp",true));
  QVERIFY(LampyreConnection::validKey("TEST-API-KEY"));QVERIFY(!LampyreConnection::validKey("bad\r\nHeader: secret"));QVERIFY(!LampyreConnection::validKey("has space"));QVERIFY(!LampyreConnection::validKey(""));
 }
 void consentAndCredentials(){
  QTemporaryDir t;FixtureServer server;LampyreConnection client(t.path(),nullptr,nullptr,nullptr,true);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",true));QVERIFY(!client.connectProvider(false));QCOMPARE(server.requests,0);
  QVERIFY(!client.resultText().contains("TEST-API-KEY"));
  LampyreConnection reopened(t.path(),nullptr,nullptr,nullptr,true);QVERIFY(reopened.hasCredential());QVERIFY(reopened.credentialSaved());QCOMPARE(reopened.endpoint(),server.url());QVERIFY(!reopened.connected());QCOMPARE(server.requests,0);
#ifdef Q_OS_WIN
  QFile f(t.filePath("lampyre-synthetic.dat"));QVERIFY(f.open(QIODevice::ReadOnly));QVERIFY(!f.readAll().contains("TEST-API-KEY"));f.close();
#endif
  reopened.forget();QVERIFY(!reopened.hasCredential());QVERIFY(!QFile::exists(t.filePath("lampyre-synthetic.dat")));
 }
 void handshakeAndNoAutomaticSearch_data(){QTest::addColumn<QString>("mode");QTest::newRow("json")<<QString();QTest::newRow("streaming-events")<<QString("sse");QTest::newRow("pagination")<<QString("pages");}
 void handshakeAndNoAutomaticSearch(){
  QFETCH(QString,mode);QTemporaryDir t;FixtureServer server(mode);LampyreConnection client(t.path(),nullptr,nullptr,nullptr,true);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",false));QVERIFY(client.connectProvider(true));QTRY_VERIFY_WITH_TIMEOUT(client.connected(),3000);QVERIFY(!client.busy());QCOMPARE(server.calls,0);QCOMPARE(client.tools().size(),mode=="pages"?2:1);
  for(const auto &h:server.headers){bool found=false;for(const auto &line:h.split('\n')){const auto colon=line.indexOf(':');if(colon>0&&line.left(colon).toLower()=="authorization"&&line.mid(colon+1).trimmed()=="Bearer TEST-API-KEY")found=true;}QVERIFY2(found,h.constData());}
  QVERIFY(server.headers.last().toLower().contains("mcp-session-id: synthetic-session"));QVERIFY(server.headers.last().toLower().contains("mcp-protocol-version: 2025-06-18"));
  QVERIFY(!client.callTool("lookup","{}",false));QCOMPARE(server.calls,0);
  QVERIFY(client.connectProvider(true));QTRY_VERIFY_WITH_TIMEOUT(client.connected(),3000);
  QVERIFY(client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));QTRY_COMPARE_WITH_TIMEOUT(client.state(),QString("completed"),3000);QCOMPARE(server.calls,1);QCOMPARE(server.lastArguments.value("selector").toString(),QString("synthetic.invalid"));QVERIFY(!client.resultText().contains("TEST-API-KEY"));QVERIFY(client.resultText().contains("credential redacted"));QCOMPARE(client.resultItems().size(),1);
  const auto path=t.filePath("result.json");QVERIFY(!client.exportResult(path,false));QVERIFY(!QFile::exists(path));QVERIFY(client.exportResult(path,true));QFile output(path);QVERIFY(output.open(QIODevice::ReadOnly));const auto saved=output.readAll();QVERIFY(!saved.contains("TEST-API-KEY"));QVERIFY(saved.contains("unverified-provider-result"));QVERIFY(saved.contains("Synthetic server reply"));QVERIFY(!client.exportResult(path,true));
 }
 void networkFailures_data(){QTest::addColumn<QString>("mode");QTest::addColumn<QString>("error");QTest::newRow("forbidden")<<QString("forbidden")<<QString("provider-authorization-denied");QTest::newRow("service")<<QString("service")<<QString("provider-service-error");QTest::newRow("auth")<<QString("auth")<<QString("authentication-failed");QTest::newRow("payment")<<QString("payment")<<QString("payment-required");QTest::newRow("redirect")<<QString("redirect")<<QString("redirect-refused");QTest::newRow("malformed")<<QString("malformed")<<QString("invalid-response");QTest::newRow("timeout")<<QString("timeout")<<QString("timeout");QTest::newRow("large")<<QString("oversize")<<QString("response-too-large");}
 void networkFailures(){
  QFETCH(QString,mode);QFETCH(QString,error);QTemporaryDir t;FixtureServer server(mode);LampyreConnection client(t.path(),nullptr,nullptr,nullptr,true,mode=="timeout"?150:3000);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",false));QVERIFY(client.connectProvider(true));QTRY_VERIFY_WITH_TIMEOUT(!client.busy(),3000);QCOMPARE(client.errorCode(),error);QVERIFY(!client.connected());QVERIFY(client.resultText().isEmpty());QCOMPARE(server.calls,0);QCOMPARE(server.requests,1);
 }
 void logoutClearsCredentialsAndStopsWork(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);QVERIFY(identity.bootstrap("admin","Synthetic admin",password));QVERIFY(identity.login("admin",password));GovernmentOperationPolicy policy(&identity);FixtureServer server("timeout");LampyreConnection client(t.path(),&identity,nullptr,nullptr,true);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",true));QVERIFY(client.connectProvider(true));identity.logout();QVERIFY(!client.busy());QVERIFY(!client.hasCredential());QVERIFY(client.tools().isEmpty());QVERIFY(client.resultText().isEmpty());QVERIFY(client.endpoint().isEmpty());QVERIFY(identity.login("admin",password));QVERIFY(client.hasCredential());QVERIFY(!client.connected());
 }
 void requestLimitValidation(){
  QTemporaryDir t;LampyreConnection c(t.path(),nullptr,nullptr,nullptr,true);QCOMPARE(c.requestLimit(),0);QVERIFY(!c.setRequestLimit(-1));QVERIFY(!c.setRequestLimit(101));QVERIFY(c.setRequestLimit(1));QCOMPARE(c.requestLimit(),1);c.forget();QCOMPARE(c.requestLimit(),0);
 }
 void productionNeverUsesInsecureFixture(){
  QTemporaryDir t;LampyreConnection client(t.path(),nullptr,nullptr);QVERIFY(!client.configure("http://127.0.0.1/mcp","TEST-API-KEY",true));QVERIFY(!client.connectProvider(true));QVERIFY(!client.callTool("lookup","{}",true));QVERIFY(QDir(t.path()).entryList(QDir::Files).isEmpty());
 }
 void reviewedPermissionAndRevocation(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);QVERIFY(identity.bootstrap("admin","Synthetic admin",password));QVERIFY(identity.login("admin",password));QVERIFY(!identity.createUser("operator","Synthetic operator","investigator",password).isEmpty());QVERIFY(identity.login("operator",password));GovernmentOperationPolicy policy(&identity);FixtureServer server;LampyreConnection client(t.path(),&identity,&policy,nullptr,true);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",false));QVERIFY(!client.connectProvider(true));QCOMPARE(client.errorCode(),QString("external-analysis-approval-required"));QCOMPARE(server.requests,0);
  // An investigator requests; another account reviews; the original account activates.
  const auto request=policy.requestException("external-analysis","Synthetic MCP query",{},600,true);QVERIFY(!request.isEmpty());QVERIFY(identity.login("admin",password));QVERIFY(policy.reviewException(request,"Synthetic independent account review"));QVERIFY(policy.decideException(request,true,"Synthetic decision"));QVERIFY(identity.login("operator",password));
  const auto costRequest=policy.requestException("paid-unknown-cost","Explicit unknown Photon cost acceptance",{},600,true);QVERIFY(!costRequest.isEmpty());
  QVERIFY(identity.login("admin",password));QVERIFY(policy.reviewException(costRequest,"Independent administrator cost review"));QVERIFY(policy.decideException(costRequest,true,"Approve unknown cost for bounded submissions"));QVERIFY(identity.login("operator",password));QVERIFY(policy.activateException(request));QVERIFY(policy.activateException(costRequest));
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",false));QVERIFY(client.setRequestLimit(2));QVERIFY(client.connectProvider(true));QTRY_VERIFY_WITH_TIMEOUT(client.connected(),3000);QVERIFY(client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));QTRY_COMPARE_WITH_TIMEOUT(client.state(),QString("completed"),3000);QCOMPARE(server.calls,1);QCOMPARE(policy.operationReceipts().first().toMap().value("kind").toString(),QString("lampyre-mcp"));QVERIFY(policy.auditStatus().value("healthy").toBool());
  QCOMPARE(client.submittedRequests(),1);QVERIFY(client.setRequestLimit(1));QVERIFY(!client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));QCOMPARE(client.errorCode(),QString("spending-limit-reached"));QCOMPARE(server.calls,1);QVERIFY(client.setRequestLimit(2));
  server.mode="timeout";QVERIFY(client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));policy.revoke("external-analysis");QVERIFY(!client.busy());QVERIFY(client.resultText().isEmpty());QCOMPARE(client.errorCode(),QString("policy-revoked"));
 }
};
QTEST_GUILESS_MAIN(LampyreConnectionTests)
#include "lampyre_connection_tests.moc"
