#pragma once
#include "ForensicAdapters.hpp"
namespace mr::forensic {
// Internal read-only prerequisite, not a registered case adapter yet.
// Whole protected NTFS3 volume-image bytes, 512-byte sectors only.
ForensicAdapterResult inspectNtfsMetadata(const ReadOnlyEvidenceRequest &request);
// Internal acceptance prerequisite: replay against freshly supplied protected
// bytes and exact case/evidence bindings. The expected report digest is only an
// equality check; this function does not authorize access or mint a receipt.
ForensicAdapterResult verifyNtfsMetadataReport(const ReadOnlyEvidenceRequest &request,
 const QByteArray &canonicalReportJson,const QString &expectedReportSha256);
}
