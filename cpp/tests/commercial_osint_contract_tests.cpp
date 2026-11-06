#include "../src/osint/CommercialProviderContract.hpp"
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
    std::cout << "Commercial OSINT contract deny-by-default checks passed\n";
}
