#include "../src/osint/CommercialProviderContract.hpp"
#include "../src/osint/CommercialProviderMock.hpp"
#include "../src/osint/OsintSourceRegistry.hpp"
#include "../src/osint/OsintEvidenceRecord.hpp"
#include <cstdlib>
#include <iostream>
using namespace mr::osint;
int main() {
    QueryRequest q{Provider::OsintIndustries, SelectorKind::Phone, "+15551234567",
                   {"CASE-TEST", "AUTH-TEST", "operator-test", true}, true, false};
    PriceEstimate price{"GBP", 100, "credit", true};
    if (!(maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    if (!(!maySubmit(q, price, false, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    if (!(!maySubmit(q, price, true, false))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.authorization.approvedForExternalDisclosure = false;
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.authorization.approvedForExternalDisclosure = true;
    q.authorization.caseId.clear();
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.authorization.caseId = "CASE-TEST";
    q.authorization.authorizationReference.clear();
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.authorization.authorizationReference = "AUTH-TEST";
    q.authorization.operatorId.clear();
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.authorization.operatorId = "operator-test";
    q.operatorConfirmedCost = false;
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.operatorConfirmedCost = true;
    q.selector.clear();
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.selector = "+15551234567";
    price.confirmedByProvider = false;
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    price.confirmedByProvider = true;
    price.minorUnits.reset();
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    price.minorUnits = -1;
    if (!(!maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    price.minorUnits = 0;
    if (!(maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.provider = Provider::Lampyre;
    if (!(maySubmit(q, price, true, true))) { std::cerr << "Contract check failed at line " << __LINE__ << "\n"; return EXIT_FAILURE; }
    q.selector = "   ";
    if (maySubmit(q, price, true, true)) return EXIT_FAILURE;
    q.selector = "+15551234567";
    q.authorization.caseId = "  ";
    if (maySubmit(q, price, true, true)) return EXIT_FAILURE;
    q.authorization.caseId = "CASE-TEST";
    price.currency = " ";
    if (maySubmit(q, price, true, true)) return EXIT_FAILURE;
    price.currency = "GBP";
    price.unit = "\t";
    if (maySubmit(q, price, true, true)) return EXIT_FAILURE;
    price.unit = "credit";
    CommercialProviderMock industries(Provider::OsintIndustries);
    CommercialProviderMock lampyre(Provider::Lampyre);
    auto denied = industries.execute(q, price, false, true);
    if (denied.state != QueryState::AwaitingAuthorization ||
        denied.errorCode != "POLICY_DENIED" || !denied.findings.empty()) return EXIT_FAILURE;
    auto completed = industries.execute(q, price, true, true);
    if (completed.state != QueryState::Completed ||
        completed.providerRequestId != "MOCK-NO-NETWORK" ||
        !completed.findings.empty()) return EXIT_FAILURE;
    auto mismatch = lampyre.execute(q, price, true, true);
    if (mismatch.state != QueryState::Failed ||
        mismatch.errorCode != "PROVIDER_MISMATCH") return EXIT_FAILURE;
    q.provider = Provider::Lampyre;
    if (lampyre.execute(q, price, true, true).state != QueryState::Completed)
        return EXIT_FAILURE;
    if (kSourceRegistry.size() != 8 || findSource("exiftool") == nullptr ||
        findSource("subfinder") == nullptr || findSource("sherlock") == nullptr ||
        findSource("maigret") == nullptr || findSource("spiderfoot") == nullptr ||
        findSource("phoneinfoga") == nullptr ||
        findSource("osint-industries") == nullptr ||
        findSource("lampyre") == nullptr || findSource("unknown") != nullptr)
        return EXIT_FAILURE;
    for (const auto& source : kSourceRegistry) {
        if (source.enabledByDefault || !source.requiresCaseAuthorization)
            return EXIT_FAILURE;
    }
    if (findSource("exiftool")->externalDisclosure ||
        !findSource("subfinder")->externalDisclosure)
        return EXIT_FAILURE;
    EvidenceRecord evidence{"CASE-TEST", "exiftool", "local-fixture",
                            "2026-11-07T00:00:00Z", std::string(64, 'a'),
                            "Synthetic test record"};
    if (!evidenceHasRequiredProvenance(evidence) ||
        evidence.status != EvidenceStatus::UnverifiedLead) return EXIT_FAILURE;
    evidence.sha256Hex = "not-a-hash";
    if (evidenceHasRequiredProvenance(evidence)) return EXIT_FAILURE;
    evidence.sha256Hex = std::string(64, 'a');
    evidence.sourceReference.clear();
    if (evidenceHasRequiredProvenance(evidence)) return EXIT_FAILURE;
    std::cout << "Commercial OSINT contract deny-by-default checks passed\n";
}
