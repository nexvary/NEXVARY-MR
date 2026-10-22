#pragma once
#include "DeviceProtocol.hpp"
#include "ServiceOperationPolicy.hpp"

namespace mr::service {
// Require an exact adapter-generated step, including its key and pinned target.
// Unknown tools, action IDs, extra arguments and arbitrary shell text fail closed.
inline bool serviceCommandAllowed(const QString &tool, const QStringList &arguments,
    const QString &action, const QString &serial, const QString &key,
    const QString &package = {}, bool replace = false) {
    if (!operationAllowed(action.toStdString()) || serial.isEmpty()) return false;
    const auto *protocol = adapter(tool);
    if (!protocol) return false;
    if (key == "state") return arguments == protocol->discovery();
    for (const auto &query : protocol->queries(action))
        if (query.key == key && arguments == protocol->addressed(serial, query.arguments)) return true;
    if (key == "execute") {
        const auto command = protocol->command(action, package, replace);
        return !command.isEmpty() && arguments == protocol->addressed(serial, command);
    }
    if (tool == "adb" && key == "verify-night" && (action == "dark_on" || action == "dark_off"))
        return arguments == protocol->addressed(serial, {"shell", "cmd", "uimode", "night"});
    return false;
}
}
