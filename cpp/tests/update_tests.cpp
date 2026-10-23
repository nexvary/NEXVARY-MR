#include "UpdateVerifier.hpp"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <array>
#include <limits>
#include <future>
#include <thread>
#include <sodium.h>
#ifndef Q_OS_WIN
#include <sys/stat.h>
#endif

using mr::update::Verifier;
class UpdateTests final : public QObject {
  Q_OBJECT
  std::array<unsigned char,crypto_sign_PUBLICKEYBYTES> public_{};
  std::array<unsigned char,crypto_sign_SECRETKEYBYTES> secret_{};
  const QByteArray payload_ = QByteArrayLiteral("SYNTHETIC DESKTOP UPDATE FIXTURE\n");
  QDateTime now() const { return QDateTime::fromString("2026-10-23T12:00:00Z",Qt::ISODate); }
  mr::update::TrustedKeys keys() const {
    return {{"synthetic",QByteArray(reinterpret_cast<const char *>(public_.data()),static_cast<qsizetype>(public_.size()))}};
  }
  Verifier verifier() const { return Verifier(keys(),{"0.4.0",5}); }
  QJsonObject manifest() const {
    return {{"schema","nexvary-update/v1"},{"product","NEXVARY-MR"},{"platform","windows-x64"},
      {"profile",Verifier::compiledProfile()},{"channel","stable"},{"keyId","synthetic"},
      {"version","0.4.1"},{"sequence","6"},{"packageKind","portable-zip"},
      {"packageBytes",QString::number(payload_.size())},
      {"sha256",QString::fromLatin1(QCryptographicHash::hash(payload_,QCryptographicHash::Sha256).toHex())},
      {"issuedUtc","2026-10-23T11:00:00Z"},{"expiresUtc","2026-10-24T12:00:00Z"}};
  }
  QByteArray compact(const QJsonObject &object) const { return QJsonDocument(object).toJson(QJsonDocument::Compact); }
  QByteArray sign(const QByteArray &bytes,bool domain=true) const {
    const auto message = domain ? Verifier::signingMessage(bytes) : bytes;
    QByteArray signature(crypto_sign_BYTES,'\0');
    if (crypto_sign_detached(reinterpret_cast<unsigned char *>(signature.data()),nullptr,
         reinterpret_cast<const unsigned char *>(message.constData()),static_cast<unsigned long long>(message.size()),secret_.data()) != 0) return {};
    return signature;
  }
  static bool write(const QString &path,const QByteArray &bytes) {
    QFile file(path); return file.open(QIODevice::WriteOnly) && file.write(bytes)==bytes.size();
  }
  static QByteArray read(const QString &path) {
    QFile file(path); return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray{};
  }
private slots:
  void initTestCase() { QVERIFY(sodium_init()>=0); QCOMPARE(crypto_sign_keypair(public_.data(),secret_.data()),0); }
  void cleanupTestCase() { sodium_memzero(secret_.data(),secret_.size()); }
  void independentlySignedReference() {
    QFile input(QString::fromUtf8(MR_UPDATE_REFERENCE_PATH)); QVERIFY(input.open(QIODevice::ReadOnly));
    const auto fixture=QJsonDocument::fromJson(input.readAll()).object(); QVERIFY(fixture["fixtureOnly"].toBool());
    const auto publicKey=QByteArray::fromHex(fixture["publicKeyHex"].toString().toLatin1());
    Verifier reference({{"synthetic-reference",publicKey}},{"0.4.0",5});
    for(const auto &value:fixture["vectors"].toArray()) {
      const auto row=value.toObject();const auto bytes=row["manifestUtf8"].toString().toUtf8();
      const auto signature=QByteArray::fromHex(row["signatureHex"].toString().toLatin1());
      const auto result=reference.checkManifest(bytes,signature,now());
      if(row["profile"].toString()==Verifier::compiledProfile()) {
        QVERIFY2(result.accepted(),qPrintable(result.error)); QVERIFY(result.report["signatureVerified"].toBool());
        QTemporaryDir directory;const auto path=directory.filePath("independent.bin");
        QVERIFY(write(path,QByteArray::fromBase64(fixture["packageBase64"].toString().toLatin1())));
        const auto package=reference.checkPackage(bytes,signature,path,now());QVERIFY2(package.accepted(),qPrintable(package.error));
        QVERIFY(package.report["packageHashVerified"].toBool());QVERIFY(!package.report["installationAuthorized"].toBool(true));
      } else QCOMPARE(result.error,QStringLiteral("build-profile-mismatch"));
    }
  }
  void validSignatureDoesNotAuthorizeInstallation() {
    const auto bytes=compact(manifest());const auto result=verifier().checkManifest(bytes,sign(bytes),now());
    QVERIFY2(result.accepted(),qPrintable(result.error));QVERIFY(result.report["signatureVerified"].toBool());
    for(const auto *flag:{"packageHashVerified","installationAuthorized","rollbackStatePersisted","publisherIdentityCertified"})
      QCOMPARE(result.report[flag].toBool(true),false);
    QVERIFY(!result.report.contains("publicKey"));
  }
  void trustAndSignatureFailures() {
    const auto bytes=compact(manifest()),signature=sign(bytes);
    auto rejected=[&](const mr::update::CheckResult &result,const char *code){QCOMPARE(result.error,QString::fromLatin1(code));QVERIFY(result.report.isEmpty());};
    rejected(Verifier({},{"0.4.0",5}).checkManifest(bytes,signature,now()),"no-approved-trust-anchors");
    rejected(Verifier({{"synthetic","bad-key"}},{"0.4.0",5}).checkManifest(bytes,signature,now()),"invalid-trust-store");
    auto tooMany=keys();for(int i=0;i<16;++i)tooMany.insert(QString("extra-%1").arg(i),tooMany.first());
    rejected(Verifier(tooMany,{"0.4.0",5}).checkManifest(bytes,signature,now()),"invalid-trust-store");
    rejected(verifier().checkManifest(bytes,signature.left(63),now()),"signature-size");
    auto altered=signature;altered[0]=static_cast<char>(altered[0]^1);
    rejected(verifier().checkManifest(bytes,altered,now()),"invalid-signature");
    rejected(verifier().checkManifest(bytes,sign(bytes,false),now()),"invalid-signature");
    auto object=manifest();object["version"]="0.4.2";
    rejected(verifier().checkManifest(compact(object),signature,now()),"invalid-signature");
    object=manifest();object["keyId"]="unknown";const auto unknown=compact(object);
    rejected(verifier().checkManifest(unknown,sign(unknown),now()),"unapproved-signing-key");
    std::array<unsigned char,crypto_sign_PUBLICKEYBYTES> otherPublic{};
    std::array<unsigned char,crypto_sign_SECRETKEYBYTES> otherSecret{};QCOMPARE(crypto_sign_keypair(otherPublic.data(),otherSecret.data()),0);
    rejected(Verifier({{"synthetic",QByteArray(reinterpret_cast<const char *>(otherPublic.data()),32)}},{"0.4.0",5}).checkManifest(bytes,signature,now()),"invalid-signature");
    sodium_memzero(otherSecret.data(),otherSecret.size());
  }
  void signedPolicyViolations_data() {
    QTest::addColumn<QString>("field");QTest::addColumn<QJsonValue>("value");QTest::addColumn<QString>("error");
    auto row=[](const char *name,const char *field,const QJsonValue &value,const char *error){QTest::newRow(name)<<QString::fromLatin1(field)<<value<<QString::fromLatin1(error);};
    row("schema","schema","nexvary-update/v2","unsupported-target");
    row("product","product","OTHER","unsupported-target");row("platform","platform","windows-arm64","unsupported-target");
    row("channel","channel","nightly","unsupported-target");
    row("profile","profile",Verifier::compiledProfile()=="production-read-only"?"laboratory-experimental":"production-read-only","build-profile-mismatch");
    row("kind","packageKind","shell-command","package-kind");
    row("extra-field","downloadCommand","execute something","manifest-fields");
    row("key-in-message","publicKey","untrusted","manifest-fields");
    row("wrong-type","sequence",6,"manifest-fields");
    row("version-equal","version","0.4.0","version-not-newer");row("version-downgrade","version","0.3.99","version-not-newer");
    row("version-leading-zero","version","0.04.1","invalid-version");row("version-overflow","version","65536.0.0","invalid-version");
    row("prerelease","version","0.4.1-rc1","invalid-version");
    row("sequence-equal","sequence","5","sequence-not-newer");row("sequence-old","sequence","4","sequence-not-newer");
    row("sequence-overflow","sequence","18446744073709551616","sequence-not-newer");row("sequence-leading-zero","sequence","06","sequence-not-newer");
    row("size-zero","packageBytes","0","package-size-limit");row("size-big","packageBytes","536870913","package-size-limit");
    row("hash-case","sha256",QString(64,'A'),"invalid-package-hash");row("hash-short","sha256","abc","invalid-package-hash");
    row("future-issue","issuedUtc","2026-10-23T12:00:01Z","manifest-time-window");
    row("expired","expiresUtc","2026-10-23T12:00:00Z","manifest-time-window");
    row("expiry-before-issue","expiresUtc","2026-10-22T12:00:00Z","manifest-time-window");
    row("long-validity","expiresUtc","2026-12-01T12:00:00Z","manifest-time-window");
    row("no-timezone","expiresUtc","2026-10-24T12:00:00","manifest-time-window");
    row("fractional-time","expiresUtc","2026-10-24T12:00:00.001Z","manifest-time-window");
  }
  void signedPolicyViolations() {
    QFETCH(QString,field);QFETCH(QJsonValue,value);QFETCH(QString,error);
    auto object=manifest();object[field]=value;const auto bytes=compact(object);
    const auto result=verifier().checkManifest(bytes,sign(bytes),now());QCOMPARE(result.error,error);QVERIFY(result.report.isEmpty());
  }
  void canonicalEncodingAndState() {
    const auto bytes=compact(manifest());
    for(const auto &invalid:QList<QByteArray>{" "+bytes,bytes+"\n",QByteArray("{\"keyId\":\"synthetic\",")+bytes.mid(1)}) {
      const auto result=verifier().checkManifest(invalid,sign(invalid),now());QCOMPARE(result.error,QStringLiteral("noncanonical-manifest"));QVERIFY(result.report.isEmpty());
    }
    const auto missing=verifier().checkManifest("{}",sign("{}"),now());QVERIFY(!missing.accepted());
    QCOMPARE(verifier().checkManifest(QByteArray(mr::update::maximumManifestBytes+1,'x'),sign(bytes),now()).error,QStringLiteral("manifest-size-limit"));
    QCOMPARE(verifier().checkManifest("[]",sign("[]"),now()).error,QStringLiteral("manifest-json"));
    QCOMPARE(Verifier(keys(),{"invalid",0}).checkManifest(bytes,sign(bytes),now()).error,QStringLiteral("invalid-installed-state"));
    QCOMPARE(Verifier(keys(),{"0.4.0",std::numeric_limits<quint64>::max()}).checkManifest(bytes,sign(bytes),now()).error,QStringLiteral("sequence-not-newer"));
    QCOMPARE(verifier().checkManifest(bytes,sign(bytes),{}).error,QStringLiteral("manifest-time-window"));
    auto object=manifest();object["version"]="0.4.10";object["sequence"]="18446744073709551615";
    const auto larger=compact(object);QVERIFY(verifier().checkManifest(larger,sign(larger),now()).accepted());
  }
  void localHashAndSourcePreservation() {
    QTemporaryDir dir;const auto path=dir.filePath("private source with spaces.bin");QVERIFY(write(path,payload_));
    const auto bytes=compact(manifest()),signature=sign(bytes);
    const auto result=verifier().checkPackage(bytes,signature,QUrl::fromLocalFile(path).toString(),now());
    QVERIFY2(result.accepted(),qPrintable(result.error));QVERIFY(result.report["packageHashVerified"].toBool());
    for(const auto *flag:{"installationAuthorized","installed","immutableSnapshotCreated","authenticodeVerified","rollbackStatePersisted"})QCOMPARE(result.report[flag].toBool(true),false);
    QVERIFY(!QJsonDocument(result.report).toJson().contains("private source"));QCOMPARE(read(path),payload_);
    auto corrupt=payload_;corrupt[0]='X';QVERIFY(write(path,corrupt));
    const auto changed=verifier().checkPackage(bytes,signature,path,now());QCOMPARE(changed.error,QStringLiteral("package-hash-mismatch"));QVERIFY(changed.report.isEmpty());
    QVERIFY(write(path,payload_+"x"));QCOMPARE(verifier().checkPackage(bytes,signature,path,now()).error,QStringLiteral("package-size-mismatch"));
    for(const auto &bad:QStringList{"https://example.invalid/package.zip","file://server/share/package.zip",dir.path(),dir.filePath("missing")})
      QCOMPARE(verifier().checkPackage(bytes,signature,bad,now()).error,QStringLiteral("local-regular-package-required"));
#ifndef Q_OS_WIN
    const auto alias=dir.filePath("alias");QVERIFY(QFile::link(path,alias));
    QCOMPARE(verifier().checkPackage(bytes,signature,alias,now()).error,QStringLiteral("local-regular-package-required"));
#endif
  }
  void authenticationAndCancellationPrecedeFileOpen() {
    QTemporaryDir dir;const auto path=dir.filePath("not-created");const auto bytes=compact(manifest());
    QCOMPARE(verifier().checkPackage(bytes,QByteArray(64,'\0'),path,now()).error,QStringLiteral("invalid-signature"));
    const auto token=std::make_shared<std::atomic_bool>(true);
    QCOMPARE(verifier().checkPackage(bytes,sign(bytes),path,now(),token).error,QStringLiteral("cancelled"));
#ifndef Q_OS_WIN
    const auto pipe=dir.filePath("fifo");QVERIFY(::mkfifo(QFile::encodeName(pipe).constData(),0600)==0);QElapsedTimer elapsed;elapsed.start();
    QCOMPARE(verifier().checkPackage(bytes,sign(bytes),pipe,now()).error,QStringLiteral("local-regular-package-required"));
    QVERIFY(elapsed.elapsed()<1000);
#endif
    QVERIFY(!QFile::exists(path));
  }
  void stagingRetainsVerifiedBytesAndCleansUp() {
    QTemporaryDir source,root;const auto path=source.filePath("replaceable.bin");QVERIFY(write(path,payload_));
    const auto unrelated=root.filePath("keep.txt");QVERIFY(write(unrelated,"caller-owned file"));
    const auto bytes=compact(manifest());
    auto staged=verifier().stagePackage(bytes,sign(bytes),path,root.path(),now());
    QVERIFY2(staged.accepted(),qPrintable(staged.check.error));
    QVERIFY(staged.check.report["stagedCopyCreated"].toBool());QVERIFY(staged.check.report["retainedHandleVerified"].toBool());
    QVERIFY(!staged.check.report["installationAuthorized"].toBool(true));QVERIFY(!staged.check.report["immutableSnapshotCreated"].toBool(true));
    QVERIFY(!QJsonDocument(staged.check.report).toJson().contains(root.path().toUtf8()));
    const auto children=QDir(root.path()).entryList(QDir::Dirs|QDir::NoDotAndDotDot);QCOMPARE(children.size(),1);
    const auto copied=QDir(root.path()).filePath(children.first()+"/package.bin");QCOMPARE(read(copied),payload_);
#ifndef Q_OS_WIN
    QVERIFY(!(QFile::permissions(copied)&(QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther)));
    QVERIFY(!(QFile::permissions(QDir(root.path()).filePath(children.first()))&(QFileDevice::ReadGroup|QFileDevice::WriteGroup|QFileDevice::ReadOther|QFileDevice::WriteOther)));
#endif
    QVERIFY(QFile::remove(path));QVERIFY(write(path,"replaced source"));
    for(int i=0;i<2;++i)QVERIFY(staged.snapshot->recheck(verifier(),now()).accepted());
    staged.snapshot.reset();QCOMPARE(QDir(root.path()).entryList(QDir::AllEntries|QDir::NoDotAndDotDot),QStringList{"keep.txt"});
    QCOMPARE(read(unrelated),QByteArray("caller-owned file"));
    QCOMPARE(read(path),QByteArray("replaced source"));
  }
  void stagingRechecksCurrentTrustStateAndActualCopy() {
    QTemporaryDir source,root;const auto path=source.filePath("source.bin");QVERIFY(write(path,payload_));const auto bytes=compact(manifest());
    auto staged=verifier().stagePackage(bytes,sign(bytes),path,root.path(),now());QVERIFY(staged.accepted());
    QCOMPARE(staged.snapshot->recheck(Verifier({},{"0.4.0",5}),now()).error,QStringLiteral("no-approved-trust-anchors"));
    QCOMPARE(staged.snapshot->recheck(Verifier(keys(),{"0.4.0",6}),now()).error,QStringLiteral("sequence-not-newer"));
    QCOMPARE(staged.snapshot->recheck(verifier(),now().addDays(2)).error,QStringLiteral("manifest-time-window"));
    const auto token=std::make_shared<std::atomic_bool>(true);
    QCOMPARE(staged.snapshot->recheck(verifier(),now(),token).error,QStringLiteral("cancelled"));
    const auto child=QDir(root.path()).entryList(QDir::Dirs|QDir::NoDotAndDotDot).first();
    auto changed=payload_;changed[0]='X';QVERIFY(write(QDir(root.path()).filePath(child+"/package.bin"),changed));
    const auto result=staged.snapshot->recheck(verifier(),now());QCOMPARE(result.error,QStringLiteral("package-hash-mismatch"));QVERIFY(result.report.isEmpty());
  }
  void stagingFailuresLeaveNoCopy() {
    QTemporaryDir source,root;const auto path=source.filePath("source.bin");QVERIFY(write(path,payload_));const auto bytes=compact(manifest()),signature=sign(bytes);
    auto empty=[&]{QVERIFY(QDir(root.path()).entryList(QDir::AllEntries|QDir::NoDotAndDotDot).isEmpty());};
    auto invalid=verifier().stagePackage(bytes,QByteArray(64,'\0'),"missing",root.filePath("not-created"),now());
    QCOMPARE(invalid.check.error,QStringLiteral("invalid-signature"));QVERIFY(!invalid.snapshot);empty();
    const auto token=std::make_shared<std::atomic_bool>(true);
    QCOMPARE(verifier().stagePackage(bytes,signature,path,root.path(),now(),token).check.error,QStringLiteral("cancelled"));empty();
    for(const auto &bad:QStringList{"https://example.invalid/stage",root.filePath("missing"),path})
      QCOMPARE(verifier().stagePackage(bytes,signature,path,bad,now()).check.error,QStringLiteral("local-staging-directory-required"));
#ifndef Q_OS_WIN
    const auto alias=source.filePath("stage-alias");QVERIFY(QFile::link(root.path(),alias));
    QCOMPARE(verifier().stagePackage(bytes,signature,path,alias,now()).check.error,QStringLiteral("local-staging-directory-required"));empty();
#endif
    auto corrupt=payload_;corrupt[0]='X';QVERIFY(write(path,corrupt));
    auto rejected=verifier().stagePackage(bytes,signature,path,root.path(),now());
    QCOMPARE(rejected.check.error,QStringLiteral("package-hash-mismatch"));QVERIFY(!rejected.snapshot);QVERIFY(rejected.check.report.isEmpty());empty();
    QCOMPARE(read(path),corrupt);
  }
  void cancelDuringStagingRemovesPartialCopy() {
    QTemporaryDir source,root;const auto path=source.filePath("large.bin");
    QFile input(path);QVERIFY(input.open(QIODevice::WriteOnly));const QByteArray chunk(1024*1024,'S');QCryptographicHash hash(QCryptographicHash::Sha256);
    for(int i=0;i<64;++i){QCOMPARE(input.write(chunk),chunk.size());hash.addData(chunk);}input.close();
    auto object=manifest();object["packageBytes"]=QString::number(64*1024*1024);object["sha256"]=QString::fromLatin1(hash.result().toHex());
    const auto bytes=compact(object),signature=sign(bytes);const auto token=std::make_shared<std::atomic_bool>(false);const auto policy=verifier();const auto clock=now();
    auto work=std::async(std::launch::async,[&]{return policy.stagePackage(bytes,signature,path,root.path(),clock,token);});
    QElapsedTimer elapsed;elapsed.start();bool observed=false;
    while(elapsed.elapsed()<5000&&work.wait_for(std::chrono::milliseconds(0))!=std::future_status::ready) {
      if(!QDir(root.path()).entryList(QDir::Dirs|QDir::NoDotAndDotDot).isEmpty()){observed=true;break;}
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    token->store(true);auto result=work.get();QVERIFY(observed);QCOMPARE(result.check.error,QStringLiteral("cancelled"));QVERIFY(!result.snapshot);
    QVERIFY(QDir(root.path()).entryList(QDir::AllEntries|QDir::NoDotAndDotDot).isEmpty());QCOMPARE(QFileInfo(path).size(),qint64(64*1024*1024));
  }
};
QTEST_GUILESS_MAIN(UpdateTests)
#include "update_tests.moc"
