#pragma once
#include "ForensicAdapters.hpp"
#include "SqliteReaderSupport.hpp"
namespace mr::forensic {
// Internal bounded extractor reused by the case adapter. Protected bytes only.
SqliteColumnPolicy chromiumDownloadsColumnPolicy(const QString &table);
ForensicAdapterResult readProtectedChromiumDownloads(const ReadOnlyEvidenceRequest &request);
}
