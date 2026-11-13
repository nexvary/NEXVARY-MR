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
 QTcpServer server;QString mode;int requests=0,calls=0,lists=0,paidCalls=0;QList<QByteArray> headers;QJsonObject lastArguments;
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
     else {result={{"protocolVersion","2025-06-18"},{"capabilities",QJsonObject{{"tools",QJsonObject{}}}},{"serverInfo",QJsonObject{{"name","Synthetic fixture"},{"version","1"}}}};if(mode!="metadata")extra="Mcp-Session-Id: synthetic-session\r\n";}
    }else if(method=="notifications/initialized"){status=202;}
    else if(method=="tools/list"){
     ++lists;
     const QJsonObject schema{{"type","object"},{"required",QJsonArray{"selector"}},{"properties",QJsonObject{{"selector",QJsonObject{{"type","string"}}}}}};
     const QJsonObject tool{{"name",msg.value("params").toObject().value("cursor")=="second-page"?"second":"lookup"},{"description","Synthetic local test; not Lampyre data"},{"inputSchema",schema}};
     result={{"tools",QJsonArray{tool}}};
     if(mode=="metadata"){
      QJsonArray tools;for(const auto *name:{"lampyre_balance","lampyre_categories_list","lampyre_endpoints_from_categories"})tools.append(QJsonObject{{"name",name},{"description","Synthetic free metadata. Free."},{"inputSchema",QJsonObject{{"type","object"}}}});
      tools.append(QJsonObject{{"name","lampyre_run_crawlers"},{"description","Synthetic paid operation"},{"inputSchema",QJsonObject{{"type","object"},{"properties",QJsonObject{{"tasks",QJsonObject{{"type","array"}}}}},{"required",QJsonArray{"tasks"}}}}});result={{"tools",tools}};
     }
if(mode=="pages"&&msg.value("params").toObject().value("cursor")!="second-page")result.insert("nextCursor","second-page");
    }else if(method=="tools/call"){
     ++calls;lastArguments=msg.value("params").toObject().value("arguments").toObject();
     result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text","Synthetic server reply; TEST-API-KEY; Lampyre source label"}}}}};
     if(mode=="metadata"){
      const auto name=msg.value("params").toObject().value("name").toString();QJsonObject data;
      if(name=="lampyre_balance")data={{"photons",37.5}};
      else if(name=="lampyre_categories_list")data={{"categories",QJsonArray{QJsonObject{{"name","email"},{"description","Synthetic email category"}},QJsonObject{{"name","phone"}}}}};
      else if(name=="lampyre_endpoints_from_categories")data={{"crawlers",QJsonObject{{"email",QJsonArray{QJsonObject{{"tool_name","synthetic_email_lookup"},{"title","Synthetic email lookup"},{"photons",2.0},{"args","email: string (Email address) (all required)"}}}}}}};
      else if(name=="lampyre_run_crawlers"){++paidCalls;data={{"fixture",true}};}
      result={{"content",QJsonArray{QJsonObject{{"type","text"},{"text",QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Compact))}}}}};
     }

    }
    if(payload.isEmpty()&&status==200)payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",result}}).toJson(QJsonDocument::Compact);
    if(mode=="unsupported"&&method=="initialize")payload=QJsonDocument(QJsonObject{{"jsonrpc","2.0"},{"id",msg.value("id")},{"result",QJsonObject{{"protocolVersion","invalid-version"}}}}).toJson();
    if(mode=="malformed"&&method=="initialize")payload="not JSON";
    if(mode=="oversize"&&method=="initialize")payload=QByteArray(2*1024*1024+1,'x');
    const bool sse=(mode=="sse"||mode=="metadata")&&status==200;
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
  QVERIFY(LampyreConnection::validEndpoint("https://account.lampyre.io/api/v2/mcp"));QVERIFY(!LampyreConnection::validEndpoint("https://provider.example/mcp"));
  for(const auto &s:{"http://provider.example/mcp","https://key:secret@provider.example/mcp","https://provider.example/mcp?key=secret","https://provider.example/mcp#secret","file:///tmp/key","https:///mcp"})QVERIFY(!LampyreConnection::validEndpoint(s));
  QVERIFY(!LampyreConnection::validEndpoint("http://127.0.0.1/mcp"));QVERIFY(LampyreConnection::validEndpoint("http://127.0.0.1/mcp",true));QVERIFY(!LampyreConnection::validEndpoint("http://not-loopback.example/mcp",true));
  QVERIFY(LampyreConnection::validKey("TEST-API-KEY"));QVERIFY(!LampyreConnection::validKey("bad\r\nHeader: secret"));QVERIFY(!LampyreConnection::validKey("has space"));QVERIFY(!LampyreConnection::validKey(""));
 }
 void consentAndCredentials(){
  QTemporaryDir t;FixtureServer server;LampyreConnection client(t.path(),nullptr,nullptr,nullptr,true);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",true));QVERIFY(!client.connectProvider(false));QCOMPARE(server.requests,0);
  QVERIFY(!client.resultText().contains("TEST-API-KEY"));
  LampyreConnection reopened(t.path(),nullptr,nullptr,nullptr,true);QVERIFY(reopened.hasCredential());QVERIFY(reopened.credentialSaved());QCOMPARE(reopened.endpoint(),server.url());QVERIFY(!reopened.connected());QCOMPARE(server.requests,0);
  QCOMPARE(reopened.metaObject()->indexOfProperty("endpoint"),-1);
  QCOMPARE(reopened.metaObject()->indexOfMethod("configure(QString,QString,bool)"),-1);
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
  const auto path=t.filePath("result.json");QVERIFY(!client.exportResult(path,false));QVERIFY(!QFile::exists(path));QVERIFY(client.exportResult(path,true));QFile output(path);QVERIFY(output.open(QIODevice::ReadOnly));const auto saved=output.readAll();QVERIFY(!saved.contains("TEST-API-KEY"));QVERIFY(saved.contains("unverified-provider-result"));QVERIFY(QJsonDocument::fromJson(saved).object().value("redactedDisplayCopy").toBool());QCOMPARE(QJsonDocument::fromJson(saved).object().value("originalResultSha256").toString().size(),64);QVERIFY(saved.contains("external service redacted"));QVERIFY(saved.contains("Synthetic server reply"));QVERIFY(!saved.contains("Lampyre"));QVERIFY(!saved.contains("endpointHost"));QVERIFY(!saved.contains(server.url().toUtf8()));QVERIFY(!client.exportResult(path,true));
 }
 void networkFailures_data(){QTest::addColumn<QString>("mode");QTest::addColumn<QString>("error");QTest::newRow("unsupported")<<QString("unsupported")<<QString("unsupported-protocol");QTest::newRow("forbidden")<<QString("forbidden")<<QString("provider-authorization-denied");QTest::newRow("service")<<QString("service")<<QString("provider-service-error");QTest::newRow("auth")<<QString("auth")<<QString("authentication-failed");QTest::newRow("payment")<<QString("payment")<<QString("payment-required");QTest::newRow("redirect")<<QString("redirect")<<QString("redirect-refused");QTest::newRow("malformed")<<QString("malformed")<<QString("invalid-response");QTest::newRow("timeout")<<QString("timeout")<<QString("timeout");QTest::newRow("large")<<QString("oversize")<<QString("response-too-large");}
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
 void cancellationNeverDispatchesSearch(){
  QTemporaryDir t;FixtureServer server("timeout");LampyreConnection c(t.path(),nullptr,nullptr,nullptr,true);
  QVERIFY(c.configure(server.url(),"TEST-API-KEY",false));QVERIFY(c.connectProvider(true));c.cancel();
  QVERIFY(!c.busy());QVERIFY(!c.connected());QCOMPARE(c.state(),QString("cancelled-local"));QCOMPARE(server.calls,0);
 }
 void displayRedactsUntrustedMetadata(){
  QTemporaryDir t;LampyreConnection c(t.path(),nullptr,nullptr,nullptr,true);
  QVERIFY(c.configure("http://127.0.0.1/mcp","TEST-API-KEY",false));
  const auto safe=c.displayText("Lampyre https://account.lampyre.io/api/v2/mcp TEST-API-KEY https://api.lampyre.io/internal");
  QVERIFY(!safe.contains("lampyre",Qt::CaseInsensitive));QVERIFY(!safe.contains("TEST-API-KEY"));QVERIFY(!safe.contains("https://"));
 }
 void productionNeverUsesInsecureFixture(){
  QTemporaryDir t;LampyreConnection client(t.path(),nullptr,nullptr);QVERIFY(!client.configure("http://127.0.0.1/mcp","TEST-API-KEY",true));QVERIFY(!client.connectProvider(true));QVERIFY(!client.callTool("lookup","{}",true));QVERIFY(QDir(t.path()).entryList(QDir::Files).isEmpty());
 }
 void directWorkMigratesSingleProtectedCredential(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);
  QVERIFY(identity.bootstrap("admin","Synthetic administrator",password));QVERIFY(identity.login("admin",password));
  const auto credentials=t.filePath("credentials");LampyreConnection client(credentials,&identity,nullptr,nullptr,true);
  QVERIFY(client.configureCredential("SYNTHETIC-OLD-API-KEY",true));QVERIFY(identity.startDirectWork());
  QVERIFY(client.hasCredential());QVERIFY(client.credentialSaved());
  LampyreConnection reopened(credentials,&identity,nullptr,nullptr,true);QVERIFY(reopened.hasCredential());
  QCOMPARE(QDir(credentials).entryList({"lampyre-*.dat"},QDir::Files).size(),2);
  client.forget();QVERIFY(!client.hasCredential());
  LampyreConnection forgotten(credentials,&identity,nullptr,nullptr,true);QVERIFY(!forgotten.hasCredential());
  QVERIFY(client.configureCredential("SYNTHETIC-TEMPORARY-KEY",false));QVERIFY(client.hasCredential());
  LampyreConnection temporaryReopened(credentials,&identity,nullptr,nullptr,true);QVERIFY(!temporaryReopened.hasCredential());
 }
#ifdef Q_OS_WIN
 void windowsDirectProtectedPersistence(){
  QTemporaryDir t;const auto path=t.filePath("identity"),credentials=t.filePath("credentials");QString principal;
  {IdentityService identity(path);QVERIFY(identity.ready());QVERIFY(identity.startDirectWork());principal=identity.userId();
   LampyreConnection client(credentials,&identity,nullptr);QVERIFY(client.configureCredential("SYNTHETIC-DPAPI-TEST-KEY",true));
   QFile protectedKey(QDir(credentials).filePath("lampyre-"+principal+".dat"));QVERIFY(protectedKey.open(QIODevice::ReadOnly));
   QVERIFY(!protectedKey.readAll().contains("SYNTHETIC-DPAPI-TEST-KEY"));}
  IdentityService reopened(path);QVERIFY(reopened.startDirectWork());QCOMPARE(reopened.userId(),principal);
  LampyreConnection client(credentials,&reopened,nullptr);QVERIFY(client.hasCredential());QVERIFY(client.credentialSaved());
 }
#endif
 void categoryCatalogIsFreeAndRetailPriceIsSeparate(){
  QTemporaryDir t;FixtureServer server("metadata");LampyreConnection client(t.path(),nullptr,nullptr,nullptr,true);
  QCOMPARE(client.retailPriceEur(),14.0);QVERIFY(client.configure(server.url(),"TEST-API-KEY",false));
  QVERIFY(client.connectProvider(true));QTRY_VERIFY_WITH_TIMEOUT(client.connected(),3000);QCOMPARE(server.calls,0);
  QVERIFY(client.refreshCatalog());QTRY_VERIFY_WITH_TIMEOUT(!client.busy(),3000);QCOMPARE(client.categories().size(),2);QCOMPARE(client.creditBalance(),37.5);QCOMPARE(client.submittedRequests(),0);QCOMPARE(server.paidCalls,0);
  QVERIFY(!client.discoverServices("invented-category"));QVERIFY(client.discoverServices("email"));
  QTRY_VERIFY_WITH_TIMEOUT(!client.busy(),3000);QCOMPARE(client.services().size(),1);
  QVERIFY(!client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",2,false));QCOMPARE(server.paidCalls,0);
  QVERIFY(!client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",3,true));QCOMPARE(server.paidCalls,0);
  QVERIFY(!client.runService("synthetic_email_lookup","{\"wrong\":\"value\"}",2,true));QCOMPARE(server.paidCalls,0);
  QVERIFY(client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",2,true));
  QTRY_COMPARE_WITH_TIMEOUT(client.state(),QString("completed"),3000);QCOMPARE(server.paidCalls,1);QCOMPARE(client.submittedRequests(),1);QCOMPARE(client.creditBalance(),-1.0);
  QCOMPARE(server.lastArguments.value("tasks").toArray().size(),1);QVERIFY(client.exportResult(t.filePath("result.json"),true));
  QFile output(t.filePath("result.json"));QVERIFY(output.open(QIODevice::ReadOnly));const auto quote=QJsonDocument::fromJson(output.readAll()).object().value("retailQuote").toObject();
  QCOMPARE(quote.value("minorUnits").toInt(),1400);QCOMPARE(quote.value("collectionStatus").toString(),QString("not-collected-by-application"));
 }
 void directWorkWithoutIndependentApproval(){
  QTemporaryDir t;IdentityService identity(t.filePath("identity"),true);QVERIFY(identity.startDirectWork());
  GovernmentOperationPolicy policy(&identity);FixtureServer server;LampyreConnection client(t.path(),&identity,&policy,nullptr,true);
  QVERIFY(client.configure(server.url(),"TEST-API-KEY",true));QVERIFY(client.connectProvider(true));
  QTRY_VERIFY_WITH_TIMEOUT(client.connected(),3000);QCOMPARE(server.calls,0);QVERIFY(policy.operationReceipts().isEmpty());
  QVERIFY(!client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",false));QCOMPARE(server.calls,0);
  QVERIFY(client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));
  QTRY_COMPARE_WITH_TIMEOUT(client.state(),QString("completed"),3000);QCOMPARE(server.calls,1);
  const auto receipt=policy.operationReceipts().first().toMap();QCOMPARE(receipt.value("schema").toString(),QString("nexvary-investigator-operation/v1"));
  QCOMPARE(receipt.value("status").toString(),QString("completed"));QVERIFY(receipt.value("requestId").toString().isEmpty());QVERIFY(policy.auditStatus().value("healthy").toBool());
  QVERIFY(client.exportResult(t.filePath("direct-result.json"),true));QCOMPARE(client.remainingSubmissions(identity.userId()),-1);
  QVERIFY(client.setRequestLimit(1));QVERIFY(!client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));QCOMPARE(server.calls,1);
  QVERIFY(client.setRequestLimit(0));server.mode="timeout";QVERIFY(client.callTool("lookup","{\"selector\":\"synthetic.invalid\"}",true));
  identity.logout();QVERIFY(!client.busy());QVERIFY(client.resultText().isEmpty());QVERIFY(!client.hasCredential());
  QVERIFY(identity.startDirectWork());QVERIFY(client.hasCredential());
  GovernmentOperationPolicy reopened(&identity);QVERIFY(reopened.auditStatus().value("healthy").toBool());
 }

};
QTEST_GUILESS_MAIN(LampyreConnectionTests)
#include "lampyre_connection_tests.moc"
