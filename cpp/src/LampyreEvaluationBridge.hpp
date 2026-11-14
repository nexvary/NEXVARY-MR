#pragma once
#include "osint/LampyreEvaluation.hpp"
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QVariantList>
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
                {QStringLiteral("source"), QStringLiteral("fixture:nexvary-intelligence-v1")},
                {QStringLiteral("findings"), static_cast<int>(reply.findings.size())},
                {QStringLiteral("networkRequests"), 0}, {QStringLiteral("photonsConsumed"), 0}};
            if (reply.state == mr::osint::QueryState::Completed) {
                QVariantList records;
                const QString source = QStringLiteral("fixture:nexvary-intelligence-v1");
                const QString at = QStringLiteral("2026-11-11T00:00:00Z (fixture)");
                records.append(QVariantMap{{QStringLiteral("type"), QStringLiteral("Email")}, {QStringLiteral("value"), QStringLiteral("research@example.invalid")}, {QStringLiteral("icon"), QStringLiteral("document_text")}, {QStringLiteral("source"), source}, {QStringLiteral("at"), at}});
                records.append(QVariantMap{{QStringLiteral("type"), QStringLiteral("Domain")}, {QStringLiteral("value"), QStringLiteral("example.invalid")}, {QStringLiteral("icon"), QStringLiteral("search_visual")}, {QStringLiteral("source"), source}, {QStringLiteral("at"), at}});
                records.append(QVariantMap{{QStringLiteral("type"), QStringLiteral("Fixture")}, {QStringLiteral("value"), QStringLiteral("SYNTHETIC-CASE")}, {QStringLiteral("icon"), QStringLiteral("document_lock")}, {QStringLiteral("source"), source}, {QStringLiteral("at"), at}});
                result_.insert(QStringLiteral("records"), records);
            }
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
    Q_INVOKABLE void clear() {
        timer_.stop(); if(provider_)provider_->cancel();provider_.reset();result_.clear();state_=QStringLiteral("trial-pending");emit changed();
    }
signals:
    void changed();
private:
    QString state_ = QStringLiteral("trial-pending");
    QVariantMap result_;
    QTimer timer_;
    std::unique_ptr<mr::osint::SyntheticLampyreProvider> provider_;
};
