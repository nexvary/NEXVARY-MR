#pragma once
#include <QVariantMap>
#include <atomic>
#include <memory>
namespace nexvary::law {
inline constexpr int artifactRowLimit = 2048;
// Pure, bounded, case-scoped derived view. No file/database/network access.
QVariantMap projectArtifacts(const QVariantMap &record, const QString &query,
    const std::shared_ptr<std::atomic_bool> &cancelled = {});
}
