#pragma once
#include <cstddef>
#include <utility>
#include <string>
#include <string_view>
#include <vector>

namespace mr {
enum class AdbConnectionState { Device, Unauthorized, Offline, Recovery, Sideload, Bootloader, Unknown };
struct AdbDeviceEntry {
    std::string serial;
    AdbConnectionState state = AdbConnectionState::Unknown;
    std::string model;
    std::string product;
    std::string transportId;
};
// ADB authorization is required before read-only shell diagnostics.
enum class AdbDiagnosticReadiness {
    Ready,
    NeedsOwnerAuthorization,
    ReconnectRequired,
    RecoveryOnly,
    SideloadOnly,
    UnsupportedState
};
inline AdbDiagnosticReadiness adbDiagnosticReadiness(AdbConnectionState state) {
    switch (state) {
    case AdbConnectionState::Device: return AdbDiagnosticReadiness::Ready;
    case AdbConnectionState::Unauthorized: return AdbDiagnosticReadiness::NeedsOwnerAuthorization;
    case AdbConnectionState::Offline: return AdbDiagnosticReadiness::ReconnectRequired;
    case AdbConnectionState::Recovery: return AdbDiagnosticReadiness::RecoveryOnly;
    case AdbConnectionState::Sideload: return AdbDiagnosticReadiness::SideloadOnly;
    default: return AdbDiagnosticReadiness::UnsupportedState;
    }
}
struct AdbInventoryResult {
    std::vector<AdbDeviceEntry> devices;
    std::string error;
};
inline bool adbSafeField(std::string_view field) {
    if (field.empty() || field.size() > 160) return false;
    for (unsigned char c : field)
        if (c < 33 || c > 126) return false;
    return true;
}
inline AdbConnectionState adbState(std::string_view s) {
    if (s == "device") return AdbConnectionState::Device;
    if (s == "unauthorized") return AdbConnectionState::Unauthorized;
    if (s == "offline") return AdbConnectionState::Offline;
    if (s == "recovery") return AdbConnectionState::Recovery;
    if (s == "sideload") return AdbConnectionState::Sideload;
    if (s == "bootloader") return AdbConnectionState::Bootloader;
    return AdbConnectionState::Unknown;
}
// Parse 'adb devices -l' output as untrusted text. No device operations occur.
inline AdbInventoryResult parseAdbDeviceInventory(std::string_view input) {
    AdbInventoryResult out;
    if (input.size() > 256 * 1024) { out.error = "output-too-large"; return out; }
    std::size_t cursor = 0;
    while (cursor < input.size()) {
        const auto end = input.find('\n', cursor);
        const auto lineEnd = end == std::string_view::npos ? input.size() : end;
        auto line = input.substr(cursor, lineEnd - cursor);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        cursor = end == std::string_view::npos ? input.size() : end + 1;
        if (line.empty() || line.starts_with("List of devices attached") ||
            line.starts_with("* daemon") || line.starts_with("adb server") ||
            line.starts_with("* failed to start daemon")) continue;
        const auto sep = line.find_first_of(" \t");
        if (sep == std::string_view::npos || !adbSafeField(line.substr(0, sep))) {
            out.error = "invalid-device-line"; out.devices.clear(); return out;
        }
        AdbDeviceEntry item;
        item.serial = std::string(line.substr(0, sep));
        auto pos = line.find_first_not_of(" \t", sep);
        if (pos == std::string_view::npos) { out.error = "missing-state"; out.devices.clear(); return out; }
        auto stop = line.find_first_of(" \t", pos);
        const auto state = line.substr(pos, stop == std::string_view::npos ? line.size() - pos : stop - pos);
        item.state = adbState(state);
        if (item.state == AdbConnectionState::Unknown) {
            out.error = "unknown-device-state"; out.devices.clear(); return out;
        }
        while (stop != std::string_view::npos) {
            pos = line.find_first_not_of(" \t", stop);
            if (pos == std::string_view::npos) break;
            stop = line.find_first_of(" \t", pos);
            const auto token = line.substr(pos, stop == std::string_view::npos ? stop : stop - pos);
            const auto colon = token.find(':');
            if (colon == std::string_view::npos) continue;
            const auto key = token.substr(0, colon);
            const auto value = token.substr(colon + 1);
            if (!adbSafeField(value)) continue;
            if (key == "model") item.model = std::string(value);
            else if (key == "product") item.product = std::string(value);
            else if (key == "transport_id") item.transportId = std::string(value);
        }
        for (const auto& existing : out.devices)
            if (existing.serial == item.serial) {
                out.error = "duplicate-serial"; out.devices.clear(); return out;
            }
        if (out.devices.size() >= 128) {
            out.error = "too-many-devices"; out.devices.clear(); return out;
        }
        out.devices.push_back(std::move(item));
    }
    return out;
}
} // namespace mr
