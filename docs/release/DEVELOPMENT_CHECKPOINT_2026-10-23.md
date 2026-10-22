# Integration checkpoint — 2026-10-23

This is an engineering test checkpoint, **not a Release Candidate or commercial release**.

## Restored state and ancestry

- Starting integration HEAD: `026cc5d2b0d2140cff3810d0b6e56ffa0a4bb94f`.
- `ServiceOperationPolicy.hpp` existed at that HEAD, but had no execution call sites; it did not protect device commands.
- PR #8 targets `feature/device-service-engine`, not main.
- Existing stack: PR #3 forensic -> PR #4 services -> PR #5 structured diagnostics -> PR #6 Windows USB -> PR #7 metallic UI. PR #8 branched from services before #5–#7.
- Reviewed and applied the #5–#7 patches in order without patch conflicts. They are integrated into #8; no independent PR was merged to main.
- Baseline Actions runs `39857960413` / `39857960417` failed with empty job step lists. The job log endpoint returned BlobNotFound. The root cause is unproven; do not attribute it to code or billing without evidence.

## Implemented changes

- Compile-time, default-deny production profile. Only `probe` and official unlock-eligibility **reading** are admitted by the service operation policy. Generic, unknown and account/lock-bypass actions remain denied in both profiles.
- Native checks at availability, public service entry, queued step, dispatch, adapter command construction and exact argument-plan validation before the QProcess start boundary. There is no QML/environment/config switch to enable writes.
- Separate `MR_ENABLE_LAB_OPERATIONS=ON` build with a differently named executable and default data directory. Existing ownership declarations, serial challenges, fresh capability checks and destructive confirmations remain mandatory.
- scrcpy launch disabled in both profiles until explicit ownership-consent and controlled process lifecycle are implemented and validated. Its server upload must not be treated as a passive read.
- Duplicate serials are displayed as ambiguous; normal property/log reads and service execution refuse them. Cross-transport serial ambiguity also fails closed.
- Structured battery/storage/partition observations and bounded Windows USB serial/tree correlation integrated. Correlation is not proof of an accepted ADB/Fastboot driver binding.
- APK/ZIP structural preflight checks EOCD, directory bounds, local headers, names, methods and APK manifest presence. It rejects truncated/unsupported containers before discovery/write. It **does not** verify payload CRC, APK signing, ZIP signing, firmware/model compatibility or recovery acceptance. These remain required for model acceptance.
- Service report schema **3** exports an explicit technical projection. Raw command output, raw observations, device serials, USB instance IDs and local paths are excluded. Local session history still preserves the reviewed target and result. Reports remain plaintext and require consent; model/build metadata and file hashes may still be identifying.
- Silver card borders, glossy black background, colored attributed SVG assets, neon accents, AR RTL/EN LTR and a visible ordered workflow. Production-unavailable operations are labelled and disabled.
- Windows packaging verifies the binary's compiled profile. It adds unsigned build metadata and an SPDX file inventory. `NOASSERTION` entries and dependency/license coverage still require review; this is not a completed transitive SBOM audit.
- CI tests production and Laboratory profiles independently on Linux and Windows. Production packaging now requires successful CTest and USB-inventory steps, rather than merely a successful compilation.
- `scripts/release_gate.py` refuses RC approval without evidence for the exact commit, independent Android and Windows acceptance, signing, SBOM review, upgrade/rollback, privacy review and reviewer sign-off. No automatic release/publish path was added.

## Stage status — no unverified stage marked complete

| Stages | Current evidence / implementation | Remaining gate |
| --- | --- | --- |
| 08–12 | Actual HEAD/stack inventory, C++ deny policy, separate build profile and identity | Exact-head remote CI and independent review before integration promotion |
| 13–18 | Controlled child-process cases: target ambiguity, disconnect/identity mismatch, cancellation, timeout, tool failure, bounded output, strict command plans, production denial before any child process | Real USB timing/disconnect acceptance |
| 19–24 | Structured observations, malformed-input cases, JSON diagnostics, USB correlation and AR/EN UI | Physical battery/storage/partition comparison and Windows driver binding |
| 25–30 | Existing workshop/customer/inventory/custody/export tests re-executed; service export privacy hardened | Customer acceptance and broader personal-data review |
| 31–35 | Python portal tests plus real Qt -> verified Caddy TLS -> Gunicorn publish/status/revocation and log-redaction integration; backup/restore and rotation fixtures | Production deployment/operator review |
| 36 | Server installer syntax, bundle creation and dry-run verified | Disposable systemd VM install/upgrade/restore test unavailable locally |
| 37–42 | Test instructions and evidence requirements retained | **Blocked: no physical Android acceptance performed** |
| 43–46 | Separate Laboratory build, positive synthetic command fixtures, corrupt-package and identity/consent rejection | **Synthetic tests only** |
| 47–48 | Native safeguards retained; irreversible writes never described as rollback-capable | **Blocked: exact-model authorized physical write/postcondition tests** |
| 49–51 | Qt rendering, navigation/back/exit, AR/EN, minimum window and light/dark automated cases; actual runtime screenshots | Human visual/accessibility review on Windows |
| 52–53 | Windows profile build/package/test jobs configured | **Blocked: current Windows CI plus clean Windows 10/11 acceptance** |
| 54 | Production profile check and unsigned packaging pipeline updated | Current successful installer/portable/uninstall artifacts required |
| 55 | No signing key created or claimed | **Blocked: Authenticode signing and independent signature verification** |
| 56 | SPDX deployed-file inventory generator added | Windows execution, dependency/license completeness and SBOM review |
| 57 | Existing backup/restore mechanisms retained | **Blocked: application upgrade/rollback acceptance** |
| 58 | Local native/Python/TLS regression evidence recorded alongside this checkpoint | Exact-head GitHub Actions still required |
| 59–60 | Fail-closed RC evidence validator and tests implemented | **Blocked: reviewer approval and all outstanding release gates** |

## Resume commands

Production:

```sh
cmake -S . -B build -G Ninja -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure --output-junit production-results.xml
```

Laboratory (separate directory; never package as production):

```sh
cmake -S . -B build-lab -G Ninja -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Release -DMR_ENABLE_LAB_OPERATIONS=ON
cmake --build build-lab --parallel 2
ctest --test-dir build-lab --output-on-failure --output-junit laboratory-results.xml
```

Next: inspect Actions on the committed integration SHA, fix any actionable code/build failures, then collect clean Windows and authorized Android evidence. Do not merge to main or label stage 60 complete until those gates are satisfied. Current success on Linux is not a Windows or phone acceptance result.
