# OEM support rollout and verification

This document tracks nine delivery gates. A catalog entry is not proof of model compatibility.

1. **CI repair:** close SQLite file handles on Windows; require green Python and Qt checks.
2. **Device targeting:** require a unique ADB/Fastboot serial and an authorized connection.
3. **Read-only diagnostics:** model, manufacturer, Android version, SDK, security patch and bootloader.
4. **Navigation:** expose OEM Service in the desktop navigation; never show unimplemented actions as executable.
5. **Confirmation:** require confirmation for reboot actions and explicit device identity.
6. **OEM adapters:** validate manufacturer, exact model, region, firmware build and transport before implementing an action.
7. **Firmware:** accept only verified OEM packages; validate hash/signature and compatible model, check backups and battery. No unsigned EDL loaders.
8. **Safety:** no account-lock bypass, EFS wipe, IMEI alteration or unsupported carrier unlocks. Use official account recovery and carrier processes.
9. **Release:** Windows build, CI tests, UI navigation checks and authorized physical-device matrix required before claiming release-ready.

## Test matrix

| Device family | Transport | Verified hardware | Status |
| --- | --- | --- | --- |
| Samsung | ADB / Download | No | Pending |
| LG | ADB / OEM recovery | No | Pending |
| Huawei | ADB / OEM recovery | No | Pending |
| Oppo | ADB / OEM recovery | No | Pending |
| Vivo | ADB / OEM recovery | No | Pending |
| ZTE | ADB / OEM recovery | No | Pending |
| Qualcomm chipset | Fastboot / signed EDL | No | Pending |

## Known limitations

* ADB commands require USB debugging enabled and the phone's authorization prompt accepted.
* Fastboot and Download/EDL support varies by model; an Android phone may not implement all reboot targets.
* Language installation depends on ROM support; keyboards do not add missing system locales.
* Factory reset does not remove Factory Reset Protection.
* The current OEM page executes only explicitly allowlisted ADB/Fastboot commands.
