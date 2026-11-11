#pragma once
#include "CommercialProviderContract.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>

namespace mr::osint {
// Internal units are millionths of a Photon; never binary floating point.
using PhotonUnits = std::int64_t;
enum class ProviderError { None, DocumentationRequired, Authentication, Timeout,
    InsufficientCredits, RateLimited, Cancelled, InvalidRequest, ResponseTooLarge };
struct ProviderCapabilities {
    std::string id;
    bool liveQueries = false;
    bool mcp = false;
    bool balance = false;
};
struct EvaluationReply {
    QueryState state = QueryState::Disabled;
    ProviderError error = ProviderError::None;
    std::string requestId;
    std::vector<Finding> findings;
};
class IOSINTProvider {
public:
    virtual ~IOSINTProvider() = default;
    virtual ProviderCapabilities capabilities() const = 0;
    virtual EvaluationReply submit(const QueryRequest&) = 0;
    virtual EvaluationReply poll() = 0;
    virtual void cancel() = 0;
};
// Deliberately has no transport, URL, credentials or discovery implementation.
class LampyreProvider final : public IOSINTProvider {
public:
    ProviderCapabilities capabilities() const override { return {"lampyre", false, false, false}; }
    EvaluationReply submit(const QueryRequest&) override {
        return {QueryState::Disabled, ProviderError::DocumentationRequired, {}, {}};
    }
    EvaluationReply poll() override { return submit(QueryRequest{}); }
    void cancel() override {}
};
class ProviderRegistry {
    std::map<std::string, std::unique_ptr<IOSINTProvider>> providers_;
public:
    bool add(std::unique_ptr<IOSINTProvider> provider) {
        if (!provider) return false;
        const auto id = provider->capabilities().id;
        if (!hasVisibleText(id) || providers_.contains(id)) return false;
        providers_.emplace(id, std::move(provider));
        return true;
    }
    IOSINTProvider* find(const std::string& id) const {
        const auto it = providers_.find(id);
        return it == providers_.end() ? nullptr : it->second.get();
    }
};
// One reservation per immutable request ID; uncertain outcomes retain the hold.
// This is an in-memory evaluation ledger, NOT a durable production wallet.
class CreditBudgetManager {
    PhotonUnits maximum_, spent_ = 0, held_ = 0;
    std::map<std::string, PhotonUnits> pending_;
    std::vector<std::string> settled_;
public:
    explicit CreditBudgetManager(PhotonUnits maximum) : maximum_(maximum) {
        if (maximum < 0) throw std::invalid_argument("negative budget");
    }
    PhotonUnits spent() const { return spent_; }
    PhotonUnits held() const { return held_; }
    PhotonUnits remaining() const { return maximum_ - spent_ - held_; }
    bool reserve(const std::string& id, std::optional<PhotonUnits> cost, bool approval) {
        if (!approval || !hasVisibleText(id) || !cost || *cost < 0 ||
            *cost > remaining() || pending_.contains(id) ||
            std::find(settled_.begin(), settled_.end(), id) != settled_.end()) return false;
        pending_.emplace(id, *cost); held_ += *cost; return true;
    }
    bool settle(const std::string& id, PhotonUnits confirmedCost) {
        const auto it = pending_.find(id);
        // A provider overcharge cannot be silently accepted as within budget.
        if (it == pending_.end() || confirmedCost < 0 || confirmedCost > it->second) return false;
        held_ -= it->second; spent_ += confirmedCost;
        pending_.erase(it); settled_.push_back(id); return true;
    }
    bool releaseConfirmedUncharged(const std::string& id, bool providerConfirmed) {
        const auto it = pending_.find(id);
        if (!providerConfirmed || it == pending_.end()) return false;
        held_ -= it->second; pending_.erase(it); settled_.push_back(id); return true;
    }
};
// Synthetic fixture adapter: no selector supplied by a caller is ever used.
class SyntheticLampyreProvider final : public IOSINTProvider {
    EvaluationReply reply_;
    ProviderError scenario_;
public:
    explicit SyntheticLampyreProvider(ProviderError scenario = ProviderError::None) : scenario_(scenario) {}
    ProviderCapabilities capabilities() const override { return {"lampyre-synthetic", false, false, false}; }
    EvaluationReply submit(const QueryRequest& request) override {
        if (reply_.state == QueryState::Running) return reply_;
        if (request.provider != Provider::Lampyre || request.selector != "research@example.invalid" ||
            !hasVisibleText(request.authorization.caseId) ||
            !hasVisibleText(request.authorization.authorizationReference) ||
            !hasVisibleText(request.authorization.operatorId) ||
            !request.authorization.approvedForExternalDisclosure || !request.operatorConfirmedCost) {
            reply_ = {QueryState::Failed, ProviderError::InvalidRequest, {}, {}};
        } else reply_ = {QueryState::Running, ProviderError::None, "SYNTHETIC-NO-NETWORK", {}};
        return reply_;
    }
    EvaluationReply poll() override {
        if (reply_.state != QueryState::Running) return reply_;
        if (scenario_ != ProviderError::None) {
            reply_.state = QueryState::Failed; reply_.error = scenario_;
        } else {
            reply_.state = QueryState::Completed;
            reply_.findings = {{"synthetic-fixture", "fixture:lampyre-evaluation-v1",
                "Synthetic observation; no external search performed", "2026-11-11T00:00:00Z", {}, false}};
        }
        return reply_;
    }
    void cancel() override {
        if (reply_.state == QueryState::Running) {
            reply_.state = QueryState::Cancelled; reply_.error = ProviderError::Cancelled;
            reply_.findings.clear();
        }
    }
};
} // namespace mr::osint
