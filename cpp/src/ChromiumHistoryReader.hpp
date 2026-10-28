#pragma once
#include "ForensicAdapters.hpp"

namespace mr::forensic {
// Internal bounded extractor. Not registered for case retention or UI use yet.
// Never accepts a path or executes SQL. Observations are source-reported only.
ForensicAdapterResult readProtectedChromiumHistory(const ReadOnlyEvidenceRequest &request);
}
