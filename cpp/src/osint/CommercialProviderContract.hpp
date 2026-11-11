#pragma once
// NEXVARY-MR: provider-neutral commercial OSINT contract.
// No vendor credentials, network calls, or paid lookups are performed here.
#include <cstdint>
#include <string_view>
#include <optional>
#include <string>
#include <vector>

namespace mr::osint {
enum class Provider { OsintIndustries, Lampyre };
enum class SelectorKind { Phone, Email, Username, Name, Wallet };
enum class QueryState { Disabled, AwaitingAuthorization, AwaitingCostApproval,
                        Ready, Running, Completed, Failed, Cancelled };
struct CaseAuthorization {
    std::string caseId;
    std::string authorizationReference;
    std::string operatorId;
    bool approvedForExternalDisclosure = false;
};
struct PriceEstimate {
    std::string currency;
    std::optional<std::int64_t> minorUnits;
    std::string unit;
    bool confirmedByProvider = false;
};
struct QueryRequest {
    Provider provider;
    SelectorKind kind;
    std::string selector;
    CaseAuthorization authorization;
    bool operatorConfirmedCost = false;
    bool premiumModules = false;
};
struct Finding {
    std::string sourceProvider;
    std::string sourceReference;
    std::string observation;
    std::string collectedAtUtc;
    std::string rawResponseSha256;
    bool independentlyVerified = false;
};
struct QueryResult {
    QueryState state = QueryState::Disabled;
    std::vector<Finding> findings;
    std::string errorCode;
    std::string providerRequestId;
};
inline bool hasVisibleText(std::string_view value) {
    return value.find_first_not_of(" \t\r\n") != std::string_view::npos;
}
inline bool maySubmit(const QueryRequest& request, const PriceEstimate& estimate,
                      bool credentialsConfigured, bool contractPermitsIntegration) {
    return credentialsConfigured && contractPermitsIntegration
        && hasVisibleText(request.selector)
        && hasVisibleText(request.authorization.caseId)
        && hasVisibleText(request.authorization.authorizationReference)
        && hasVisibleText(request.authorization.operatorId)
        && request.authorization.approvedForExternalDisclosure
        && request.operatorConfirmedCost
        && hasVisibleText(estimate.currency)
        && hasVisibleText(estimate.unit)
        && estimate.confirmedByProvider
        && estimate.minorUnits.has_value()
        && *estimate.minorUnits >= 0;
}
} // namespace mr::osint
