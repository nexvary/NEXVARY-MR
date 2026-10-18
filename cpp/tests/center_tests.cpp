#include <QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include "WorkshopBridge.hpp"
#include "TrackingPortal.hpp"
class CenterTests:public QObject {
 Q_OBJECT
private slots:
 void profilesPersistWithoutDuplicateOwnership(){
  QTemporaryDir t;const auto path=t.filePath("center.db");int device=0,customer=0;
  {WorkshopBridge w(path);QVERIFY(w.ready());const int first=w.createOrder("Synthetic owner","010-test","Samsung","SM-test","serial-private","No charge","T","private notes");QVERIFY(first>0);
   const auto o=w.invoice(first);customer=o.value("customer_id").toInt();device=o.value("device_id").toInt();QVERIFY(customer>0);QVERIFY(device>0);
   const int repeat=w.createOrderForDevice(device,"USB","Other technician","Second visit");QVERIFY(repeat>first);QCOMPARE(w.customers().size(),1);QCOMPARE(w.devices().size(),1);
   const int otherPhone=w.createOrderForCustomer(customer,"OPPO","CPH-test","serial-2","Screen","Tech","");QVERIFY(otherPhone>repeat);QCOMPARE(w.customers().size(),1);QCOMPARE(w.devices().size(),2);
   QCOMPARE(w.customerProfile(customer).value("orders").toList().size(),3);QCOMPARE(w.deviceProfile(device).value("orders").toList().size(),2);
   QVERIFY(w.updateCustomer(customer,QStringLiteral("عميل اصطناعي"),"new-contact"));QCOMPARE(w.deviceProfile(device).value("owner").toString(),QStringLiteral("عميل اصطناعي"));
   QVERIFY(!w.updateCustomer(customer," ","x"));QVERIFY(w.createOrderForDevice(99999,"USB","T","")<0);QVERIFY(w.createOrderForCustomer(99999,"B","M","","USB","T","")<0);QCOMPARE(w.devices().size(),2);QCOMPARE(w.orders().size(),3);QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));}
  WorkshopBridge w(path);QVERIFY(w.ready());QCOMPARE(w.customerProfile(customer).value("orders").toList().size(),3);QCOMPARE(w.deviceProfile(device).value("orders").toList().size(),2);
 }
 void trackingPrivacyRotationAndRestore(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("center.db"));const int id=w.createOrder("DO-NOT-PUBLISH-NAME","DO-NOT-PUBLISH-PHONE","B","M","DO-NOT-PUBLISH-SERIAL","DO-NOT-PUBLISH-FAULT","PRIVATE-TECH","DO-NOT-PUBLISH-NOTES");QVERIFY(id>0);
  QVERIFY(w.issueTrackingToken(id,30,false).isEmpty());QVERIFY(w.issueTrackingToken(id,91,true).isEmpty());QVERIFY(w.issueTrackingToken(999,30,true).isEmpty());
  const auto token=w.issueTrackingToken(id,30,true);QCOMPARE(token.size(),64);QVERIFY(!w.publicTracking(token).isEmpty());
  QVERIFY(w.updateOrder(id,"Repairing","PRIVATE-TECH",1200,"DO-NOT-PUBLISH-NOTES"));QVERIFY(w.setPublicMessage(id,"<script>alert('x')</script>"));
  const auto snapshot=w.publicTracking(token);QCOMPARE(snapshot.value("status").toString(),QStringLiteral("Repairing"));QCOMPARE(snapshot.value("history").toList().size(),2);
  const auto html=TrackingPortal::html(snapshot);QVERIFY(html.contains("&lt;script&gt;"));QVERIFY(!html.contains("<script>"));QVERIFY(!html.contains("DO-NOT-PUBLISH"));QVERIFY(!html.contains("PRIVATE-TECH"));QVERIFY(!html.contains("1200"));
  QVERIFY(w.backup(t.filePath("saved.db")));QFile db(w.databasePath());QVERIFY(db.open(QIODevice::ReadOnly));QVERIFY(!db.readAll().contains(token.toLatin1()));db.close();
  const auto next=w.issueTrackingToken(id,1,true);QVERIFY(next!=token);QVERIFY(w.publicTracking(token).isEmpty());QVERIFY(!w.publicTracking(next).isEmpty());
  QVERIFY(w.revokeTracking(id));QVERIFY(w.publicTracking(next).isEmpty());QVERIFY(w.restore(t.filePath("saved.db"),true));QVERIFY(w.publicTracking(token).isEmpty());QCOMPARE(w.orders().size(),1);
 }
 void realHttpPortal(){
  QTemporaryDir t;WorkshopBridge w(t.filePath("center.db"));const int id=w.createOrder("Hidden name","Hidden contact","B","M","Hidden serial","Hidden fault","Hidden tech","Hidden notes");QVERIFY(id>0);TrackingPortal p(&w);
  QVERIFY(!p.start("0.0.0.0",0,true));QVERIFY(!p.start("127.0.0.1",0,false));QVERIFY(!p.start("8.8.8.8",80,true));QVERIFY(p.start("127.0.0.1",0,true));
  const auto url=p.link(id,30,true);QVERIFY(url.startsWith(p.baseUrl()+"/t/"));QNetworkAccessManager network;
  auto get=[&network](const QString &u,bool wrongHost=false){QNetworkRequest request{QUrl(u)};if(wrongHost)request.setRawHeader("Host","untrusted.invalid");auto *reply=network.get(request);QEventLoop loop;QTimer::singleShot(3000,&loop,&QEventLoop::quit);QObject::connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);loop.exec();const auto body=reply->readAll();const int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();const auto cache=reply->rawHeader("Cache-Control");reply->deleteLater();return QVariantMap{{"code",code},{"body",body},{"cache",cache}};};
  auto r=get(url);QCOMPARE(r.value("code").toInt(),200);QCOMPARE(r.value("cache").toByteArray(),QByteArray("no-store"));QVERIFY(!r.value("body").toByteArray().contains("Hidden"));
  QVERIFY(w.updateOrder(id,"Ready","T",0,"Private"));r=get(url);QVERIFY(r.value("body").toByteArray().contains("Ready"));
  QCOMPARE(get(url,true).value("code").toInt(),404);QCOMPARE(get(p.baseUrl()+"/t/1").value("code").toInt(),404);QVERIFY(w.revokeTracking(id));QCOMPARE(get(url).value("code").toInt(),404);p.stop();QVERIFY(!p.running());QVERIFY(p.link(id,30,true).isEmpty());
 }
 void migrationV2AndExpiry(){
  QTemporaryDir t;const auto path=t.filePath("v2.db");int order=0;
  {WorkshopBridge w(path);order=w.createOrder("C","P","B","M","S","USB","T","");QVERIFY(order>0);}
  const QString conn="center-expiry";
  {auto db=QSqlDatabase::addDatabase("QSQLITE",conn);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("DROP TABLE tracking_links"));QVERIFY(q.exec("PRAGMA user_version=2"));db.close();}QSqlDatabase::removeDatabase(conn);
  WorkshopBridge w(path);QVERIFY2(w.ready(),qPrintable(w.error()));QCOMPARE(w.orders().size(),1);const auto token=w.issueTrackingToken(order,1,true);QVERIFY(!token.isEmpty());
  {auto db=QSqlDatabase::addDatabase("QSQLITE",conn);db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("UPDATE tracking_links SET expires_at='2000-01-01T00:00:00Z'"));db.close();}QSqlDatabase::removeDatabase(conn);
  QVERIFY(w.publicTracking(token).isEmpty());QCOMPARE(w.checkIntegrity(),QStringLiteral("ok"));
 }
};
QTEST_MAIN(CenterTests)
#include "center_tests.moc"
