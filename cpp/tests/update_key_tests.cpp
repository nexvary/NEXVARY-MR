#include "UpdateKeyStore.hpp"
#include "CaseVault.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QLockFile>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
using namespace mr::update;
namespace {
JournalAnchor bootstrap(){return {QString(32,QLatin1Char('a')),{"0.4.0",5},0};}
#ifdef Q_OS_WIN
QByteArray read(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray{};}
bool write(const QString &path,const QByteArray &bytes){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();}
QString keyPath(const QString &root){return QDir(root).filePath("update-auth.dpapi");}
#endif
}
class UpdateKeyTests final:public QObject{
 Q_OBJECT
private slots:
 void availabilityAndNoPlaintextFallback(){
#ifdef Q_OS_WIN
  QVERIFY(KeyStore::available());
#else
  QTemporaryDir root;
  QVERIFY(!KeyStore::available());const auto created=KeyStore::create(root.path(),bootstrap());
  QCOMPARE(created.error,QStringLiteral("key-store-unavailable"));QVERIFY(created.report.isEmpty());
  const auto opened=KeyStore::openJournal(root.path(),bootstrap());QCOMPARE(opened.error,QStringLiteral("key-store-unavailable"));QVERIFY(!opened.journal);
  QVERIFY(QDir(root.path()).entryList(QDir::AllEntries|QDir::NoDotAndDotDot).isEmpty());
#endif
 }
#ifdef Q_OS_WIN
 void currentUserRoundTripAndJournalAuthentication(){
  QTemporaryDir parent;const auto root=parent.filePath("private state with spaces");QVERIFY(QDir().mkdir(root));
  const auto created=KeyStore::create(QUrl::fromLocalFile(root).toString(),bootstrap());QVERIFY2(created.accepted(),qPrintable(created.error));
  QVERIFY(created.report["currentUserProtected"].toBool());
  for(const auto *flag:{"publisherKeyCreated","installationAuthorized","protectedAnchorPersisted"})QVERIFY(!created.report[flag].toBool(true));
  const auto bytes=read(keyPath(root));QVERIFY(!bytes.isEmpty());QVERIFY(!bytes.contains("local journal authentication key"));QVERIFY(!bytes.contains("production-read-only"));QVERIFY(!bytes.contains("laboratory-experimental"));
  auto first=KeyStore::openJournal(root,bootstrap());QVERIFY2(first.accepted(),qPrintable(first.error));const auto initialized=first.journal->initialize();QVERIFY2(initialized.accepted(),qPrintable(initialized.error));
  auto anchor=bootstrap();anchor.revisionFloor=1;auto reopened=KeyStore::openJournal(root,anchor);QVERIFY(reopened.accepted());
  const auto loaded=reopened.journal->read();QVERIFY2(loaded.accepted(),qPrintable(loaded.error));QCOMPARE(loaded.report,initialized.report);
 }
 void noOverwriteOrSilentRegeneration(){
  QTemporaryDir root;QVERIFY(write(root.filePath("keep.txt"),"caller owned"));QVERIFY(KeyStore::create(root.path(),bootstrap()).accepted());
  const auto before=QCryptographicHash::hash(read(keyPath(root.path())),QCryptographicHash::Sha256);
  QCOMPARE(KeyStore::create(root.path(),bootstrap()).error,QStringLiteral("key-store-already-initialized"));
  QVERIFY(QCryptographicHash::hash(read(keyPath(root.path())),QCryptographicHash::Sha256)==before);
  auto handle=KeyStore::openJournal(root.path(),bootstrap());QVERIFY(handle.accepted());QVERIFY(handle.journal->initialize().accepted());const auto journal=read(root.filePath("update-state.json"));
  QVERIFY(QFile::remove(keyPath(root.path())));auto missing=KeyStore::openJournal(root.path(),bootstrap());QVERIFY(!missing.accepted());QVERIFY(!QFile::exists(keyPath(root.path())));
  QCOMPARE(KeyStore::create(root.path(),bootstrap()).error,QStringLiteral("key-store-already-initialized"));QVERIFY(read(root.filePath("update-state.json"))==journal);
  QVERIFY(read(root.filePath("keep.txt"))==QByteArray("caller owned"));
 }
 void corruptionAndScopeRefuseWithoutReplacement(){
  QTemporaryDir root;QVERIFY(KeyStore::create(root.path(),bootstrap()).accepted());const auto original=read(keyPath(root.path()));
  auto wrong=bootstrap();wrong.installationId=QString(32,QLatin1Char('b'));auto foreign=KeyStore::openJournal(root.path(),wrong);QCOMPARE(foreign.error,QStringLiteral("key-store-scope"));QVERIFY(!foreign.journal);
  auto altered=original;altered[altered.size()-1]=static_cast<char>(altered.back()^1);QVERIFY(write(keyPath(root.path()),altered));
  auto corrupt=KeyStore::openJournal(root.path(),bootstrap());QVERIFY(!corrupt.accepted());QVERIFY(!corrupt.error.isEmpty());
  QCOMPARE(KeyStore::create(root.path(),bootstrap()).error,QStringLiteral("key-store-already-initialized"));QVERIFY(read(keyPath(root.path()))==altered);
  QVERIFY(write(keyPath(root.path()),QByteArray(8193,'X')));QCOMPARE(KeyStore::openJournal(root.path(),bootstrap()).error,QStringLiteral("key-store-read"));
  QVERIFY(write(keyPath(root.path()),"plaintext must not be accepted"));QCOMPARE(KeyStore::openJournal(root.path(),bootstrap()).error,QStringLiteral("key-store-format"));
  QString error;const auto unrelated=mr::recovery::sealCase("unrelated protected recovery fixture",false,error);QVERIFY(!unrelated.isEmpty());
  QVERIFY(write(keyPath(root.path()),QByteArray("NMR-UPDATE-KEY-DPAPI-1\n")+unrelated));QCOMPARE(KeyStore::openJournal(root.path(),bootstrap()).error,QStringLiteral("key-store-scope"));
  QVERIFY(write(keyPath(root.path()),original));QVERIFY(KeyStore::openJournal(root.path(),bootstrap()).accepted());
 }
 void invalidBootstrapAndPathsLeaveNoKey(){
  QTemporaryDir root;auto protectedState=bootstrap();protectedState.revisionFloor=1;
  QCOMPARE(KeyStore::create(root.path(),protectedState).error,QStringLiteral("key-store-bootstrap-forbidden"));
  auto invalid=bootstrap();invalid.installationId="invalid";QCOMPARE(KeyStore::create(root.path(),invalid).error,QStringLiteral("key-store-scope"));
  for(const auto &path:QStringList{"https://example.invalid/store",root.filePath("missing"),"file://server/share/key"})QCOMPARE(KeyStore::create(path,bootstrap()).error,QStringLiteral("key-store-directory"));
  QVERIFY(QDir(root.path()).entryList(QDir::AllEntries|QDir::NoDotAndDotDot).isEmpty());
 }
 void liveJournalLockBlocksKeyAccess(){
  QTemporaryDir root;const auto path=root.filePath("update-state.lock");
  {QLockFile lock(path);lock.setStaleLockTime(0);QVERIFY(lock.tryLock(0));QCOMPARE(KeyStore::create(root.path(),bootstrap()).error,QStringLiteral("key-store-busy"));}
  QVERIFY(KeyStore::create(root.path(),bootstrap()).accepted());
  {QLockFile lock(path);lock.setStaleLockTime(0);QVERIFY(lock.tryLock(0));auto handle=KeyStore::openJournal(root.path(),bootstrap());QCOMPARE(handle.error,QStringLiteral("key-store-busy"));QVERIFY(!handle.journal);}
  QVERIFY(KeyStore::openJournal(root.path(),bootstrap()).accepted());
 }
#endif
};
QTEST_GUILESS_MAIN(UpdateKeyTests)
#include "update_key_tests.moc"
