#include "../src/osint/CommercialProviderContract.hpp"
#include <cassert>
#include <iostream>
using namespace mr::osint;
int main() {
    QueryRequest q{Provider::OsintIndustries, SelectorKind::Phone, "+15551234567",
                   {"CASE-TEST", "AUTH-TEST", "operator-test", true}, true, false};
    PriceEstimate price{"GBP", 100, "credit", true};
    assert(maySubmit(q, price, true, true));
    assert(!maySubmit(q, price, false, true));
    assert(!maySubmit(q, price, true, false));
    q.authorization.approvedForExternalDisclosure = false;
    assert(!maySubmit(q, price, true, true));
    q.authorization.approvedForExternalDisclosure = true;
    q.authorization.caseId.clear();
    assert(!maySubmit(q, price, true, true));
    q.authorization.caseId = "CASE-TEST";
    q.authorization.authorizationReference.clear();
    assert(!maySubmit(q, price, true, true));
    q.authorization.authorizationReference = "AUTH-TEST";
    q.authorization.operatorId.clear();
    assert(!maySubmit(q, price, true, true));
    q.authorization.operatorId = "operator-test";
    q.operatorConfirmedCost = false;
    assert(!maySubmit(q, price, true, true));
    q.operatorConfirmedCost = true;
    q.selector.clear();
    assert(!maySubmit(q, price, true, true));
    q.selector = "+15551234567";
    price.confirmedByProvider = false;
    assert(!maySubmit(q, price, true, true));
    price.confirmedByProvider = true;
    price.minorUnits.reset();
    assert(!maySubmit(q, price, true, true));
    price.minorUnits = -1;
    assert(!maySubmit(q, price, true, true));
    price.minorUnits = 0;
    assert(maySubmit(q, price, true, true));
    q.provider = Provider::Lampyre;
    assert(maySubmit(q, price, true, true));
    std::cout << "Commercial OSINT contract deny-by-default checks passed\n";
}
