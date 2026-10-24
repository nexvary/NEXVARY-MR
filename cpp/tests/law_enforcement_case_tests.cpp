#include "nexvary/LawEnforcementCase.hpp"
#include <cassert>
#include <iostream>

using namespace nexvary::law_enforcement;

int main() {
    CaseRecord record;
    assert(!validateCaseForReview(record).valid);
    record.caseId = "case-internal-1";
    record.caseNumber = "2026-001";
    record.investigatingAgency = "Test agency";
    record.investigatorName = "Test investigator";
    record.seizedDeviceDescription = "Synthetic training device";
    record.authorization.authorityReference = "TEST-ORDER";
    record.authorization.issuingAuthority = "Test authority";
    record.authorization.scope = "Read-only synthetic training data";
    assert(validateCaseForReview(record).valid);
    record.authorization.scope = "  ";
    assert(!validateCaseForReview(record).valid);
    assert(record.status == CaseStatus::Draft);
    std::cout << "Law enforcement case validation tests passed\n";
}
