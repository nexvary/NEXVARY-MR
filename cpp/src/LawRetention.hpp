#pragma once
#include <QVariantMap>
#include <QDateTime>
namespace nexvary::law {
// Pure, offline assessment. Only the native bridge supplies the operational clock.
QVariantMap assessRetention(const QVariantMap &record,const QString &evidenceId,const QDateTime &asOf);
bool validRetention(const QVariantMap &record);
}
