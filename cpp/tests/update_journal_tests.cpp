#include "UpdateJournal.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLockFile>
#include <QProcess>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QThread>
#include <array>
#include <cstdio>
#include <limits>
#include <sodium.h>
#ifndef Q_OS_WIN
#include <sys/stat.h>
#endif
using namespace mr::update;
namespace {
JournalAnchor anchor(){return {QString(32,QLatin1Char('a')),{"0.4.0",5},0};}
QDateTime clockNow(){return QDateTime::fromString("2026-10-23T12:00:00Z",Qt::ISODate);}
QByteArray compact(const QJsonObject &o){return QJsonDocument(o).toJson(QJsonDocument::Compact);}
bool write(const QString &path,const QByteArray &bytes){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size();}
QByteArray read(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray{};}
QString statePath(const QString &root){return QDir(root).filePath("update-state.json");}
struct Reference {
 QByteArray bytes,signature,payload;TrustedKeys keys;
 Reference(){
  QFile f(QString::fromUtf8(MR_UPDATE_REFERENCE_PATH));if(!f.open(QIODevice::ReadOnly))return;
  const auto o=QJsonDocument::fromJson(f.readAll()).object();payload=QByteArray::fromBase64(o["packageBase64"].toString().toLatin1());
  keys.insert("synthetic-reference",QByteArray::fromHex(o["publicKeyHex"].toString().toLatin1()));
  for(const auto &v:o["vectors"].toArray())if(v.toObject()["profile"].toString()==Verifier::compiledProfile()){
   bytes=v.toObject()["manifestUtf8"].toString().toUtf8();signature=QByteArray::fromHex(v.toObject()["signatureHex"].toString().toLatin1());
  }
 }
 SnapshotResult stage(const QString &source,const QString &root)const{
  if(!write(source,payload))return {{"fixture-write",{}},nullptr};
  return Verifier(keys,anchor().installed).stagePackage(bytes,signature,source,root,clockNow());
 }
};
void forge(const QString &path,const QJsonObject &body,const QByteArray &key){
 const auto message=QByteArrayLiteral("NEXVARY-MR update journal v1\n")+compact(body);std::array<unsigned char,crypto_auth_BYTES> mac{};
 if(crypto_auth(mac.data(),reinterpret_cast<const unsigned char *>(message.constData()),static_cast<unsigned long long>(message.size()),reinterpret_cast<const unsigned char *>(key.constData()))!=0)return;
 write(path,compact({{"body",body},{"mac",QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(mac.data()),static_cast<qsizetype>(mac.size())).toHex())}}));
}
void ready(){std::fputs("READY\n",stdout);std::fflush(stdout);}
int runJournalChild(QCoreApplication &app,const QStringList &args){
 if(args.size()!=4)return 50;
 const auto root=args[2],mode=args[3];
 if(mode=="hold"||mode=="uncommitted"){
  QLockFile lock(QDir(root).filePath("update-state.lock"));lock.setStaleLockTime(0);if(!lock.tryLock(0))return 51;
  QSaveFile pending(statePath(root));pending.setDirectWriteFallback(false);
  if(mode=="uncommitted"&&(!pending.open(QIODevice::WriteOnly)||pending.write("interrupted uncommitted bytes")<0||!pending.flush()))return 52;
  ready();QTimer::singleShot(10000,&app,&QCoreApplication::quit);return app.exec();
 }
 QFile input;if(!input.open(stdin,QIODevice::ReadOnly))return 53;
 auto encoded=input.read(crypto_auth_KEYBYTES*2);auto key=QByteArray::fromHex(encoded);
 const bool valid=encoded.size()==crypto_auth_KEYBYTES*2&&key.toHex()==encoded;
 sodium_memzero(encoded.data(),static_cast<size_t>(encoded.size()));if(!valid)return 54;
 Journal journal(root,key,anchor());sodium_memzero(key.data(),static_cast<size_t>(key.size()));
 QTemporaryDir source,stage;Reference fixture;auto snapshot=fixture.stage(source.filePath("fixture.bin"),stage.path());
 if(!snapshot.accepted())return 55;
 if(mode=="race"){
  // Windows QFile(FILE*) may fill its read buffer until EOF. Key input ends
  // before READY; a separate test-only file barrier starts both writers.
  ready();QElapsedTimer barrier;barrier.start();
  const auto release=QDir(root).filePath("race-release");
  while(!QFile::exists(release)&&barrier.elapsed()<20000)QThread::msleep(1);
  if(!QFile::exists(release))return 56;
 }
 const auto result=journal.prepare(1,*snapshot.snapshot,fixture.keys,clockNow());
 if(!result.accepted())return result.error=="journal-conflict"?11:result.error=="journal-busy"?12:57;
 if(mode=="prepared"){ready();QTimer::singleShot(10000,&app,&QCoreApplication::quit);return app.exec();}
 return 0;
}
}
class JournalTests final:public QObject{
 Q_OBJECT
 QByteArray key_;
 static void expectError(const CheckResult &r,const char *code){QCOMPARE(r.error,QString::fromLatin1(code));QVERIFY(r.report.isEmpty());}
 static bool start(QProcess &p,const QString &root,const QString &mode){
  p.start(QCoreApplication::applicationFilePath(),{"--journal-child",root,mode});return p.waitForStarted(5000);
 }
 static bool receiveReady(QProcess &p){
  QElapsedTimer timer;timer.start();
  while(!p.canReadLine()&&timer.elapsed()<5000)
   if(!p.waitForReadyRead(qMax(1,5000-static_cast<int>(timer.elapsed()))))break;
  return p.canReadLine()&&p.readLine().trimmed()==QByteArray("READY");
 }
 bool sendKey(QProcess &p)const{
  auto encoded=key_.toHex();const auto count=p.write(encoded);sodium_memzero(encoded.data(),static_cast<size_t>(encoded.size()));
  return count==crypto_auth_KEYBYTES*2&&(p.bytesToWrite()==0||p.waitForBytesWritten(5000));
 }
private slots:
 void initTestCase(){QVERIFY(sodium_init()>=0);key_.resize(crypto_auth_KEYBYTES);crypto_auth_keygen(reinterpret_cast<unsigned char *>(key_.data()));}
 void cleanupTestCase(){sodium_memzero(key_.data(),static_cast<size_t>(key_.size()));key_.clear();}
 void initializeReopenAndPrivacy(){
  QTemporaryDir root;QVERIFY(root.isValid());QVERIFY(write(root.filePath("keep.txt"),"unrelated"));Journal j(root.path(),key_,anchor());
  expectError(j.read(),"journal-missing-or-unsafe");const auto initialized=j.initialize();QVERIFY2(initialized.accepted(),qPrintable(initialized.error));
  QCOMPARE(initialized.report["revision"].toString(),QString("1"));QCOMPARE(initialized.report["phase"].toString(),QString("idle"));
  QVERIFY(initialized.report["journalAuthenticated"].toBool());
  for(const auto *field:{"installationAuthorized","protectedAnchorPersisted","powerLossDurabilityGuaranteed"})QVERIFY(!initialized.report[field].toBool(true));
  const auto before=read(statePath(root.path()));QVERIFY(!before.contains(key_));QVERIFY(!before.contains(root.path().toUtf8()));
  Journal reopened(root.path(),key_,anchor());QCOMPARE(reopened.read().report,initialized.report);
  expectError(reopened.initialize(),"journal-already-exists");QCOMPARE(read(statePath(root.path())),before);QCOMPARE(read(root.filePath("keep.txt")),QByteArray("unrelated"));
 }
 void invalidConfigurationAndDirectories(){
  QTemporaryDir root;
  for(const auto &bad:QList<QByteArray>{QByteArray{},QByteArray(31,'a'),QByteArray(32,'\0')}){
   Journal j(root.path(),bad,anchor());expectError(j.initialize(),"journal-configuration");
  }
  for(const auto &path:QStringList{"https://example.invalid/state",root.filePath("missing"),root.filePath("file")}){
   Journal j(path,key_,anchor());expectError(j.initialize(),"journal-directory");
  }
  auto bad=anchor();bad.installationId="invalid";Journal invalid(root.path(),key_,bad);expectError(invalid.initialize(),"journal-configuration");
  QVERIFY(QDir(root.path()).entryList(QDir::AllEntries|QDir::NoDotAndDotDot).isEmpty());
#ifndef Q_OS_WIN
  QTemporaryDir parent;const auto link=parent.filePath("alias");QVERIFY(QFile::link(root.path(),link));
  Journal symlink(link,key_,anchor());expectError(symlink.initialize(),"journal-directory");
  QVERIFY(QFile::setPermissions(root.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::WriteGroup));
  Journal shared(root.path(),key_,anchor());expectError(shared.initialize(),"journal-directory-permissions");
  QVERIFY(QFile::setPermissions(root.path(),QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));
#endif
 }
 void transactionCommitRequiresSnapshotAndObservation(){
  QTemporaryDir root,source,stage;Reference f;auto snapshot=f.stage(source.filePath("source.bin"),stage.path());QVERIFY(snapshot.accepted());
  Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());
  expectError(j.startVerifying(1),"journal-transition");
  auto r=j.prepare(1,*snapshot.snapshot,f.keys,clockNow());QVERIFY(r.accepted());QCOMPARE(r.report["revision"].toString(),QString("2"));
  const auto prepared=read(statePath(root.path()));expectError(j.prepare(2,*snapshot.snapshot,f.keys,clockNow()),"journal-pending-transaction");
  expectError(j.startApplying(1,*snapshot.snapshot,f.keys,clockNow()),"journal-conflict");QCOMPARE(read(statePath(root.path())),prepared);
  QVERIFY(j.startApplying(2,*snapshot.snapshot,f.keys,clockNow()).accepted());expectError(j.discardPrepared(3),"journal-transition");
  expectError(j.confirmInstalled(3,{"0.4.1",6},*snapshot.snapshot,f.keys,clockNow()),"journal-transition");
  QVERIFY(j.startVerifying(3).accepted());
  expectError(j.confirmInstalled(4,{"0.4.1",5},*snapshot.snapshot,f.keys,clockNow()),"journal-postcondition-mismatch");
  expectError(j.confirmInstalled(4,{"0.4.2",6},*snapshot.snapshot,f.keys,clockNow()),"journal-postcondition-mismatch");
  r=j.confirmInstalled(4,{"0.4.1",6},*snapshot.snapshot,f.keys,clockNow());QVERIFY(r.accepted());
  QCOMPARE(r.report["phase"].toString(),QString("idle"));QCOMPARE(r.report["revision"].toString(),QString("5"));
  QCOMPARE(r.report["installedVersion"].toString(),QString("0.4.1"));QCOMPARE(r.report["sequenceFloor"].toString(),QString("6"));
  QVERIFY(!r.report["installationAuthorized"].toBool(true));expectError(j.prepare(5,*snapshot.snapshot,f.keys,clockNow()),"version-not-newer");
  auto protectedAnchor=anchor();protectedAnchor.installed={"0.4.1",6};protectedAnchor.revisionFloor=5;
  Journal reopened(root.path(),key_,protectedAnchor);QVERIFY(reopened.read().accepted());
  auto regressed=QJsonDocument::fromJson(read(statePath(root.path()))).object()["body"].toObject();
  regressed["revision"]="6";regressed["installedVersion"]="0.4.0";regressed["sequenceFloor"]="5";forge(statePath(root.path()),regressed,key_);
  expectError(j.read(),"journal-rollback");expectError(reopened.read(),"journal-rollback");
 }
 void interruptedStatesAndBaselineResolution(){
  QTemporaryDir root,source,stage;Reference f;auto snapshot=f.stage(source.filePath("s"),stage.path());QVERIFY(snapshot.accepted());
  Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());QVERIFY(j.prepare(1,*snapshot.snapshot,f.keys,clockNow()).accepted());
  Journal prepared(root.path(),key_,anchor());auto r=prepared.read();QVERIFY(r.accepted());QCOMPARE(r.report["nextAction"].toString(),QString("revalidate-or-discard"));
  QVERIFY(j.discardPrepared(2).accepted());QVERIFY(j.prepare(3,*snapshot.snapshot,f.keys,clockNow()).accepted());QVERIFY(j.startApplying(4,*snapshot.snapshot,f.keys,clockNow()).accepted());
  Journal interrupted(root.path(),key_,anchor());r=interrupted.read();QVERIFY(r.report["pendingTransaction"].toBool());
  QCOMPARE(r.report["nextAction"].toString(),QString("verify-installed-state-before-resolution"));
  expectError(interrupted.recordRestoredBaseline(5,{"0.4.0",4}),"journal-postcondition-mismatch");
  r=interrupted.recordRestoredBaseline(5,{"0.4.0",5});QVERIFY(r.accepted());QCOMPARE(r.report["sequenceFloor"].toString(),QString("5"));
  QCOMPARE(r.report["phase"].toString(),QString("idle"));QVERIFY(read(source.filePath("s"))==f.payload);
 }
 void trustExpiryAndSnapshotChangesRejectWithoutWrite(){
  QTemporaryDir root,source,stage;Reference f;auto snapshot=f.stage(source.filePath("s"),stage.path());QVERIFY(snapshot.accepted());
  Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());QVERIFY(j.prepare(1,*snapshot.snapshot,f.keys,clockNow()).accepted());
  const auto before=read(statePath(root.path()));
  expectError(j.startApplying(2,*snapshot.snapshot,{},clockNow()),"no-approved-trust-anchors");
  expectError(j.startApplying(2,*snapshot.snapshot,f.keys,clockNow().addDays(2)),"manifest-time-window");
  const auto child=QDir(stage.path()).entryList(QDir::Dirs|QDir::NoDotAndDotDot).first();auto corrupt=f.payload;corrupt[0]=static_cast<char>(corrupt[0]^1);
  QVERIFY(write(QDir(stage.path()).filePath(child+"/package.bin"),corrupt));
  expectError(j.startApplying(2,*snapshot.snapshot,f.keys,clockNow()),"package-hash-mismatch");QCOMPARE(read(statePath(root.path())),before);
 }
 void authenticationScopeAndEncodingFailures(){
  QTemporaryDir root;Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());const auto original=read(statePath(root.path()));
  auto other=key_;other[0]=static_cast<char>(other[0]^1);Journal wrong(root.path(),other,anchor());expectError(wrong.read(),"journal-authentication");sodium_memzero(other.data(),static_cast<size_t>(other.size()));
  auto otherAnchor=anchor();otherAnchor.installationId=QString(32,QLatin1Char('b'));Journal otherInstall(root.path(),key_,otherAnchor);expectError(otherInstall.read(),"journal-scope");
  auto outer=QJsonDocument::fromJson(original).object();auto body=outer["body"].toObject();body["installedVersion"]="0.4.1";outer["body"]=body;
  QVERIFY(write(statePath(root.path()),compact(outer)));expectError(j.read(),"journal-authentication");
  QVERIFY(write(statePath(root.path())," "+original));expectError(j.read(),"journal-encoding");
  QVERIFY(write(statePath(root.path()),QByteArray(8193,'X')));expectError(j.read(),"journal-read");
  QVERIFY(write(statePath(root.path()),original));QVERIFY(j.read().accepted());
 }
 void malformedAuthenticatedState_data(){
  QTest::addColumn<QString>("field");QTest::addColumn<QJsonValue>("value");QTest::addColumn<QString>("error");
  auto row=[](const char *name,const char *field,const QJsonValue &value,const char *error){QTest::newRow(name)<<QString::fromLatin1(field)<<value<<QString::fromLatin1(error);};
  row("unknown","extra","bad","journal-schema");row("schema","schema","other","journal-scope");
  row("profile","profile",Verifier::compiledProfile()=="production-read-only"?"laboratory-experimental":"production-read-only","journal-scope");
  row("revision-type","revision",1,"journal-schema");row("revision-zero","revision","0","journal-revision");row("revision-leading-zero","revision","01","journal-revision");
  row("revision-overflow","revision","18446744073709551616","journal-revision");
  row("version","installedVersion","bad","journal-installed-state");row("version-old","installedVersion","0.3.0","journal-rollback");
  row("floor-old","sequenceFloor","4","journal-rollback");row("floor-negative","sequenceFloor","-1","journal-installed-state");
  row("phase","phase","unknown","journal-phase");row("idle-transaction","transaction",QString(32,QLatin1Char('a')),"journal-candidate");
  row("candidate-type","candidate","bad","journal-candidate");row("candidate-idle","candidate",QJsonObject{{"version","0.4.1"}},"journal-candidate");
 }
 void malformedAuthenticatedState(){
  QFETCH(QString,field);QFETCH(QJsonValue,value);QFETCH(QString,error);QTemporaryDir root;Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());
  auto body=QJsonDocument::fromJson(read(statePath(root.path()))).object()["body"].toObject();body[field]=value;forge(statePath(root.path()),body,key_);
  const auto r=j.read();QCOMPARE(r.error,error);QVERIFY(r.report.isEmpty());
 }
 void memoryAndIndependentAnchorsDetectReplay(){
  QTemporaryDir root,source,stage;Reference f;auto snapshot=f.stage(source.filePath("s"),stage.path());QVERIFY(snapshot.accepted());
  Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());const auto old=read(statePath(root.path()));QVERIFY(j.prepare(1,*snapshot.snapshot,f.keys,clockNow()).accepted());
  QVERIFY(write(statePath(root.path()),old));expectError(j.read(),"journal-replay");
  auto independent=anchor();independent.revisionFloor=2;Journal protectedJournal(root.path(),key_,independent);expectError(protectedJournal.read(),"journal-replay");
  Journal noExternalAnchor(root.path(),key_,anchor());QVERIFY(noExternalAnchor.read().accepted()); // MAC alone cannot detect valid replay.
  QVERIFY(QFile::remove(statePath(root.path())));expectError(j.initialize(),"journal-missing-protected-state");expectError(protectedJournal.initialize(),"journal-missing-protected-state");
 }
 void malformedPendingCandidate_data(){
  QTest::addColumn<QString>("field");QTest::addColumn<QJsonValue>("value");
  auto row=[](const char *name,const char *field,const QJsonValue &value){QTest::newRow(name)<<QString::fromLatin1(field)<<value;};
  row("old-version","version","0.4.0");row("bad-version","version","0.04.1");
  row("old-sequence","sequence","5");row("sequence-type","sequence",6);
  row("missing-digest","sha256",QJsonValue());row("uppercase-digest","sha256",QString(64,QLatin1Char('A')));
  row("manifest-digest","manifestSha256","bad");row("package-kind","packageKind","shell");
  row("empty-size","packageBytes","0");row("huge-size","packageBytes","536870913");
  row("leading-zero-size","packageBytes","01");row("unknown","extra","value");
  row("transaction","@transaction","bad");
 }
 void malformedPendingCandidate(){
  QFETCH(QString,field);QFETCH(QJsonValue,value);QTemporaryDir root,source,stage;Reference f;auto snapshot=f.stage(source.filePath("s"),stage.path());QVERIFY(snapshot.accepted());
  Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());QVERIFY(j.prepare(1,*snapshot.snapshot,f.keys,clockNow()).accepted());
  auto body=QJsonDocument::fromJson(read(statePath(root.path()))).object()["body"].toObject();
  if(field.startsWith(QLatin1Char('@')))body[field.mid(1)]=value;
  else{auto target=body["candidate"].toObject();target[field]=value;body["candidate"]=target;}
  forge(statePath(root.path()),body,key_);expectError(j.read(),"journal-candidate");
 }
 void differentValidCandidateCannotReplacePreparedTransaction(){
  QTemporaryDir root,source,stage,otherStage;Reference f;auto snapshot=f.stage(source.filePath("s"),stage.path());QVERIFY(snapshot.accepted());
  std::array<unsigned char,crypto_sign_PUBLICKEYBYTES> publicKey{};std::array<unsigned char,crypto_sign_SECRETKEYBYTES> privateKey{};
  QCOMPARE(crypto_sign_keypair(publicKey.data(),privateKey.data()),0);
  auto object=QJsonDocument::fromJson(f.bytes).object();object["keyId"]="synthetic-other";object["version"]="0.4.2";object["sequence"]="7";
  const auto bytes=compact(object),message=Verifier::signingMessage(bytes);QByteArray signature(crypto_sign_BYTES,'\0');
  const auto signedResult=crypto_sign_detached(reinterpret_cast<unsigned char *>(signature.data()),nullptr,reinterpret_cast<const unsigned char *>(message.constData()),static_cast<unsigned long long>(message.size()),privateKey.data());
  sodium_memzero(privateKey.data(),privateKey.size());QCOMPARE(signedResult,0);
  auto keys=f.keys;keys.insert("synthetic-other",QByteArray(reinterpret_cast<const char *>(publicKey.data()),static_cast<qsizetype>(publicKey.size())));
  auto other=Verifier(keys,anchor().installed).stagePackage(bytes,signature,source.filePath("s"),otherStage.path(),clockNow());QVERIFY(other.accepted());
  Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());QVERIFY(j.prepare(1,*snapshot.snapshot,f.keys,clockNow()).accepted());
  const auto before=read(statePath(root.path()));expectError(j.startApplying(2,*other.snapshot,keys,clockNow()),"journal-candidate-mismatch");QCOMPARE(read(statePath(root.path())),before);
 }
 void revisionExhaustionPreservesRecord(){
  QTemporaryDir root,source,stage;Reference f;auto snapshot=f.stage(source.filePath("s"),stage.path());QVERIFY(snapshot.accepted());Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());
  auto body=QJsonDocument::fromJson(read(statePath(root.path()))).object()["body"].toObject();body["revision"]="18446744073709551615";forge(statePath(root.path()),body,key_);
  const auto before=read(statePath(root.path()));expectError(j.prepare(std::numeric_limits<quint64>::max(),*snapshot.snapshot,f.keys,clockNow()),"journal-revision-exhausted");QCOMPARE(read(statePath(root.path())),before);
 }
 void liveLockAndCrashRecovery(){
  QTemporaryDir root;Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());const auto before=read(statePath(root.path()));
  for(const auto &mode:QStringList{"hold","uncommitted"}){
   QProcess process;QVERIFY(start(process,root.path(),mode));QVERIFY(receiveReady(process));expectError(j.read(),"journal-busy");
   process.kill();QVERIFY(process.waitForFinished(5000));QVERIFY(j.read().accepted());QCOMPARE(read(statePath(root.path())),before);
  }
  QProcess committed;QVERIFY(start(committed,root.path(),"prepared"));QVERIFY(sendKey(committed));committed.closeWriteChannel();QVERIFY(receiveReady(committed));
  committed.kill();QVERIFY(committed.waitForFinished(5000));Journal reopened(root.path(),key_,anchor());const auto r=reopened.read();QVERIFY2(r.accepted(),qPrintable(r.error));
  QCOMPARE(r.report["phase"].toString(),QString("prepared"));QCOMPARE(r.report["revision"].toString(),QString("2"));QVERIFY(reopened.discardPrepared(2).accepted());
 }
 void twoProcessesCannotBothCommitSameRevision(){
  QTemporaryDir root;Journal j(root.path(),key_,anchor());QVERIFY(j.initialize().accepted());QProcess first,second;
  QVERIFY(start(first,root.path(),"race"));QVERIFY(start(second,root.path(),"race"));
  QVERIFY(sendKey(first));QVERIFY(sendKey(second));first.closeWriteChannel();second.closeWriteChannel();
  QVERIFY(receiveReady(first));QVERIFY(receiveReady(second));QVERIFY(write(root.filePath("race-release"),"go"));
  QVERIFY(first.waitForFinished(10000));QVERIFY(second.waitForFinished(10000));QCOMPARE(first.exitStatus(),QProcess::NormalExit);QCOMPARE(second.exitStatus(),QProcess::NormalExit);
  const auto a=first.exitCode(),b=second.exitCode();QVERIFY((a==0&&(b==11||b==12))||(b==0&&(a==11||a==12)));
  const auto r=j.read();QVERIFY(r.accepted());QCOMPARE(r.report["revision"].toString(),QString("2"));
 }
 void unsafeStateAndLockPaths(){
#ifndef Q_OS_WIN
  QTemporaryDir root,outside;const auto sentinel=outside.filePath("keep");QVERIFY(write(sentinel,"unrelated"));
  QVERIFY(QFile::link(sentinel,statePath(root.path())));Journal j(root.path(),key_,anchor());expectError(j.read(),"journal-missing-or-unsafe");expectError(j.initialize(),"journal-already-exists");
  QVERIFY(QFile::remove(statePath(root.path())));const auto lock=root.filePath("update-state.lock");QVERIFY(QFile::link(sentinel,lock));expectError(j.initialize(),"journal-lock-path");QVERIFY(QFile::remove(lock));
  QVERIFY(::mkfifo(QFile::encodeName(lock).constData(),0600)==0);expectError(j.initialize(),"journal-lock-path");QCOMPARE(read(sentinel),QByteArray("unrelated"));
#else
  QTemporaryDir root;QVERIFY(QDir().mkdir(statePath(root.path())));Journal j(root.path(),key_,anchor());expectError(j.read(),"journal-missing-or-unsafe");expectError(j.initialize(),"journal-already-exists");
#endif
 }
};
int main(int argc,char **argv){QCoreApplication app(argc,argv);if(app.arguments().contains("--journal-child"))return runJournalChild(app,app.arguments());JournalTests tests;return QTest::qExec(&tests,argc,argv);}
#include "update_journal_tests.moc"
