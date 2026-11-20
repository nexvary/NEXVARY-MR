#include "CaseVault.hpp"
#include <QCryptographicHash>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dpapi.h>
#endif
namespace mr::recovery {
namespace {constexpr qsizetype maximum=96*1024*1024;const QByteArray encryptedHeader("NMR-DPAPI-1\n");const QByteArray syntheticHeader("NMR-SYNTHETIC-ONLY-1\n");}
bool vaultAvailable() {
#ifdef Q_OS_WIN
 return true;
#else
 return false;
#endif
}
QByteArray sealCase(const QByteArray &plain,bool syntheticProfile,QString &error) {
 error.clear();
 if(plain.isEmpty()||plain.size()>maximum){error=QStringLiteral("Case exceeds the bounded vault size.");return {};}
#ifdef Q_OS_WIN
 Q_UNUSED(syntheticProfile);
 DATA_BLOB input{static_cast<DWORD>(plain.size()),reinterpret_cast<BYTE *>(const_cast<char *>(plain.constData()))},output{};
 if(!CryptProtectData(&input,L"NEXVARY recovery case",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&output)){error=QStringLiteral("Windows case encryption failed (%1).").arg(GetLastError());return {};}
 QByteArray result=encryptedHeader+QByteArray(reinterpret_cast<const char *>(output.pbData),static_cast<qsizetype>(output.cbData));
 SecureZeroMemory(output.pbData,output.cbData);LocalFree(output.pbData);return result;
#else
 if(!syntheticProfile){error=QStringLiteral("Sensitive recovery cases require Windows current-user DPAPI; this platform has no production vault.");return {};}
 return syntheticHeader+QCryptographicHash::hash(plain,QCryptographicHash::Sha256).toHex()+'\n'+plain;
#endif
}
QByteArray openCaseBytes(const QByteArray &sealed,bool syntheticProfile,QString &error) {
 error.clear();if(sealed.size()>maximum+1024*1024){error=QStringLiteral("Case file exceeds vault limit.");return {};}
#ifdef Q_OS_WIN
 Q_UNUSED(syntheticProfile);
 if(!sealed.startsWith(encryptedHeader)){error=QStringLiteral("Only Windows DPAPI case files are accepted.");return {};}
 const auto payload=sealed.mid(encryptedHeader.size());
 DATA_BLOB input{static_cast<DWORD>(payload.size()),reinterpret_cast<BYTE *>(const_cast<char *>(payload.constData()))},output{};
 if(!CryptUnprotectData(&input,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&output)){error=QStringLiteral("Case integrity/account protection failed. Use its original Windows account and computer.");return {};}
 QByteArray result(reinterpret_cast<const char *>(output.pbData),static_cast<qsizetype>(output.cbData));SecureZeroMemory(output.pbData,output.cbData);LocalFree(output.pbData);
 if(result.isEmpty()||result.size()>maximum){error=QStringLiteral("Invalid decrypted case size.");return {};}return result;
#else
 if(!syntheticProfile||!sealed.startsWith(syntheticHeader)){error=QStringLiteral("Production case decryption is available on Windows only.");return {};}
 const auto start=syntheticHeader.size();const auto plain=sealed.mid(start+65);
 if(sealed.mid(start,64)!=QCryptographicHash::hash(plain,QCryptographicHash::Sha256).toHex()||sealed.mid(start+64,1)!="\n"){error=QStringLiteral("Synthetic fixture integrity failed.");return {};}return plain;
#endif
}
}
