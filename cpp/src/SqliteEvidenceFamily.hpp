#pragma once
#include "ForensicAdapters.hpp"

namespace mr::forensic {
// A bounded page-image observation, never a live SQLite connection or file.
// WAL checksums establish internal consistency, not authenticity/common acquisition.
struct SqliteFamilySnapshot {
 ForensicAdapterResult analysis;
 QByteArray committedPages;
};
SqliteFamilySnapshot inspectSqliteEvidenceFamily(const ReadOnlyEvidenceRequest &request,QVariantMap report);
ForensicAdapterResult validateRetainedSqliteFamily(const QVariantMap &report,
 const QMap<QString,QVariantMap> &evidenceInventory);
}
