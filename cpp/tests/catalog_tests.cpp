#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include "ServiceCatalog.hpp"
class CatalogTests:public QObject {
 Q_OBJECT
private slots:
 void exactMatchAndValidation(){
  QTemporaryDir t;const auto storage=t.filePath("guides.json"),input=t.filePath("import.json");ServiceCatalog c(storage);QCOMPARE(c.drivers().size(),6);
  for(const auto &d:c.drivers()){const QUrl u(d.toMap().value("url").toString());QCOMPARE(u.scheme(),QStringLiteral("https"));QVERIFY(!u.host().isEmpty());}
  QVariantMap leaf{{"id","end"},{"terminal",true},{"ar",QStringLiteral("نتيجة تدريبية")},{"en","Training outcome"}};
  QVariantMap root{{"id","start"},{"terminal",false},{"ar",QStringLiteral("سؤال تدريبي؟")},{"en","Training question?"},{"yes","end"},{"no","end"}};
  QVariantMap guide{{"schemaVersion",1},{"brand","SYNTHETIC"},{"model","TRAINING-01"},{"symptom","USB not detected"},{"sourceUrl","https://example.invalid/synthetic-guide"},{"license","Synthetic test fixture only"},{"root","start"},{"nodes",QVariantList{root,leaf}}};
  QVERIFY(ServiceCatalog::validGuide(guide));auto cycle=guide;root["yes"]="start";cycle["nodes"]=QVariantList{root,leaf};QVERIFY(!ServiceCatalog::validGuide(cycle));
  auto wrong=guide;wrong["sourceUrl"]="javascript:alert(1)";QVERIFY(!ServiceCatalog::validGuide(wrong));
  QFile f(input);QVERIFY(f.open(QIODevice::WriteOnly));f.write(QJsonDocument::fromVariant(guide).toJson());f.close();QVERIFY(!c.importGuide(input,false));QVERIFY(c.importGuide(input,true));QCOMPARE(c.guides().size(),1);
  QCOMPARE(c.tree("synthetic","training-01","USB not detected",false).value("root").toString(),QStringLiteral("start"));
  QCOMPARE(c.tree("synthetic","training-02","USB not detected",false).value("root").toString(),QStringLiteral("safety"));QVERIFY(c.tree("synthetic","training-02","USB not detected",false).value("scope").toString().contains("not an exact-model"));
  ServiceCatalog reopened(storage);QCOMPARE(reopened.guides().size(),1);QVERIFY(reopened.tree("","","USB",false).isEmpty());
  for(const auto &symptom:QStringList{"USB not detected","Not charging","Bootloop","No network"}){auto generic=c.tree("Brand","Unknown-model",symptom,false);generic["schemaVersion"]=1;generic["license"]="Original triage";generic["sourceUrl"]="https://example.invalid/test";QVERIFY2(ServiceCatalog::validGuide(generic),qPrintable(symptom));}
 }
};
QTEST_MAIN(CatalogTests)
#include "catalog_tests.moc"
