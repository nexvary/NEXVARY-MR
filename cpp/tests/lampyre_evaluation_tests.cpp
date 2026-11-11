#include "../src/osint/LampyreEvaluation.hpp"
#include <iostream>
#include <limits>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << '\n'; return 1; } } while(false)
using namespace mr::osint;
int main() {
    CHECK(!hasVisibleText(" \t\r\n")); CHECK(hasVisibleText("t"));
    ProviderRegistry registry;
    CHECK(!registry.add(nullptr)); CHECK(registry.add(std::make_unique<LampyreProvider>()));
    CHECK(!registry.add(std::make_unique<LampyreProvider>())); CHECK(!registry.find("untrusted"));
    QueryRequest q{Provider::Lampyre, SelectorKind::Email, "research@example.invalid",
        {"SYNTHETIC-CASE", "Synthetic fixture purpose", "synthetic-operator", true}, true, false};
    auto* live = registry.find("lampyre");
    CHECK(!live->capabilities().liveQueries); CHECK(!live->capabilities().mcp);
    CHECK(live->submit(q).error == ProviderError::DocumentationRequired);
    CreditBudgetManager budget(200000000);
    CHECK(!budget.reserve("a", std::nullopt, true)); CHECK(!budget.reserve("a", -1, true));
    CHECK(!budget.reserve("a", 1, false)); CHECK(budget.reserve("a", 35000000, true));
    CHECK(!budget.reserve("a", 35000000, true)); CHECK(budget.remaining() == 165000000);
    CHECK(!budget.settle("a", 35000001)); CHECK(budget.held() == 35000000);
    CHECK(budget.settle("a", 34000000)); CHECK(!budget.settle("a", 1));
    CHECK(!budget.reserve("a", 1, true)); CHECK(budget.spent() == 34000000);
    CHECK(budget.reserve("b", 166000000, true)); CHECK(budget.remaining() == 0);
    CHECK(!budget.reserve("c", 1, true)); CHECK(!budget.releaseConfirmedUncharged("b", false));
    CHECK(budget.held() == 166000000); CHECK(budget.releaseConfirmedUncharged("b", true));
    CreditBudgetManager large(std::numeric_limits<PhotonUnits>::max());
    CHECK(large.reserve("max", std::numeric_limits<PhotonUnits>::max(), true));
    CHECK(!large.reserve("overflow", 1, true)); CHECK(large.remaining() == 0);
    bool threw = false; try { CreditBudgetManager bad(-1); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
    SyntheticLampyreProvider mock;
    CHECK(mock.submit(q).state == QueryState::Running); mock.cancel();
    CHECK(mock.poll().state == QueryState::Cancelled); CHECK(mock.poll().findings.empty());
    CHECK(mock.submit(q).state == QueryState::Running);
    CHECK(mock.poll().state == QueryState::Completed); CHECK(mock.poll().findings.size() == 1);
    CHECK(!mock.poll().findings.front().independentlyVerified);
    q.selector = "real@example.com"; CHECK(mock.submit(q).error == ProviderError::InvalidRequest);
    q.selector = "research@example.invalid";
    for (const auto error : {ProviderError::Authentication, ProviderError::Timeout,
        ProviderError::InsufficientCredits, ProviderError::RateLimited, ProviderError::ResponseTooLarge}) {
        SyntheticLampyreProvider failure(error); CHECK(failure.submit(q).state == QueryState::Running);
        CHECK(failure.poll().error == error); CHECK(failure.poll().findings.empty());
    }
    q.authorization.approvedForExternalDisclosure = false;
    CHECK(mock.submit(q).error == ProviderError::InvalidRequest);
    std::cout << "Lampyre offline registry, lifecycle and credit tests passed\n";
}
