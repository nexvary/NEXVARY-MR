#pragma once
#include "ForensicAdapters.hpp"
namespace mr::forensic {
ForensicAdapterResult inspectChromiumDownloads(const ReadOnlyEvidenceRequest &request,QVariantMap report);
ForensicAdapterResult validateRetainedChromiumDownloads(const QVariantMap &report,const QMap<QString,QVariantMap> &inventory);
}
