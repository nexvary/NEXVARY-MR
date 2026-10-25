#pragma once
#include <array>
#include <string_view>

// Compile-time profile only. No environment, configuration or QML override.
namespace mr::service {
inline constexpr bool laboratoryBuild() {
#if defined(MR_ENABLE_LAB_OPERATIONS) && MR_ENABLE_LAB_OPERATIONS == 1
    return true;
#else
    return false;
#endif
}
// Separate consented component execution, never a blanket device-write grant.
inline constexpr bool screenViewAllowed() { return true; }
inline constexpr bool operationAllowed(std::string_view operation) {
    if (operation == "probe" || operation == "unlock_ability") return true;
    if (!laboratoryBuild()) return false;
    constexpr std::array writes{
        "reboot_normal", "reboot_recovery", "reboot_bootloader", "reboot_sideload",
        "dark_on", "dark_off", "languages", "keyboard", "settings", "apn",
        "install_apk", "sideload_zip", "fastboot_reboot", "fastboot_bootloader",
        "erase_cache", "erase_userdata", "mirror"};
    for (const auto allowed : writes) if (operation == allowed) return true;
    return false; // Laboratory also denies unknown/protection-bypass operations.
}
}
