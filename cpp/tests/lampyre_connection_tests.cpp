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
#include "McpFixtureServer.hpp"
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
  QVERIFY(client.refreshCatalog());QTRY_VERIFY_WITH_TIMEOUT(!client.busy(),3000);QCOMPARE(client.categories().size(),2);QCOMPARE(client.creditBalance(),37.5);QCOMPARE(client.submittedRequests(),0);QCOMPARE(server.paidCalls,0);QVERIFY(client.invoices().isEmpty());
  QVERIFY(!client.discoverServices("invented-category"));QVERIFY(client.discoverServices("email"));
  QTRY_VERIFY_WITH_TIMEOUT(!client.busy(),3000);QCOMPARE(client.services().size(),1);
  QVERIFY(!client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",2,false));QCOMPARE(server.paidCalls,0);
  QVERIFY(!client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",3,true));QCOMPARE(server.paidCalls,0);
  QVERIFY(!client.runService("synthetic_email_lookup","{\"wrong\":\"value\"}",2,true));QCOMPARE(server.paidCalls,0);
  QVERIFY(client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",2,true));
  QTRY_COMPARE_WITH_TIMEOUT(client.state(),QString("completed"),3000);QCOMPARE(server.paidCalls,1);QCOMPARE(client.submittedRequests(),1);QCOMPARE(client.creditBalance(),-1.0);
  QCOMPARE(client.invoices().size(),1);const auto invoice=client.invoices().first().toMap();
  QCOMPARE(invoice.value("currency").toString(),QString("EUR"));QCOMPARE(invoice.value("amountDueMinorUnits").toInt(),1400);
  QCOMPARE(client.totalDueMinorUnits(),qint64(1400));QCOMPARE(invoice.value("status").toString(),QString("due"));QCOMPARE(invoice.value("paymentStatus").toString(),QString("unpaid"));
  QVERIFY(!invoice.contains("internalQuotedPhotons"));QCOMPARE(client.providerUsage().first().toMap().value("internalQuotedPhotons").toDouble(),2.0);
  QCOMPARE(server.paidCalls,1);
  QVERIFY(client.providerUsage().first().toMap().value("internalChargedPhotons").isNull());
  LampyreConnection reopened(t.path(),nullptr,nullptr,nullptr,true);QCOMPARE(reopened.invoices().size(),1);QCOMPARE(reopened.invoices().first().toMap().value("id"),invoice.value("id"));
  QCOMPARE(server.lastArguments.value("tasks").toArray().size(),1);QVERIFY(client.exportResult(t.filePath("result.json"),true));
  QFile output(t.filePath("result.json"));QVERIFY(output.open(QIODevice::ReadOnly));const auto quote=QJsonDocument::fromJson(output.readAll()).object().value("retailQuote").toObject();
  QCOMPARE(quote.value("minorUnits").toInt(),1400);QCOMPARE(quote.value("collectionStatus").toString(),QString("not-collected-by-application"));
  server.failPaid=true;QVERIFY(client.runService("synthetic_email_lookup","{\"email\":\"research@example.invalid\"}",2,true));
  QTRY_COMPARE_WITH_TIMEOUT(client.state(),QString("provider-tool-error"),3000);QCOMPARE(client.invoices().size(),2);
  QCOMPARE(client.invoices().first().toMap().value("amountDueMinorUnits").toInt(),0);QCOMPARE(client.invoices().first().toMap().value("status").toString(),QString("void"));
  QCOMPARE(client.invoices().at(1).toMap().value("amountDueMinorUnits").toInt(),1400);

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
