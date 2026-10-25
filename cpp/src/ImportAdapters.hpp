#pragma once
#include <QByteArray>
#include <QVariantMap>
namespace mr::forensic {
inline constexpr qsizetype importInputLimit=4*1024*1024;
inline constexpr qsizetype importReportLimit=60*1024;
inline constexpr int importFindingLimit=128;
bool importAdapterSupported(const QString &adapter);
// Runs inside the dedicated child. SQLite receives only its private working copy.
QVariantMap parseAuthorizedImport(const QByteArray &bytes,const QString &adapter,const QString &workingPath,QString &error);
}
