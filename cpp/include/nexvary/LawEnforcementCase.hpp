#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace nexvary::law_enforcement {

enum class CaseStatus { Draft, Authorized, Suspended, Closed };
enum class AccessOutcome { Supported, ResearchRequired, Unsupported };

struct LegalAuthority {
    std::string authorityReference;
    std::string issuingAuthority;
    std::string prosecutorName;
    std::string courtName;
    std::string scope;
    std::string validFromUtc;
    std::string validUntilUtc;
};

struct CaseRecord {
    std::string caseId;
    std::string caseNumber;
    std::string investigatingAgency;
    std::string investigatorName;
    std::string seizedDeviceDescription;
    LegalAuthority authorization;
    CaseStatus status = CaseStatus::Draft;
};

struct ValidationResult {
    bool valid = false;
    std::vector<std::string> errors;
};

inline bool hasText(std::string_view value) {
    return std::any_of(value.begin(), value.end(), [](unsigned char c) {
        return !std::isspace(c);
    });
}

// No device operation is authorized by this model. Authorization requires
// a separate verified workflow and a case-specific scope decision.
inline ValidationResult validateCaseForReview(const CaseRecord& record) {
    ValidationResult result;
    if (!hasText(record.caseId)) result.errors.emplace_back("caseId is required");
    if (!hasText(record.caseNumber)) result.errors.emplace_back("caseNumber is required");
    if (!hasText(record.investigatingAgency)) result.errors.emplace_back("investigatingAgency is required");
    if (!hasText(record.investigatorName)) result.errors.emplace_back("investigatorName is required");
    if (!hasText(record.seizedDeviceDescription)) result.errors.emplace_back("seizedDeviceDescription is required");
    if (!hasText(record.authorization.authorityReference)) result.errors.emplace_back("authorityReference is required");
    if (!hasText(record.authorization.issuingAuthority)) result.errors.emplace_back("issuingAuthority is required");
    if (!hasText(record.authorization.scope)) result.errors.emplace_back("scope is required");
    result.valid = result.errors.empty();
    return result;
}

} // namespace nexvary::law_enforcement
