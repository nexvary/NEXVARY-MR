#pragma once
#include <string>
#include <string_view>

namespace mr::osint {
enum class EvidenceStatus { UnverifiedLead, Corroborated, Rejected };
struct EvidenceRecord {
    std::string caseId;
    std::string sourceId;
    std::string sourceReference;
    std::string collectedAtUtc;
    std::string sha256Hex;
    std::string observation;
    EvidenceStatus status = EvidenceStatus::UnverifiedLead;
};
inline bool isHexDigest(std::string_view value) {
    if (value.size() != 64) return false;
    for (const char ch : value) {
        if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
              (ch >= 'A' && ch <= 'F'))) return false;
    }
    return true;
}
inline bool evidenceHasRequiredProvenance(const EvidenceRecord& record) {
    return !record.caseId.empty() && !record.sourceId.empty() &&
           !record.sourceReference.empty() && !record.collectedAtUtc.empty() &&
           isHexDigest(record.sha256Hex);
}
} // namespace mr::osint
