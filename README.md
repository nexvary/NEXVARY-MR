# NEXVARY MR — Mobile Repair Workstation

Native **C++20 / Qt 6 Quick** Windows workstation for authorized phone diagnostics, licensed board data and local repair-shop operations. **0.4.0 is an unsigned development build, not a commercially accepted final release.**

## Implemented

- Arabic RTL / English desktop workspace with ten preserved workspaces and device-service/catalog tabs, keyboard navigation and explicit exit confirmation.
- Async bounded ADB / Fastboot discovery, explicit device selection, read-only properties, 200-line logcat and evidence-based text-pattern indicators, Windows PnP inspection, optional polling and cancellation. Tools can be selected using a native file picker and persisted locally without editing PATH.
- Optional separately installed official scrcpy, launched only on request for an authorized selected device.
- Native component/net board viewer: zoom, pan, search, component/net highlighting, manual measurements, reference comparisons, text/photo annotations and atomic session files. Imports require declared license metadata and explicit permission. The bundled CC0 training board is fictional.
- SQLite v1 migration: customers, devices, repair orders, repair status, technicians, inventory, atomic stock allocation/restocking with historical prices preserved, exact integer monetary values, operation history, CSV export, PDF invoices/printing through a PDF reader, backups and validated restoration with pre-restore snapshots.
- Firmware preflight: exact device brand/model metadata match and cancellable local SHA-256 hash. A matching untrusted manifest **does not authenticate a firmware package**. Native preflight does not authenticate OEM signatures. Reviewed ZIP transfer to an already compatible ADB sideload recovery is available separately; transfer completion is not proof of successful flashing.
- CTest/QtTest/Qt Quick tests and Windows CI deployment/installer smoke tests.

No lock/FRP bypass, device-identity modification, automatic driver installation or unrestricted shell command interface is provided. Selected-device services require owner authorization; optional Fastboot cache/userdata erasure requires fresh unlocked/partition checks and explicit serial/data-loss confirmation. Devices without authorized ADB access, iOS diagnostics, proprietary board formats and real instrument telemetry are unsupported.

## Capability-based service increment

Devices & USB now includes services and a searchable reference catalog. The same bounded native process runner uses separate ADB/Fastboot protocol adapters, a 60-second capability snapshot and fresh target/identity checks before execution. Implemented services include information/health observations, reboot requests, dark-mode write/readback, system language/keyboard/APN settings, verified private APK copies and recovery ZIP sideload transfer. Settings availability is probed; languages absent from firmware are not installed.

The data-only catalog covers 14 requested brand groups and 44 model references with evidence labels. It grants no operations or hardware certification. Hash-reviewed catalog updates cannot inject commands. See [competitive research and comparison](docs/device-services/COMPETITIVE_ENGINEERING.md), [service behavior and validation](docs/device-services/IMPLEMENTATION.md), and [catalog data](assets/data/device-service-catalog.json). All phone/OEM acceptance remains **Pending Hardware Validation**.

## Windows usage

Download the **unsigned portable ZIP** or **unsigned setup EXE** only from the successful [C++/Qt Actions run](https://github.com/nexvary/NEXVARY-MR/actions/workflows/cpp-qt.yml). Extract the portable folder and double-click `NEXVARY-MR.exe`, or run the installer. No Terminal is needed for normal use. Use artifacts from a fully successful run for the exact commit; diagnostic artifacts may also be retained on failure. An artifact's presence is not physical-device acceptance.

See [user guide](docs/USER_GUIDE.md), [verification gates](docs/REQUIREMENTS.md), [dependency notices](docs/THIRD_PARTY.md), [external integration/license register](docs/INTEGRATIONS.md), [board format](docs/BOARD_FORMAT.md), and [firmware manifest format](docs/FIRMWARE_FORMAT.md).

## Developer build

The Windows CI uses Qt **6.8.3 / MSVC2022 x64**. Linux Qt 6.4.2 is also used for automated local verification. Install Qt Quick, QuickControls2, Sql, Concurrent, Test and QuickTest.

```sh
python scripts/prepare_sodium.py --destination build/sodium
cmake -S . -B build -DMR_SODIUM_ROOT=build/sodium -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

`MR_WARNINGS_AS_ERRORS=ON` and `MR_SANITIZERS=ON` are available. Core-only builds omit Qt with `MR_BUILD_GUI=OFF`. Release tests use actual checks, including in `NDEBUG` builds.

The separate internal signed-update verifier is opt-in with `MR_BUILD_UPDATE_VERIFIER=ON` and a prepared, pinned `MR_SODIUM_ROOT`; see [UPDATE_VERIFIER.md](docs/UPDATE_VERIFIER.md). Desktop builds now also require that same pinned dependency for local credentials and signed research/audit records; see [LOCAL_IDENTITIES.md](docs/LOCAL_IDENTITIES.md). The update-verifier library itself remains opt-in and separate. Native Production/Laboratory CI and the Linux sanitizer gate explicitly enable and test it. It does not enable installation or supply a production signing key.

The Python 0.1 foundation remains available for parity review (`python -m nexvary_mr`); it is not the new commercial runtime. Its regression suite is `python -m unittest discover -s tests -v`. No legacy user database is silently migrated or removed. Importing legacy records into the native schema is not implemented.

## Validation boundaries

GitHub-hosted Windows build, Qt integration/UI tests and isolated portable/installer smoke checks are automated gates. Clean end-user Windows 10/11 VM acceptance, physical phones, OEM drivers, unplug/replug behavior, real licensed board files, external scrcpy controls, code signing, privacy/retention policy and a final dependency audit remain separate release gates. See `docs/VALIDATION.md` for results tied to the development revision.

## Development recovery increment

The ninth workspace now retains source assessment and adds native local image analysis and encrypted Windows cases. See [recovery guide](docs/RECOVERY_GUIDE.md), [research](research/android-recovery/FACTORY_RESET_RESEARCH.md), [tool/license matrix](research/android-recovery/TOOLS.md) and [optional Android companion](recovery-agent/README.md). Existing Python research scanners remain offset-only tools; they are not the desktop processing engine. Builds are unsigned development artifacts. Physical Shizuku/OEM/reset experiments and clean Windows acceptance remain unverified.


## Device-operation build profiles

The native desktop build defaults to **production-read-only** for connected
phones. Device changes (including reboot, Settings activities, APK installation,
sideload and erase) cannot be enabled through the UI or configuration.
`MR_ENABLE_LAB_OPERATIONS=ON` creates a separate experimental Laboratory binary
and default data directory; model-specific physical acceptance is still pending.
Screen mirroring has an owned, consent-gated Laboratory lifecycle for reviewed
scrcpy 5.0.1; production denies it. Physical frame delivery, device cleanup and
exact-model acceptance remain pending.
See [the current development checkpoint](docs/release/DEVELOPMENT_CHECKPOINT_2026-10-23.md).

### Advanced Mobile Research Lab

Firmware Lab now includes bounded local raw-vbmeta AVB metadata inspection and JSON export. It never treats parsed declarations as signature/OEM trust, rollback safety or phone compatibility. See [scope and tests](research/advanced-mobile/README.md), [traceability registry](research/advanced-mobile/knowledge.json), [competitive evidence](docs/research/COMPETITIVE_EVIDENCE_2026-10-23.md) and [open-source integration decisions](docs/research/OPEN_SOURCE_SELECTION_2026-10-23.md). No physical model is approved by the registry.

### Offline capture summaries

Forensic Lab can now summarize an explicitly authorized local classic-PCAP file (up to 8 MiB / 200,000 records) through the existing case/evidence engine. It records source SHA-256, aggregate packet/time/header statistics and audit evidence after a second matching source read. No addresses or packet payload are included in the summary. PCAPNG, live capture, flow reconstruction and TLS decryption are unsupported. See [scope and test evidence](docs/forensics/PCAP_SUMMARY.md).

Forensic Lab → Tools also reads a bounded JPEG Exif allowlist and stores a
hash-linked case summary after explicit source consent and operator recording.
Raw metadata dates are not authenticated or converted to UTC. GPS values,
serials, owner comments and MakerNotes are omitted. See
[Exif scope](docs/forensics/EXIF_SUMMARY.md) and the current
[requirements gap matrix](docs/REQUIREMENTS_GAP_MATRIX.md).
