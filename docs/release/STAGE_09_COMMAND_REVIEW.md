# Stage 09 — command safety review (2026-10-22)

Reviewed `cpp/src/DeviceProtocol.hpp` and `cpp/src/DeviceServices.cpp` on `feature/device-service-engine`.

## Verified static properties
- Command plans are constructed from fixed allowlisted actions; arbitrary UI/catalog command text is not passed to adapters.
- ADB actions include reboot, dark-mode changes, settings intents, APK installation and sideload. Fastboot includes reboot, eligibility query and cache/userdata erase.
- The action catalog includes unsupported OEM and protection-related actions explicitly marked unavailable.
- Read-only observations are distinct from device-changing operations.

## Next implementation gate (not yet satisfied)
A production build must enforce an independent, default-off gate for ALL device-changing commands in native C++ (not only hide QML controls). Unit tests must assert that release-profile writes are rejected before spawning a child process, including APK install, sideload, reboot, dark mode, settings and erasure. Only a separate laboratory build/profile may allow them with existing per-operation consent and target checks.

## Evidence needed
Record exact head SHA, compile/test jobs, release-profile negative tests, laboratory positive fixtures, and physical-device results. This review is static and does not certify runtime behavior.
