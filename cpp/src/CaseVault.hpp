#pragma once
#include <QByteArray>
#include <QString>
namespace mr::recovery {
// DPAPI current-user protection on Windows. No plaintext production fallback.
bool vaultAvailable();
QByteArray sealCase(const QByteArray &plain,bool syntheticProfile,QString &error);
QByteArray openCaseBytes(const QByteArray &sealed,bool syntheticProfile,QString &error);
}
