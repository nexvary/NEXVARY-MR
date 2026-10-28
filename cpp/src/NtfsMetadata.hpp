#pragma once
#include "ForensicAdapters.hpp"
namespace mr::forensic {
// Internal read-only prerequisite, not a registered case adapter yet.
// Whole protected NTFS3 volume-image bytes, 512-byte sectors only.
ForensicAdapterResult inspectNtfsMetadata(const ReadOnlyEvidenceRequest &request);
}
