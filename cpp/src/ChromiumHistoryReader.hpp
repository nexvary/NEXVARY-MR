#pragma once
#include "ForensicAdapters.hpp"

namespace mr::forensic {
// Internal bounded extractor reused by the production case adapter.
// Only ChromiumHistory.cpp owns case report/retention policy.
// Never accepts a path or executes SQL. Observations are source-reported only.
ForensicAdapterResult readProtectedChromiumHistory(const ReadOnlyEvidenceRequest &request);
}
