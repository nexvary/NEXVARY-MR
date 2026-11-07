#include "AdbDeviceInventory.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
int main() {
    const auto good = mr::parseAdbDeviceInventory(
        "List of devices attached\n"
        "SERIAL123 device product:foo model:CPH2577 transport_id:1\n"
        "SERIAL456 unauthorized\n");
    if (!good.error.empty() || good.devices.size() != 2 ||
        good.devices[0].model != "CPH2577" ||
        good.devices[0].state != mr::AdbConnectionState::Device ||
        good.devices[1].state != mr::AdbConnectionState::Unauthorized) return EXIT_FAILURE;
    if (!mr::parseAdbDeviceInventory("SERIAL device\nSERIAL offline\n").error.size())
        return EXIT_FAILURE;
    if (!mr::parseAdbDeviceInventory("SERIAL unknown\n").error.size())
        return EXIT_FAILURE;
    if (!mr::parseAdbDeviceInventory(std::string(256 * 1024 + 1, 'x')).error.size())
        return EXIT_FAILURE;
    if (!mr::parseAdbDeviceInventory("SERIAL\n").error.size())
        return EXIT_FAILURE;
    if (!mr::parseAdbDeviceInventory("").devices.empty()) return EXIT_FAILURE;
    std::cout << "ADB inventory parser tests passed\n";
    return EXIT_SUCCESS;
}
