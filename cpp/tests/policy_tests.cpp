#include "ServiceOperationPolicy.hpp"
#include <iostream>
int main() {
    using namespace mr::service;
    for (const auto read : {"probe", "unlock_ability"})
        if (!operationAllowed(read)) return 1;
    for (const auto write : {"reboot_normal", "reboot_recovery", "reboot_bootloader", "reboot_sideload",
        "dark_on", "dark_off", "languages", "keyboard", "settings", "apn", "install_apk",
        "sideload_zip", "fastboot_reboot", "fastboot_bootloader", "erase_cache", "erase_userdata", "mirror"})
        if (operationAllowed(write) != laboratoryBuild()) return 2;
    for (const auto unknown : {"", "PROBE", "probe ", "shell", "frp", "lockscreen", "edl",
        "bootloader_unlock", "bootloader_relock", "factory_reset", "format", "flash", "install", "probe;reboot"})
        if (operationAllowed(unknown)) return 3;
    std::cout << (laboratoryBuild() ? "laboratory" : "production-read-only") << ": policy passed\n";
}
