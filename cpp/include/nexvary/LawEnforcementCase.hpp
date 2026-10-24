#pragma once
#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace nexvary::law_enforcement {

// Existing persisted values are stable. "Authorized" records an operator's
// status declaration; it never grants a legal entitlement or device capability.
enum class CaseStatus { Draft = 0, Authorized = 1, Suspended = 2, Closed = 3, Archived = 4 };
enum class AccessOutcome { Supported = 0, ResearchRequired = 1, Unsupported = 2 };

namespace limits {
inline constexpr std::size_t caseId = 128;
inline constexpr std::size_t caseNumber = 128;
inline constexpr std::size_t investigatingAgency = 256;
inline constexpr std::size_t investigatorName = 256;
inline constexpr std::size_t seizedDeviceDescription = 2048;
inline constexpr std::size_t authorityReference = 256;
inline constexpr std::size_t issuingAuthority = 256;
inline constexpr std::size_t prosecutorName = 256;
inline constexpr std::size_t courtName = 256;
inline constexpr std::size_t scope = 8192;
inline constexpr std::size_t utcTimestamp = 30;
} // namespace limits

struct LegalAuthority {
    std::string authorityReference;
    std::string issuingAuthority;
    std::string prosecutorName; // Optional; no prosecutor or conviction requirement.
    std::string courtName;      // Optional; no court or conviction requirement.
    std::string scope;
    std::string validFromUtc;   // Optional UTC observation; not a permission decision.
    std::string validUntilUtc;  // Optional UTC observation; not an expiry decision.
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
    // Structurally complete enough for review only. This does not establish
    // legal validity, identity, ownership, present-time validity or authorization.
    bool valid = false;
    std::vector<std::string> errors;
};

inline bool isValidCaseStatus(CaseStatus value) {
    switch (value) {
    case CaseStatus::Draft:
    case CaseStatus::Authorized:
    case CaseStatus::Suspended:
    case CaseStatus::Closed:
    case CaseStatus::Archived: return true;
    }
    return false;
}

inline bool isValidAccessOutcome(AccessOutcome value) {
    switch (value) {
    case AccessOutcome::Supported:
    case AccessOutcome::ResearchRequired:
    case AccessOutcome::Unsupported: return true;
    }
    return false;
}

namespace detail {
struct TextInspection { bool wellFormed = true; bool hasVisibleText = false; };

inline bool unicodeWhitespace(unsigned codepoint) {
    return codepoint == 0x20 || codepoint == 0x09 || codepoint == 0x0a || codepoint == 0x0d ||
           codepoint == 0xa0 || codepoint == 0x1680 ||
           (codepoint >= 0x2000 && codepoint <= 0x200a) ||
           codepoint == 0x2028 || codepoint == 0x2029 || codepoint == 0x202f ||
           codepoint == 0x205f || codepoint == 0x3000;
}

// Reject invalid/overlong UTF-8, surrogate scalars, NUL, C0/C1 controls,
// directional overrides/isolates and a BOM. Only prose fields admit TAB/LF/CR.
// Joiners/marks may occur in international names but cannot alone fill a field.
inline TextInspection inspectText(std::string_view value, bool multiline) {
    TextInspection result;
    std::size_t cursor = 0;
    while (cursor < value.size()) {
        const auto lead = static_cast<unsigned char>(value[cursor++]);
        unsigned codepoint = lead;
        unsigned continuation = 0;
        unsigned minimum = 0;
        if (lead >= 0xc2 && lead <= 0xdf) { codepoint = lead & 0x1fU; continuation = 1; minimum = 0x80; }
        else if (lead >= 0xe0 && lead <= 0xef) { codepoint = lead & 0x0fU; continuation = 2; minimum = 0x800; }
        else if (lead >= 0xf0 && lead <= 0xf4) { codepoint = lead & 0x07U; continuation = 3; minimum = 0x10000; }
        else if (lead >= 0x80) return {false, false};
        if (continuation > value.size() - cursor) return {false, false};
        for (unsigned index = 0; index < continuation; ++index) {
            const auto byte = static_cast<unsigned char>(value[cursor++]);
            if ((byte & 0xc0U) != 0x80U) return {false, false};
            codepoint = (codepoint << 6U) | (byte & 0x3fU);
        }
        if (codepoint < minimum || codepoint > 0x10ffff ||
            (codepoint >= 0xd800 && codepoint <= 0xdfff)) return {false, false};
        const bool proseBreak = multiline && (codepoint == 0x09 || codepoint == 0x0a || codepoint == 0x0d);
        if ((codepoint < 0x20 && !proseBreak) || (codepoint >= 0x7f && codepoint <= 0x9f) ||
            (!multiline && (codepoint == 0x2028 || codepoint == 0x2029)) ||
            (codepoint >= 0x202a && codepoint <= 0x202e) ||
            (codepoint >= 0x2066 && codepoint <= 0x2069) || codepoint == 0xfeff) return {false, false};
        const bool formatOnly = (codepoint >= 0x200b && codepoint <= 0x200f) ||
                                (codepoint >= 0x2060 && codepoint <= 0x2064) || codepoint == 0x061c;
        if (!unicodeWhitespace(codepoint) && !formatOnly) result.hasVisibleText = true;
    }
    return result;
}

using UtcParts = std::array<unsigned, 7>; // year, month, day, hour, minute, second, nanoseconds

inline std::optional<UtcParts> parseUtc(std::string_view value) {
    if (value.size() < 20 || value.size() > limits::utcTimestamp || value.back() != 'Z' ||
        value[4] != '-' || value[7] != '-' || value[10] != 'T' || value[13] != ':' || value[16] != ':') return std::nullopt;
    UtcParts parts{};
    constexpr std::array<std::size_t, 6> starts{0, 5, 8, 11, 14, 17};
    constexpr std::array<std::size_t, 6> lengths{4, 2, 2, 2, 2, 2};
    for (std::size_t index = 0; index < starts.size(); ++index) {
        for (std::size_t digit = starts[index]; digit < starts[index] + lengths[index]; ++digit) {
            if (value[digit] < '0' || value[digit] > '9') return std::nullopt;
            parts[index] = parts[index] * 10U + static_cast<unsigned>(value[digit] - '0');
        }
    }
    if (parts[0] == 0 || parts[3] > 23 || parts[4] > 59 || parts[5] > 59) return std::nullopt;
    const std::chrono::year_month_day date{std::chrono::year{static_cast<int>(parts[0])},
                                         std::chrono::month{parts[1]}, std::chrono::day{parts[2]}};
    if (!date.ok()) return std::nullopt;
    if (value.size() != 20) {
        if (value.size() < 22 || value[19] != '.') return std::nullopt;
        const auto count = value.size() - 21;
        for (std::size_t digit = 20; digit < value.size() - 1; ++digit) {
            if (value[digit] < '0' || value[digit] > '9') return std::nullopt;
            parts[6] = parts[6] * 10U + static_cast<unsigned>(value[digit] - '0');
        }
        for (std::size_t pad = count; pad < 9; ++pad) parts[6] *= 10U;
    } else if (value[19] != 'Z') return std::nullopt;
    return parts;
}

inline void validateText(std::vector<std::string>& errors, std::string_view name,
                         std::string_view value, std::size_t maximumBytes,
                         bool required = true, bool multiline = false) {
    if (value.size() > maximumBytes) {
        errors.emplace_back(std::string(name) + " exceeds its UTF-8 byte limit");
        return;
    }
    const auto inspected = inspectText(value, multiline);
    if (!inspected.wellFormed) errors.emplace_back(std::string(name) + " contains invalid UTF-8 or prohibited control characters");
    else if (required && !inspected.hasVisibleText) errors.emplace_back(std::string(name) + " is required");
}
} // namespace detail

inline bool hasText(std::string_view value) {
    const auto inspected = detail::inspectText(value, true);
    return inspected.wellFormed && inspected.hasVisibleText;
}

inline bool isValidUtcTimestamp(std::string_view value) {
    return detail::parseUtc(value).has_value();
}

// No device operation is authorized by this model. Authorization requires a
// separate verified workflow and case-specific scope decision. Optional dates
// are syntactically checked and ordered, never compared against the current clock.
inline ValidationResult validateCaseForReview(const CaseRecord& record) {
    ValidationResult result;
    detail::validateText(result.errors, "caseId", record.caseId, limits::caseId);
    detail::validateText(result.errors, "caseNumber", record.caseNumber, limits::caseNumber);
    detail::validateText(result.errors, "investigatingAgency", record.investigatingAgency, limits::investigatingAgency);
    detail::validateText(result.errors, "investigatorName", record.investigatorName, limits::investigatorName);
    detail::validateText(result.errors, "seizedDeviceDescription", record.seizedDeviceDescription, limits::seizedDeviceDescription, true, true);
    detail::validateText(result.errors, "authorityReference", record.authorization.authorityReference, limits::authorityReference);
    detail::validateText(result.errors, "issuingAuthority", record.authorization.issuingAuthority, limits::issuingAuthority);
    detail::validateText(result.errors, "prosecutorName", record.authorization.prosecutorName, limits::prosecutorName, false);
    detail::validateText(result.errors, "courtName", record.authorization.courtName, limits::courtName, false);
    detail::validateText(result.errors, "scope", record.authorization.scope, limits::scope, true, true);
    if (!isValidCaseStatus(record.status)) result.errors.emplace_back("status is invalid");
    const auto& from = record.authorization.validFromUtc;
    const auto& until = record.authorization.validUntilUtc;
    const auto parsedFrom = from.empty() ? std::optional<detail::UtcParts>{} : detail::parseUtc(from);
    const auto parsedUntil = until.empty() ? std::optional<detail::UtcParts>{} : detail::parseUtc(until);
    if (!from.empty() && !parsedFrom) result.errors.emplace_back("validFromUtc must be an ISO UTC timestamp ending in Z");
    if (!until.empty() && !parsedUntil) result.errors.emplace_back("validUntilUtc must be an ISO UTC timestamp ending in Z");
    if (parsedFrom && parsedUntil && *parsedFrom > *parsedUntil) result.errors.emplace_back("validUntilUtc precedes validFromUtc");
    result.valid = result.errors.empty();
    return result;
}

} // namespace nexvary::law_enforcement
