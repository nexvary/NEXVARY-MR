#include "UpdateJournal.hpp"
#include "UpdateFormat.hpp"
#include "RecoveryFile.hpp"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLockFile>
#include <QSaveFile>
#include <QSet>
#include <algorithm>
#include <limits>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif

namespace mr::update {
namespace {
constexpr qint64 maximumJournalBytes = 8192;
CheckResult fail(const QString &code) { return {code,{}}; }
QByteArray compact(const QJsonObject &body) { return QJsonDocument(body).toJson(QJsonDocument::Compact); }
QByteArray authenticatedMessage(const QJsonObject &body) { return QByteArrayLiteral("NEXVARY-MR update journal v1\n")+compact(body); }
bool hex(const QString &value, qsizetype size) {
  if(value.size()!=size)return false;
  for(const auto c:value)if(!((c>=QLatin1Char('0')&&c<=QLatin1Char('9'))||(c>=QLatin1Char('a')&&c<=QLatin1Char('f'))))return false;
  return true;
}
bool fields(const QJsonObject &body, const QSet<QString> &names) {
  if(body.size()!=names.size())return false;
  for(auto i=body.begin();i!=body.end();++i)if(!names.contains(i.key()))return false;
  return true;
}
bool newerVersion(const QString &a,const QString &b) {
  std::array<quint64,3> x{},y{};return detail::version(a,x)&&detail::version(b,y)&&x>y;
}
bool installed(const QJsonObject &body,InstalledState &state) {
  std::array<quint64,3> parts{};
  if(!body["installedVersion"].isString()||!body["sequenceFloor"].isString())return false;
  state.version=body["installedVersion"].toString();
  return detail::version(state.version,parts)&&detail::decimal(body["sequenceFloor"].toString(),state.sequenceFloor);
}
QJsonObject candidate(const QJsonObject &report) {
  QJsonObject selected;
  for(const auto *field:{"version","sequence","sha256","manifestSha256","packageKind","packageBytes"})selected.insert(field,report.value(field));
  return selected;
}
QString validate(const QJsonObject &body,const JournalAnchor &anchor) {
  if(!fields(body,{"schema","installationId","profile","revision","installedVersion","sequenceFloor","phase","transaction","candidate"}))return "journal-schema";
  for(auto i=body.begin();i!=body.end();++i)if(i.key()!="candidate"&&(!i.value().isString()||i.value().toString().size()>128))return "journal-schema";
  if(body["schema"]!="nexvary-update-journal/v1"||body["profile"].toString()!=Verifier::compiledProfile()||body["installationId"].toString()!=anchor.installationId)return "journal-scope";
  quint64 revision=0;
  if(!detail::decimal(body["revision"].toString(),revision)||revision==0)return "journal-revision";
  if(revision<anchor.revisionFloor)return "journal-replay";
  InstalledState state;
  if(!installed(body,state))return "journal-installed-state";
  if(state.sequenceFloor<anchor.installed.sequenceFloor||newerVersion(anchor.installed.version,state.version))return "journal-rollback";
  const auto phase=body["phase"].toString();
  if(phase!="idle"&&phase!="prepared"&&phase!="applying"&&phase!="verifying")return "journal-phase";
  if(!body["candidate"].isObject())return "journal-candidate";
  const auto target=body["candidate"].toObject();
  if(phase=="idle")return target.isEmpty()&&body["transaction"].toString().isEmpty()?QString():QStringLiteral("journal-candidate");
  if(!hex(body["transaction"].toString(),32)||!fields(target,{"version","sequence","sha256","manifestSha256","packageKind","packageBytes"}))return "journal-candidate";
  for(auto i=target.begin();i!=target.end();++i)if(!i.value().isString()||i.value().toString().size()>128)return "journal-candidate";
  quint64 sequence=0,size=0;
  const auto kind=target["packageKind"].toString();
  if(!newerVersion(target["version"].toString(),state.version)||!detail::decimal(target["sequence"].toString(),sequence)||sequence<=state.sequenceFloor||
     !detail::decimal(target["packageBytes"].toString(),size)||size==0||size>static_cast<quint64>(maximumPackageBytes)||
     !hex(target["sha256"].toString(),64)||!hex(target["manifestSha256"].toString(),64)||(kind!="portable-zip"&&kind!="installer-exe"))return "journal-candidate";
  return {};
}
CheckResult projection(const QJsonObject &body) {
  QJsonObject report;
  for(const auto *field:{"revision","installedVersion","sequenceFloor","phase","candidate"})report.insert(field,body.value(field));
  const auto phase=body["phase"].toString();
  report.insert("pendingTransaction",phase!="idle");
  report.insert("nextAction",phase=="idle"?"none":phase=="prepared"?"revalidate-or-discard":"verify-installed-state-before-resolution");
  report.insert("journalAuthenticated",true);
  report.insert("atomicCheckpoint",true);
  report.insert("protectedAnchorPersisted",false);
  report.insert("postconditionVerifiedByJournal",false);
  report.insert("powerLossDurabilityGuaranteed",false);
  report.insert("installationAuthorized",false);
  return {{},report};
}
void clearPending(QJsonObject &body) {
  body["phase"]="idle";body["transaction"]="";body["candidate"]=QJsonObject{};
}
QString checkSnapshot(QJsonObject &body,StagedPackage &snapshot,const TrustedKeys &keys,const QDateTime &now,QJsonObject &checked) {
  InstalledState state;if(!installed(body,state))return "journal-installed-state";
  const auto result=snapshot.recheck(Verifier(keys,state),now);
  if(!result.accepted())return result.error;
  checked=candidate(result.report);
  if(body["phase"]!="idle"&&checked!=body["candidate"].toObject())return "journal-candidate-mismatch";
  return {};
}
}
Journal::Journal(QString directory,const QByteArray &key,JournalAnchor anchor)
  : directory_(std::move(directory)),anchor_(std::move(anchor)),
    observedRevision_(anchor_.revisionFloor),observedInstalled_(anchor_.installed) {
  static_assert(crypto_auth_KEYBYTES==32);
  keyValid_=key.size()==static_cast<qsizetype>(key_.size())&&std::any_of(key.cbegin(),key.cend(),[](char c){return c!=0;});
  if(keyValid_)std::copy(key.cbegin(),key.cend(),key_.begin());
}
Journal::~Journal(){sodium_memzero(key_.data(),key_.size());}
CheckResult Journal::access(bool create,const quint64 *expected,const Mutation &mutation) const {
  std::array<quint64,3> parts{};
  if(!keyValid_||!hex(anchor_.installationId,32)||!detail::version(anchor_.installed.version,parts))return fail("journal-configuration");
  if(!detail::cryptoInitialized()||!detail::approvedCryptoVersion())return fail("crypto-unavailable");
  const auto selected=mr::recovery::localFilePath(directory_);const QFileInfo root(selected);
  if(selected.isEmpty()||!root.isDir()||root.isSymLink())return fail("journal-directory");
  const auto canonical=root.canonicalFilePath();if(canonical.isEmpty())return fail("journal-directory");
#ifndef Q_OS_WIN
  struct stat permissions{};const auto encoded=QFile::encodeName(canonical);
  if(::stat(encoded.constData(),&permissions)!=0||permissions.st_uid!=::geteuid()||(permissions.st_mode&(S_IWGRP|S_IWOTH)))return fail("journal-directory-permissions");
#endif
  const auto path=QDir(canonical).filePath("update-state.json"),lockPath=QDir(canonical).filePath("update-state.lock");
  const QFileInfo lockInfo(lockPath);
  if(lockInfo.isSymLink()||(lockInfo.exists()&&(!lockInfo.isFile()||lockInfo.size()>4096)))return fail("journal-lock-path");
  QLockFile lock(lockPath);lock.setStaleLockTime(0);
  if(!lock.tryLock(0))return fail("journal-busy");
  auto effective=anchor_;effective.revisionFloor=observedRevision_;effective.installed=observedInstalled_;
  const QFileInfo info(path);QJsonObject body;
  if(create) {
    if(info.exists()||info.isSymLink())return fail("journal-already-exists");
    if(observedRevision_!=0)return fail("journal-missing-protected-state");
    body={{"schema","nexvary-update-journal/v1"},{"installationId",anchor_.installationId},{"profile",Verifier::compiledProfile()},
      {"revision","1"},{"installedVersion",anchor_.installed.version},{"sequenceFloor",QString::number(anchor_.installed.sequenceFloor)},
      {"phase","idle"},{"transaction",""},{"candidate",QJsonObject{}}};
  } else {
    if(info.isSymLink()||!info.isFile())return fail("journal-missing-or-unsafe");
    QFile input(path);if(!mr::recovery::openRegularInput(input)||input.size()>maximumJournalBytes)return fail("journal-read");
    const auto bytes=input.read(maximumJournalBytes+1);
    if(input.error()!=QFileDevice::NoError||bytes.size()>maximumJournalBytes||!input.atEnd())return fail("journal-read");
    QJsonParseError parse;const auto doc=QJsonDocument::fromJson(bytes,&parse);
    if(parse.error!=QJsonParseError::NoError||!doc.isObject()||doc.toJson(QJsonDocument::Compact)!=bytes)return fail("journal-encoding");
    const auto envelope=doc.object();
    if(!fields(envelope,{"body","mac"})||!envelope["body"].isObject()||!envelope["mac"].isString()||!hex(envelope["mac"].toString(),crypto_auth_BYTES*2))return fail("journal-envelope");
    body=envelope["body"].toObject();const auto tag=QByteArray::fromHex(envelope["mac"].toString().toLatin1()),message=authenticatedMessage(body);
    if(crypto_auth_verify(reinterpret_cast<const unsigned char *>(tag.constData()),reinterpret_cast<const unsigned char *>(message.constData()),static_cast<unsigned long long>(message.size()),key_.data())!=0)return fail("journal-authentication");
  }
  auto invalid=validate(body,effective);if(!invalid.isEmpty())return fail(invalid);
  quint64 revision=0;detail::decimal(body["revision"].toString(),revision);
  if(!create){observedRevision_=revision;installed(body,observedInstalled_);}
  if(expected&&*expected!=revision)return fail("journal-conflict");
  if(!create&&!mutation)return projection(body);
  if(mutation) {
    if(revision==std::numeric_limits<quint64>::max())return fail("journal-revision-exhausted");
    invalid=mutation(body);if(!invalid.isEmpty())return fail(invalid);
    body["revision"]=QString::number(revision+1);
    invalid=validate(body,effective);if(!invalid.isEmpty())return fail(invalid);
  }
  const auto message=authenticatedMessage(body);std::array<unsigned char,crypto_auth_BYTES> tag{};
  if(crypto_auth(tag.data(),reinterpret_cast<const unsigned char *>(message.constData()),static_cast<unsigned long long>(message.size()),key_.data())!=0)return fail("journal-authentication");
  const auto bytes=compact({{"body",body},{"mac",QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(tag.data()),static_cast<qsizetype>(tag.size())).toHex())}});
  QSaveFile output(path);output.setDirectWriteFallback(false);
  if(!output.open(QIODevice::WriteOnly))return fail("journal-write");
#ifndef Q_OS_WIN
  if(!output.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner)){output.cancelWriting();return fail("journal-permissions");}
#endif
  if(output.write(bytes)!=bytes.size()||!output.commit())return fail("journal-commit");
  detail::decimal(body["revision"].toString(),observedRevision_);installed(body,observedInstalled_);
  return projection(body);
}
CheckResult Journal::read() const{return access(false,nullptr,{});}
CheckResult Journal::initialize(){return access(true,nullptr,{});}
CheckResult Journal::prepare(quint64 revision,StagedPackage &snapshot,const TrustedKeys &keys,const QDateTime &now) {
  return access(false,&revision,[&](QJsonObject &body)->QString{
    if(body["phase"]!="idle")return "journal-pending-transaction";
    QJsonObject checked;const auto error=checkSnapshot(body,snapshot,keys,now,checked);if(!error.isEmpty())return error;
    std::array<unsigned char,16> id{};randombytes_buf(id.data(),id.size());
    body["transaction"]=QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(id.data()),static_cast<qsizetype>(id.size())).toHex());
    body["candidate"]=checked;body["phase"]="prepared";return {};
  });
}
CheckResult Journal::startApplying(quint64 revision,StagedPackage &snapshot,const TrustedKeys &keys,const QDateTime &now) {
  return access(false,&revision,[&](QJsonObject &body)->QString{
    if(body["phase"]!="prepared")return "journal-transition";
    QJsonObject checked;const auto error=checkSnapshot(body,snapshot,keys,now,checked);if(!error.isEmpty())return error;
    body["phase"]="applying";return {};
  });
}
CheckResult Journal::startVerifying(quint64 revision) {
  return access(false,&revision,[](QJsonObject &body)->QString{
    if(body["phase"]!="applying")return "journal-transition";
    body["phase"]="verifying";return {};
  });
}
CheckResult Journal::confirmInstalled(quint64 revision,const InstalledState &observed,StagedPackage &snapshot,const TrustedKeys &keys,const QDateTime &now) {
  return access(false,&revision,[&](QJsonObject &body)->QString{
    if(body["phase"]!="verifying")return "journal-transition";
    QJsonObject checked;const auto error=checkSnapshot(body,snapshot,keys,now,checked);if(!error.isEmpty())return error;
    if(observed.version!=checked["version"].toString()||QString::number(observed.sequenceFloor)!=checked["sequence"].toString())return "journal-postcondition-mismatch";
    body["installedVersion"]=observed.version;body["sequenceFloor"]=QString::number(observed.sequenceFloor);clearPending(body);return {};
  });
}
CheckResult Journal::discardPrepared(quint64 revision) {
  return access(false,&revision,[](QJsonObject &body)->QString{
    if(body["phase"]!="prepared")return "journal-transition";
    clearPending(body);return {};
  });
}
CheckResult Journal::recordRestoredBaseline(quint64 revision,const InstalledState &observed) {
  return access(false,&revision,[&](QJsonObject &body)->QString{
    if(body["phase"]!="applying"&&body["phase"]!="verifying")return "journal-transition";
    if(observed.version!=body["installedVersion"].toString()||QString::number(observed.sequenceFloor)!=body["sequenceFloor"].toString())return "journal-postcondition-mismatch";
    clearPending(body);return {};
  });
}
}
