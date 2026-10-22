#pragma once
#include <QString>

namespace mr::service {
inline bool productionOperationAllowed(const QString &operation) {
#ifdef MR_ENABLE_LAB_OPERATIONS
    Q_UNUSED(operation);
    return true;
#else
    return operation == QStringLiteral("probe") || operation == QStringLiteral("unlock_ability");
#endif
}
}
