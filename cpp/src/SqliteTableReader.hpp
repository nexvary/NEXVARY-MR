#pragma once
#include "ForensicAdapters.hpp"
#include <QSet>

namespace mr::forensic {
// Internal prerequisite for History, not an allowlisted forensic adapter.
// Offsets refer ONLY to the derived page image, never to original main/WAL bytes.
struct SqliteTableRow {
 QString rowid;
 QVariantList fields;
 QVariantList imageExtents;
 QList<quint64> serialTypes;
 QVariantMap imageCellExtent;
};
struct SqliteTableRead {
 QList<SqliteTableRow> rows;
 QString errorCode;
 QSet<quint32> visitedPages{};
 bool ok() const { return errorCode.isEmpty(); }
};
// Revalidates the protected family before traversing a table root. No SQL,
// filesystem access, deleted-record recovery or database-wide integrity claim.
SqliteTableRead readProtectedSqliteTable(const ReadOnlyEvidenceRequest &request,quint32 rootPage);
}
