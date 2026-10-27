#pragma once
#include <QByteArray>
#include <QVariantMap>

namespace mr::forensic {
// Inventory only. Never creates archive members or follows links.
bool inventoryAdbBackup(const QByteArray &bytes, QVariantList &findings,
                        int &observedCount, QVariantMap &summary);
}
