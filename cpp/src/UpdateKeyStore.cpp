#include "UpdateKeyStore.hpp"
#include "UpdateFormat.hpp"
#include "CaseVault.hpp"
#include "RecoveryFile.hpp"
#include <QCryptographicHash>
#include <QDir>
#include <QLockFile>
#include <QSaveFile>
#include <algorithm>
namespace mr::update {
namespace {
const QByteArray header=QByteArrayLiteral("NMR-UPDATE-KEY-DPAPI-1\n");
constexpr qint64 maximumSealedKeyBytes=8192;
struct ClearBytes {
  QByteArray &bytes;
  ~ClearBytes(){sodium_memzero(bytes.data(),static_cast<size_t>(bytes.size()));bytes.clear();}
};
bool identity(const QString &id){
 if(id.size()!=32)return false;
 for(const auto c:id)if(!((c>=QLatin1Char('0')&&c<=QLatin1Char('9'))||(c>=QLatin1Char('a')&&c<=QLatin1Char('f'))))return false;
 return true;
}
QByteArray prefix(const QString &id){return QByteArrayLiteral("NEXVARY-MR local journal authentication key v1\n")+id.toLatin1()+'\n'+Verifier::compiledProfile().toLatin1()+'\n';}
QString directory(const QString &input){
 const auto path=mr::recovery::localFilePath(input);const QFileInfo info(path);
 return !path.isEmpty()&&info.isDir()&&!info.isSymLink()?info.canonicalFilePath():QString();
}
bool unsafeLock(const QString &path){const QFileInfo info(path);return info.isSymLink()||(info.exists()&&(!info.isFile()||info.size()>4096));}
CheckResult fail(const char *code){return {QString::fromLatin1(code),{}};}
JournalHandle handleError(const char *code){return {QString::fromLatin1(code),nullptr};}
}
bool KeyStore::available(){return mr::recovery::vaultAvailable();}
CheckResult KeyStore::create(const QString &input,const JournalAnchor &anchor){
 if(!available())return fail("key-store-unavailable"); // No Linux plaintext/test fallback.
 std::array<quint64,3> parts{};const auto &id=anchor.installationId;
 if(!identity(id)||!detail::version(anchor.installed.version,parts))return fail("key-store-scope");
 if(anchor.revisionFloor!=0)return fail("key-store-bootstrap-forbidden");
 if(!detail::cryptoInitialized()||!detail::approvedCryptoVersion())return fail("crypto-unavailable");
 const auto root=directory(input);if(root.isEmpty())return fail("key-store-directory");
 const auto lockPath=QDir(root).filePath("update-state.lock");if(unsafeLock(lockPath))return fail("key-store-lock-path");
 QLockFile lock(lockPath);lock.setStaleLockTime(0);if(!lock.tryLock(0))return fail("key-store-busy");
 const auto path=QDir(root).filePath("update-auth.dpapi");
 for(const auto &existing:QStringList{path,QDir(root).filePath("update-state.json")}){
  const QFileInfo info(existing);if(info.exists()||info.isSymLink())return fail("key-store-already-initialized");
 }
 QByteArray key(crypto_auth_KEYBYTES,'\0');ClearBytes clearKey{key};crypto_auth_keygen(reinterpret_cast<unsigned char *>(key.data()));
 QByteArray plain;plain.reserve(256);plain+=prefix(id);plain+=key;plain+=QCryptographicHash::hash(plain,QCryptographicHash::Sha256);ClearBytes clearPlain{plain};
 QString error;const auto encrypted=mr::recovery::sealCase(plain,false,error);
 if(encrypted.isEmpty()||!error.isEmpty())return fail("key-store-protection");
 const auto sealed=header+encrypted;if(sealed.size()>maximumSealedKeyBytes)return fail("key-store-limit");
 QSaveFile output(path);output.setDirectWriteFallback(false);
 if(!output.open(QIODevice::WriteOnly)||output.write(sealed)!=sealed.size()||!output.commit())return fail("key-store-commit");
 return {{},{{"localJournalKeyCreated",true},{"currentUserProtected",true},{"publisherKeyCreated",false},{"protectedAnchorPersisted",false},{"installationAuthorized",false}}};
}
JournalHandle KeyStore::openJournal(const QString &input,const JournalAnchor &anchor){
 if(!available())return handleError("key-store-unavailable");
 std::array<quint64,3> parts{};
 if(!identity(anchor.installationId)||!detail::version(anchor.installed.version,parts))return handleError("key-store-scope");
 if(!detail::cryptoInitialized()||!detail::approvedCryptoVersion())return handleError("crypto-unavailable");
 const auto root=directory(input);if(root.isEmpty())return handleError("key-store-directory");
 const auto lockPath=QDir(root).filePath("update-state.lock");if(unsafeLock(lockPath))return handleError("key-store-lock-path");
 QLockFile lock(lockPath);lock.setStaleLockTime(0);if(!lock.tryLock(0))return handleError("key-store-busy");
 const auto path=QDir(root).filePath("update-auth.dpapi");const QFileInfo info(path);
 if(!info.isFile()||info.isSymLink())return handleError("key-store-missing-or-unsafe");
 QFile inputFile(path);if(!mr::recovery::openRegularInput(inputFile)||inputFile.size()>maximumSealedKeyBytes)return handleError("key-store-read");
 const auto sealed=inputFile.read(maximumSealedKeyBytes+1);
 if(inputFile.error()!=QFileDevice::NoError||sealed.size()>maximumSealedKeyBytes||!inputFile.atEnd())return handleError("key-store-read");
 if(!sealed.startsWith(header))return handleError("key-store-format");
 QString error;auto plain=mr::recovery::openCaseBytes(sealed.mid(header.size()),false,error);ClearBytes clearPlain{plain};
 if(plain.isEmpty()||!error.isEmpty())return handleError("key-store-protection");
 const auto expected=prefix(anchor.installationId);const auto bodySize=expected.size()+crypto_auth_KEYBYTES;
 if(plain.size()!=bodySize+32||!plain.startsWith(expected))return handleError("key-store-scope");
 if(QCryptographicHash::hash(QByteArrayView(plain).first(bodySize),QCryptographicHash::Sha256)!=plain.last(32))return handleError("key-store-integrity");
 auto key=plain.mid(expected.size(),crypto_auth_KEYBYTES);ClearBytes clearKey{key};
 if(std::all_of(key.cbegin(),key.cend(),[](char c){return c==0;}))return handleError("key-store-integrity");
 auto journal=std::make_unique<Journal>(root,key,anchor);
 return {{},std::move(journal)};
}
}
