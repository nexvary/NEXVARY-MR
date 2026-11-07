#pragma once
#include "CommercialProviderContract.hpp"

namespace mr::osint {
// Deterministic local test double. No network access or external lookups.
class CommercialProviderMock {
public:
    explicit CommercialProviderMock(Provider provider) : provider_(provider) {}
    QueryResult execute(const QueryRequest& request, const PriceEstimate& price,
                        bool credentialsConfigured, bool contractPermitsIntegration) const {
        QueryResult result;
        if (request.provider != provider_) {
            result.state = QueryState::Failed;
            result.errorCode = "PROVIDER_MISMATCH";
            return result;
        }
        if (!maySubmit(request, price, credentialsConfigured, contractPermitsIntegration)) {
            result.state = QueryState::AwaitingAuthorization;
            result.errorCode = "POLICY_DENIED";
            return result;
        }
        result.state = QueryState::Completed;
        result.providerRequestId = "MOCK-NO-NETWORK";
        return result;
    }
private:
    Provider provider_;
};
} // namespace mr::osint
