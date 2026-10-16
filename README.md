# NEXVARY MR — Mobile Repair Workstation

Native **C++20 / Qt 6 Quick** Windows workstation for authorized phone diagnostics, licensed board data and local repair-shop operations. **0.3.0 is an unsigned development build, not a commercially accepted final release.**

## Implemented

- Arabic RTL / English desktop workspace with eight functional sections, keyboard navigation and explicit exit confirmation.
- Async bounded ADB / Fastboot discovery, explicit device selection, read-only properties, 200-line logcat and evidence-based text-pattern indicators, Windows PnP inspection, optional polling and cancellation. Tools can be selected using a native file picker and persisted locally without editing PATH.
- Optional separately installed official scrcpy, launched only on request for an authorized selected device.
- Native component/net board viewer: zoom, pan, search, component/net highlighting, manual measurements, reference comparisons, text/photo annotations and atomic session files. Imports require declared license metadata and explicit permission. The bundled CC0 training board is fictional.
- SQLite v1 migration: customers, devices, repair orders, repair status, technicians, inventory, atomic stock allocation/restocking with historical prices preserved, exact integer monetary values, operation history, CSV export, PDF invoices/printing through a PDF reader, backups and validated restoration with pre-restore snapshots.
- Firmware preflight: exact device brand/model metadata match and cancellable local SHA-256 hash. A matching untrusted manifest **does not authenticate a firmware package**. No flashing is implemented.
- CTest/QtTest/Qt Quick tests and Windows CI deployment/installer smoke tests.

No lock/FRP bypass, device-identity modification, data erasure, automatic driver installation or unrestricted shell command interface is provided. Devices without authorized ADB access, iOS diagnostics, proprietary board formats and real instrument telemetry are unsupported.

## Windows usage

Download the **unsigned portable ZIP** or **unsigned setup EXE** only from the successful [C++/Qt Actions run](https://github.com/nexvary/NEXVARY-MR/actions/workflows/cpp-qt.yml). Extract the portable folder and double-click `NEXVARY-MR.exe`, or run the installer. No Terminal is needed for normal use. Artifacts are published only after the workflow tests pass; an artifact's presence is not physical-device acceptance.

See [user guide](docs/USER_GUIDE.md), [verification gates](docs/REQUIREMENTS.md), [dependency notices](docs/THIRD_PARTY.md), [external integration/license register](docs/INTEGRATIONS.md), [board format](docs/BOARD_FORMAT.md), and [firmware manifest format](docs/FIRMWARE_FORMAT.md).

## Developer build

The Windows CI uses Qt **6.8.3 / MSVC2022 x64**. Linux Qt 6.4.2 is also used for automated local verification. Install Qt Quick, QuickControls2, Sql, Concurrent, Test and QuickTest.

```sh
cmake -S . -B build -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

`MR_WARNINGS_AS_ERRORS=ON` and `MR_SANITIZERS=ON` are available. Core-only builds omit Qt with `MR_BUILD_GUI=OFF`. Release tests use actual checks, including in `NDEBUG` builds.

The Python 0.1 foundation remains available for parity review (`python -m nexvary_mr`); it is not the new commercial runtime. Its regression suite is `python -m unittest discover -s tests -v`. No legacy user database is silently migrated or removed. Importing legacy records into the native schema is not implemented.

## Validation boundaries

GitHub-hosted Windows build, Qt integration/UI tests and isolated portable/installer smoke checks are automated gates. Clean end-user Windows 10/11 VM acceptance, physical phones, OEM drivers, unplug/replug behavior, real licensed board files, external scrcpy controls, code signing, privacy/retention policy and a final dependency audit remain separate release gates. See `docs/VALIDATION.md` for results tied to the development revision.

## Development recovery increment

The ninth workspace now retains source assessment and adds native local image analysis and encrypted Windows cases. See [recovery guide](docs/RECOVERY_GUIDE.md), [research](research/android-recovery/FACTORY_RESET_RESEARCH.md), [tool/license matrix](research/android-recovery/TOOLS.md) and [optional Android companion](recovery-agent/README.md). Existing Python research scanners remain offset-only tools; they are not the desktop processing engine. Builds are unsigned development artifacts. Physical Shizuku/OEM/reset experiments and clean Windows acceptance remain unverified.
