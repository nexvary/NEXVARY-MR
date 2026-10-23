# Modern Android observation increment — 2026-10-23

Branch `integration/gate-1-stages-08-60`, draft [PR #8](https://github.com/nexvary/NEXVARY-MR/pull/8). Starting HEAD was rechecked as `9f9298351c12c4107d05ca2b85a9aa2ff047f346`. This commit extends stages 19–24 and native safety/UI fixtures; it does not complete physical-device or release gates. PR #8 records the resulting exact commit and remote CI links.

## What changes for newer phones

- Native compatibility observations recognize Android API families through **37 / Android 17**, including 15/35 and 16/36. Recognition uses the SDK property, not a marketing/model name. Unknown future APIs remain readable but unrecognized; no OEM approval is granted. Preview, first-API inconsistencies and missing/invalid values remain explicit. Integer API 36 does not identify minor API 36.1.
- Existing authorized ADB probe reads selected dynamic-partition, A/B, Virtual A/B, slot, verified-boot and encryption properties. A single optional, exact `shell getconf PAGE_SIZE` query adds observed page size, including synthetic 16 KiB coverage. Optional unsupported queries do not fabricate data.
- Duplicate allowlisted properties are discarded. Fastboot variables with duplicate matches are unknown. Fastbootd/unknown mode permits probing but no non-probe operation; fresh `is-userspace` checks prevent a reviewed bootloader snapshot being used after switching modes. Legacy bootloaders omitting this variable are intentionally restricted too.
- Compile-time production read-only policy, owner consent, pinned serial, ambiguity checks, snapshot expiry, queue cancellation/deadlines and exact command plans are retained. No arbitrary shell, device-side agent or download/flash/bypass adapter was added.
- AR/EN UI displays the native profile and pending physical acceptance. Technical JSON exports include an explicit compatibility projection, excluding arbitrary nested fields and raw outputs. Boot colors and encryption flags are device reports, not attestation or proof of protected-data access.
- Reference catalog grows from 44 to **48 references**, with Pixel 11 and Galaxy S26 / S26+ / S26 Ultra from official product sources. All remain `nexvaryValidated:false`. Google, OnePlus, Nothing and Honor recognition/reference filters and Google's official Windows driver source were added. Filters are not driver/protocol acceptance; older user-imported catalogs can still override the bundled reference data. Compatibility probing is independent of catalog entries.
- [Primary-source review](../research/MODERN_ANDROID_COMPATIBILITY_2026-10-23.md) and research knowledge graph connect the implementation to synthetic evidence without populating physical device/build approvals.

## Executed validation

Ubuntu 24.04, GCC 13, Qt 6.4.2, Release, `MR_WARNINGS_AS_ERRORS=ON`; separate production and Laboratory builds. Build source matches the code files uploaded in this increment. CTest ran all groups after fixing one stale UI driver-count assertion introduced by adding Google; no test was disabled. Qt case totals include init/cleanup and data rows.

| Check | Production | Laboratory |
| --- | --- | --- |
| Full CTest | **22/22 passed**, 107.05 s | **22/22 passed**, 117.81 s |
| New native compatibility suite | 16 passed, no failure/skip | 16 passed, no failure/skip |
| Native service fixtures | 35 passed; 21 Laboratory-only write cases intentionally skipped | 56 passed, no failure/skip |
| QML navigation / compatibility panel | 28 passed, no failure/skip | 28 passed, no failure/skip |

Fresh Python suites: **39 passed** (14 core/release/research, 19 recovery research, 6 tracking portal). Existing native tool/forensic/AVB/PCAP/vault/export and GUI smoke groups are included in full CTest. AVB/PCAP mutation bodies were unchanged; current full runs use ordinary Release instrumentation, not a new sanitizer or leak-check claim.

Disclosed skips per native profile: real-server TLS credentials fixture, separately installed real ClamAV and real mmls were unavailable/unconfigured. Their controlled fixture tests ran. No Windows build, installer/uninstaller, physical Android, real USB-disconnect acceptance or OEM repair passed here.

Raw current XML/text are in [`evidence/modern-android-2026-10-23/`](evidence/modern-android-2026-10-23/). [`modern-ui-ar.png`](evidence/modern-android-2026-10-23/modern-ui-ar.png) and [`modern-ui-en.png`](evidence/modern-android-2026-10-23/modern-ui-en.png) are actual native Qt/Linux window captures at 1180×760, using an injected **synthetic API 37 / 16 KiB observation fixture**. They are not phone or Windows screenshots. The details column scrolls to remaining fields.

Reproduce:

```sh
cmake -S . -B build -G Ninja -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure --output-junit modern-production.xml
# Repeat in a different build directory with -DMR_ENABLE_LAB_OPERATIONS=ON.
python3 -m unittest discover -s tests -v
python3 -m unittest discover -s research/android-recovery -v
python3 -m unittest discover -s server/tracking-portal/tests -v
```

## Remaining gates and continuation

At starting HEAD `9f92983`, native Actions [39894106501](https://github.com/nexvary/NEXVARY-MR/actions/runs/39894106501) and Python runs 39894106379 / 39894100986 were completed failure, with previously inspected zero-step/empty-runner evidence. The cause is unproven. Inspect Actions for this increment's exact SHA; do not treat local success as remote CI success.

Keep PR draft and main stable. Next: resolve actionable CI evidence, obtain authorized current Android regional models/builds and record per-capability physical results; validate Windows 10/11 drivers and package lifecycle. OEM writes, encrypted-data recovery, iOS/non-Android HarmonyOS and model-specific flashing remain unsupported/unapproved. Platform-Tools version/hash and independent boot trust still need acceptance; no current SDK binary is bundled or silently installed. Signing, complete dependency/SBOM/license review, upgrade/rollback, privacy and independent release review remain gates. No RC or Windows download is asserted, and no work is scheduled in the background.
