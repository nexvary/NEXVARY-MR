#pragma once
#include "osint/LampyreEvaluation.hpp"
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <memory>

// No credential field or network transport is exposed to QML.
class LampyreEvaluationBridge final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QVariantMap result READ result NOTIFY changed)
public:
    explicit LampyreEvaluationBridge(QObject* parent = nullptr) : QObject(parent) {
        timer_.setSingleShot(true);
        connect(&timer_, &QTimer::timeout, this, [this] {
            if (!provider_) return;
            const auto reply = provider_->poll();
            state_ = reply.state == mr::osint::QueryState::Completed ? QStringLiteral("synthetic-completed") : QStringLiteral("synthetic-failed");
            result_ = {{QStringLiteral("classification"), QStringLiteral("synthetic-not-evidence")},
                {QStringLiteral("errorCode"), static_cast<int>(reply.error)},
                {QStringLiteral("source"), QStringLiteral("fixture:lampyre-evaluation-v1")},
                {QStringLiteral("findings"), static_cast<int>(reply.findings.size())},
                {QStringLiteral("networkRequests"), 0}, {QStringLiteral("photonsConsumed"), 0}};
            emit changed();
        });
    }
    QString state() const { return state_; }
    bool busy() const { return timer_.isActive(); }
    QVariantMap result() const { return result_; }
    Q_INVOKABLE void runSynthetic(int scenario) {
        if (busy() || scenario < 0 || scenario > 4) return;
        using namespace mr::osint;
        const ProviderError errors[] = {ProviderError::None, ProviderError::Authentication,
            ProviderError::Timeout, ProviderError::InsufficientCredits, ProviderError::RateLimited};
        provider_ = std::make_unique<SyntheticLampyreProvider>(errors[scenario]);
        const QueryRequest request{Provider::Lampyre, SelectorKind::Email, "research@example.invalid",
            {"SYNTHETIC-CASE", "Offline evaluation fixture", "synthetic-operator", true}, true, false};
        provider_->submit(request);
        state_ = QStringLiteral("synthetic-running"); result_.clear();
        timer_.start(500); emit changed();
    }
    Q_INVOKABLE void cancel() {
        if (!busy()) return;
        timer_.stop(); provider_->cancel(); result_.clear();
        state_ = QStringLiteral("synthetic-cancelled"); emit changed();
    }
signals:
    void changed();
private:
    QString state_ = QStringLiteral("trial-pending");
    QVariantMap result_;
    QTimer timer_;
    std::unique_ptr<mr::osint::SyntheticLampyreProvider> provider_;
};
