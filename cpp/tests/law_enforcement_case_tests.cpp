#include "nexvary/LawEnforcementCase.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>

using namespace nexvary::law_enforcement;
namespace {
unsigned checks = 0;
void require(bool condition, std::string_view message) {
    ++checks;
    if (!condition) throw std::runtime_error(std::string(message));
}
CaseRecord completeRecord() {
    CaseRecord record;
    record.caseId = "synthetic-internal-1";
    record.caseNumber = "TEST-2026-001";
    record.investigatingAgency = "Synthetic review agency";
    record.investigatorName = "Synthetic reviewer";
    record.seizedDeviceDescription = "Synthetic training device; no physical phone";
    record.authorization.authorityReference = "SYNTHETIC-REFERENCE";
    record.authorization.issuingAuthority = "Synthetic authority declaration";
    record.authorization.scope = "Review synthetic local fixtures only";
    return record;
}
struct Field {
    const char* name;
    std::size_t limit;
    bool required;
    bool multiline;
    std::string& (*value)(CaseRecord&);
};
const std::array<Field, 10> fields{{
    {"caseId", limits::caseId, true, false, [](CaseRecord& r) -> std::string& { return r.caseId; }},
    {"caseNumber", limits::caseNumber, true, false, [](CaseRecord& r) -> std::string& { return r.caseNumber; }},
    {"investigatingAgency", limits::investigatingAgency, true, false, [](CaseRecord& r) -> std::string& { return r.investigatingAgency; }},
    {"investigatorName", limits::investigatorName, true, false, [](CaseRecord& r) -> std::string& { return r.investigatorName; }},
    {"seizedDeviceDescription", limits::seizedDeviceDescription, true, true, [](CaseRecord& r) -> std::string& { return r.seizedDeviceDescription; }},
    {"authorityReference", limits::authorityReference, true, false, [](CaseRecord& r) -> std::string& { return r.authorization.authorityReference; }},
    {"issuingAuthority", limits::issuingAuthority, true, false, [](CaseRecord& r) -> std::string& { return r.authorization.issuingAuthority; }},
    {"prosecutorName", limits::prosecutorName, false, false, [](CaseRecord& r) -> std::string& { return r.authorization.prosecutorName; }},
    {"courtName", limits::courtName, false, false, [](CaseRecord& r) -> std::string& { return r.authorization.courtName; }},
    {"scope", limits::scope, true, true, [](CaseRecord& r) -> std::string& { return r.authorization.scope; }}
}};
void completenessAndBounds() {
    const auto missing = validateCaseForReview({});
    require(!missing.valid && missing.errors.size() == 8, "empty case must identify eight required fields");
    const auto complete = completeRecord();
    require(validateCaseForReview(complete).valid, "optional prosecutor/court/dates must not become requirements");
    require(complete.status == CaseStatus::Draft, "draft default changed");
    for (const auto& field : fields) {
        auto record = complete;
        field.value(record).clear();
        require(validateCaseForReview(record).valid == !field.required, std::string(field.name) + " optionality changed");
        record = complete;
        field.value(record) = std::string(field.limit, 'x');
        require(validateCaseForReview(record).valid, std::string(field.name) + " exact bound refused");
        field.value(record).push_back('x');
        const auto oversized = validateCaseForReview(record);
        require(!oversized.valid, std::string(field.name) + " oversized value accepted");
        require(std::any_of(oversized.errors.begin(), oversized.errors.end(), [&](const auto& error) {
            return error.starts_with(field.name);
        }), "validation error did not identify its field");
        if (field.required) {
            field.value(record) = "   ";
            require(!validateCaseForReview(record).valid, "ASCII-whitespace-only required field accepted");
            field.value(record) = "\xc2\xa0\xe3\x80\x80";
            require(!validateCaseForReview(record).valid, "Unicode-whitespace-only required field accepted");
        }
    }
}
void unicodeAndControls() {
    const std::array<std::string, 10> invalidUtf8{{
        "\x80", "\xc0\xaf", "\xe0\x80\xaf", "\xf0\x80\x80\xaf", "\xed\xa0\x80",
        "\xf4\x90\x80\x80", "\xf5\x80\x80\x80", "\xe2\x82", "\xe2\x28\xa1", std::string("A\0B", 3)
    }};
    const std::array<std::string, 5> validUtf8{{"محقق تجريبي", "审查测试", "Reviewer é", "Device \xf0\x9f\x94\x92", "\xf4\x8f\xbf\xbf"}};
    const std::array<std::string, 12> controls{{
        std::string(1, '\x01'), std::string(1, '\x1b'), std::string(1, '\x1f'), std::string(1, '\x7f'),
        "\xc2\x85", "\xe2\x80\xae", "\xe2\x81\xa6", "\xef\xbb\xbf",
        "\t", "\n", "\r", "\xe2\x80\xa8"
    }};
    for (const auto& field : fields) {
        for (const auto& value : invalidUtf8) {
            auto record = completeRecord();field.value(record) = value;
            require(!validateCaseForReview(record).valid, std::string(field.name) + " accepted malformed UTF-8 or NUL");
        }
        for (const auto& value : validUtf8) {
            auto record = completeRecord();field.value(record) = value;
            require(validateCaseForReview(record).valid, std::string(field.name) + " refused valid international text");
        }
        for (const auto& value : controls) {
            auto record = completeRecord();field.value(record) = "A" + value + "B";
            const bool proseWhitespace = field.multiline && (value == "\t" || value == "\n" || value == "\r" || value == "\xe2\x80\xa8");
            require(validateCaseForReview(record).valid == proseWhitespace, std::string(field.name) + " control policy mismatch");
        }
    }
    auto record = completeRecord();
    record.investigatorName.clear();
    for (std::size_t index = 0; index < limits::investigatorName / 2; ++index) record.investigatorName += "é";
    require(validateCaseForReview(record).valid, "UTF-8 exact byte limit refused");
    record.investigatorName += "é";
    require(!validateCaseForReview(record).valid, "UTF-8 byte limit counted characters instead of bytes");
    require(hasText("محقق"), "Arabic text not recognized");
    require(!hasText("\xc2\xa0"), "NBSP-only text accepted");
    require(!hasText("\xe2\x80\x8d"), "joiner-only text accepted");
    require(!hasText("\xe2\x82"), "malformed UTF-8 accepted by hasText");
}
void datesAndOrdering() {
    const std::array<std::string_view, 7> valid{{
        "0001-01-01T00:00:00Z", "2000-02-29T23:59:59Z", "2024-02-29T12:34:56.1Z",
        "2026-10-24T00:00:00.000Z", "2026-10-24T00:00:00.123456789Z", "9999-12-31T23:59:59Z", "1900-02-28T00:00:00Z"
    }};
    const std::array<std::string_view, 19> invalid{{
        "", "0000-01-01T00:00:00Z", "1900-02-29T00:00:00Z", "2026-02-29T00:00:00Z", "2026-04-31T00:00:00Z",
        "2026-00-01T00:00:00Z", "2026-13-01T00:00:00Z", "2026-01-00T00:00:00Z", "2026-01-01T24:00:00Z",
        "2026-01-01T00:60:00Z", "2026-01-01T00:00:60Z", "2026-01-01T00:00:00z", "2026-01-01T00:00:00+00:00",
        "2026-01-01 00:00:00Z", "2026-01-01T00:00:00.Z", "2026-01-01T00:00:00.1234567890Z",
        "2026-01-01T00:00:00.XZ", "2026-01-01T00:00:00Z ", "2026-01-01T00:00:00"
    }};
    for (const auto value : valid) {
        require(isValidUtcTimestamp(value), "valid UTC timestamp refused");
        auto record = completeRecord();record.authorization.validFromUtc = value;
        require(validateCaseForReview(record).valid, "valid from-only timestamp refused");
        record.authorization.validFromUtc.clear();record.authorization.validUntilUtc = value;
        require(validateCaseForReview(record).valid, "valid until-only timestamp refused");
    }
    for (const auto value : invalid) {
        require(!isValidUtcTimestamp(value), "invalid UTC timestamp accepted");
        if (value.empty()) continue; // Empty case endpoints are deliberately optional.
        auto record = completeRecord();record.authorization.validFromUtc = value;
        require(!validateCaseForReview(record).valid, "invalid from endpoint accepted");
        record.authorization.validFromUtc.clear();record.authorization.validUntilUtc = value;
        require(!validateCaseForReview(record).valid, "invalid until endpoint accepted");
    }
    auto record = completeRecord();
    record.authorization.validFromUtc = "2026-10-24T12:00:00.1Z";
    record.authorization.validUntilUtc = "2026-10-24T12:00:00.100Z";
    require(validateCaseForReview(record).valid, "equivalent fractions were ordered lexically");
    record.authorization.validFromUtc = "2026-10-24T12:00:00.100000001Z";
    require(!validateCaseForReview(record).valid, "reversed nanosecond endpoints accepted");
    record.authorization.validFromUtc = "2027-01-01T00:00:00Z";
    record.authorization.validUntilUtc = "2026-12-31T23:59:59Z";
    require(!validateCaseForReview(record).valid, "reversed calendar endpoints accepted");
    record.authorization.validFromUtc = "1900-01-01T00:00:00Z";
    record.authorization.validUntilUtc = "1900-01-02T00:00:00Z";
    require(validateCaseForReview(record).valid, "structural validation improperly inferred present-time permission or expiry");
    record.authorization.validUntilUtc = std::string("2026-01-01T00:00:00Z\0", 21);
    require(!validateCaseForReview(record).valid, "UTC endpoint with embedded NUL accepted");
    record.authorization.validUntilUtc = std::string(100000, 'x');
    require(!validateCaseForReview(record).valid, "oversized UTC endpoint accepted");
}
void enumsAndReviewBoundary() {
    require(static_cast<int>(CaseStatus::Draft) == 0 && static_cast<int>(CaseStatus::Authorized) == 1 &&
            static_cast<int>(CaseStatus::Suspended) == 2 && static_cast<int>(CaseStatus::Closed) == 3 &&
            static_cast<int>(CaseStatus::Archived) == 4, "persisted status values changed");
    for (const auto status : {CaseStatus::Draft, CaseStatus::Authorized, CaseStatus::Suspended, CaseStatus::Closed, CaseStatus::Archived}) {
        auto record = completeRecord();record.status = status;
        require(isValidCaseStatus(status) && validateCaseForReview(record).valid, "valid declaration status rejected");
    }
    for (const int value : {-1, 5, 100}) {
        auto record = completeRecord();record.status = static_cast<CaseStatus>(value);
        require(!isValidCaseStatus(record.status) && !validateCaseForReview(record).valid, "unknown status accepted");
    }
    for (const auto outcome : {AccessOutcome::Supported, AccessOutcome::ResearchRequired, AccessOutcome::Unsupported})
        require(isValidAccessOutcome(outcome), "valid outcome enum refused");
    require(!isValidAccessOutcome(static_cast<AccessOutcome>(-1)), "unknown outcome enum accepted");
    require(!isValidAccessOutcome(static_cast<AccessOutcome>(3)), "unknown outcome enum accepted");
    auto record = completeRecord();record.status = CaseStatus::Authorized;
    record.authorization.scope.clear();
    require(!validateCaseForReview(record).valid, "Authorized declaration bypassed structural completeness");
}
} // namespace

int main() {
    try {
        completenessAndBounds();unicodeAndControls();datesAndOrdering();enumsAndReviewBoundary();
        std::cout << checks << " law-enforcement structural validation checks passed; no legal/device authorization inferred\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Law-enforcement validation failure after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
