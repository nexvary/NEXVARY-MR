#pragma once
#include "ForensicAdapters.hpp"
namespace mr::forensic {
ForensicAdapterResult inspectChromiumHistory(const ReadOnlyEvidenceRequest &request,QVariantMap report);
ForensicAdapterResult validateRetainedChromiumHistory(const QVariantMap &report,const QMap<QString,QVariantMap> &inventory);
QString chromiumHistoryTime(const QString &raw);
}
