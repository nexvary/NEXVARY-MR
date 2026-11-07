#pragma once
#include <QVariantMap>
#include <atomic>
#include <memory>
namespace nexvary::law {
inline constexpr int timelineEventLimit=2048;
// Pure bounded projection. The caller supplies an already authorized case snapshot.
QVariantMap projectTimeline(const QVariantMap &record,const QString &query,
 const std::shared_ptr<std::atomic_bool> &cancelled={});
}
