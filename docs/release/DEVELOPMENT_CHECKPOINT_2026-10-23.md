# Integration checkpoint — 2026-10-23

This is an engineering test checkpoint, **not a Release Candidate or commercial release**.

Latest continuation: [Modern Android observation increment](MODERN_ANDROID_INCREMENT_2026-10-23.md), built on verified HEAD `9f9298351c12c4107d05ca2b85a9aa2ff047f346`. The commit containing that increment is the source checkpoint; PR #8 records the exact resulting SHA and Actions links. Android/model acceptance remains pending; no stage 37–42 or 60 promotion is implied.

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

Current next step: inspect Actions for the modern-Android commit, then collect exact regional model/build/transport and Windows driver evidence as described in the modern-Android increment. Generic API recognition is not phone approval. No background execution is scheduled.

## Execution-first continuation: owned scrcpy screen view

Starting HEAD `c674f0b38802c9cace2dc73694f8c1062815f9dc`; the commit containing this entry is the scrcpy source checkpoint (exact SHA in PR #8). `MirrorSession` owns version preflight, ADB discovery/identity, viewer lifecycle, 2-second connection monitoring, 3-second check deadlines and terminate/kill shutdown. Laboratory-only, exact reviewed scrcpy 5.0.1 CLI, pinned ADB/serial, explicit owner/component consent; no control/audio/clipboard sync, no detached viewer or target fallback. Production rejects before any child. Native AR/EN consent/start/stop/log controls are reachable under Devices. Process startup is not proof of frames, phone cleanup or hardware acceptance.

Final Linux Release warnings-as-errors CTest: production **23/23 (86.42 s)**; Laboratory **23/23 (105.48 s)**. Mirror lifecycle: Laboratory **21 passed**, production **4 passed / 17 intentional Laboratory-only skips**. QML UI **29 passed per profile**. The initial UI case exposed a dialog closing-animation timing issue; the test now waits for the modal to become invisible/rendering before clicking navigation. No test was disabled. Current real TLS/ClamAV/mmls integrations remain disclosed skips; no new Python/sanitizer or physical/Windows acceptance claim. Raw XML/text: `evidence/scrcpy-2026-10-23/`.

GitHub access investigation: authenticated connector login `nexvary`, repository permission `admin`. Git/Qt/CMake/Python are available on this Linux host; no git remote, gh CLI, Codex CLI or GH/GITHUB token is configured. Remote Desktop Commander returned **no connected devices**. The cloud browser is FG Machines and repository/run pages return 404 for that session. Connector action-settings and check-annotation endpoints are outside its allowed resource surface; job logs return 404 BlobNotFound. YAML parses; hosted runner labels/matrix and job conditions are present; zero-step failures do not identify the underlying cause. This is an access-to-diagnostics blocker, not proof of billing, YAML failure or absent owner rights. No credential/account/security setting was changed. Obtain the run's annotation through an authenticated NEXVARY-capable diagnostic surface when available; do not infer a cause or stop independent development.

Next executable slice: immutable per-order barcode lookup and printable intake/device card, with database migration/restore and native UI tests. Keep main/RC blocked pending exact-head remote CI and physical/Windows release gates. No background work is scheduled.

## Execution-first continuation: workshop barcode and intake card

Starting HEAD `aa8602ce4c589f06581718a77e4cfff518234c57`; the containing commit is the barcode checkpoint (exact SHA/CI links in PR #8). Workshop schema 4 migrates validated v1–v3 stores transactionally and preserves label mapping through backup/restore. Each order receives one immutable local Code 39 identifier; the same device's later order has a different label. Native lookup is exact and bounded. The Workshop UI issues/displays the card, accepts scanner/keyboard input, shows scoped repair history, and exports an explicitly approved customer/device receipt PDF without overwriting files. Printing is through a PDF reader; hardware acceptance remains pending. CSV now appends the barcode column.

The Code 39 encoding table comes from pinned ZXing-C++ v2.3.0 under Apache-2.0; full license/attribution ship through the existing packaging license directory. No ZXing/ZBar/Poppler runtime is bundled. Independent ZBar decoded the native label, PDF rasterized by Poppler, and both actual AR/EN UI captures. Synthetic customer data only.

Linux Release warnings-as-errors: production **24/24 (90.25 s)**; Laboratory **24/24 (125.90 s full recheck)**; barcode **13 passed, zero skips per profile**; QML **30 passed per profile**. The first Laboratory run was 23/24 due to a GUI smoke timeout at the unchanged 30-second limit; standalone recheck passed (17.41 s) and the full recheck passed (smoke 15.03 s). Cause remains unproven; no threshold or assertion was relaxed. Raw initial and successful outputs are retained under `evidence/barcode-2026-10-23/`. Qt UI teardown warns of one item still being created; review remains pending. Existing real TLS/ClamAV/mmls integrations are disclosed skips. No new Python/sanitizer, phone, Windows, scanner or printer claim.

On parent aa8602, Actions native run 39899472267 failed before any step/runner allocation, matching the earlier 39896846776 symptom. Authenticated connector permissions are admin; annotation text/settings endpoints are unavailable through the connector and job logs return BlobNotFound. No root-cause/billing/YAML attribution is justified. Remote Desktop Commander returned no connected devices, so Windows/phone acceptance remains blocked. Main and RC remain unchanged/blocked.

Next executable slice: bounded read-only firmware package structure/metadata inspection, using synthetic fixtures and the existing SHA-256/AVB bridges. Do not mark stages 37–42, 47–48 or 60 complete. Inspect the exact containing commit's Actions through PR #8 before further changes; if runner startup remains externally blocked, continue independent code/tests. No background execution is scheduled.

## Public repository continuation: actual CI cause and publication guard

Verified Public repository and connector `nexvary` with admin permission at `f7576d9b0e712408ba341d79d5b18b7b6bcd8c06`. The public summary and anonymous annotation endpoint for original run 39896846776 now expose GitHub's account billing/spending-limit startup rejection. It is no longer an unproven cause. The message does not distinguish failed payment from spending limit; no account/billing change was made. Retrying failed jobs of current run 39901371652 after the visibility change allocated runners: core/Linux/portal jobs pass; Windows Qt compiles and exposes a report-scroll UI assertion that is being fixed separately. No desktop device is connected.

Added `scripts/audit_public_history.py`, six synthetic regression cases, and the `Public history safety` workflow with a pinned/hash-verified Gitleaks 8.30.1 runtime. All branches and PR 1–8 head/merge refs were fetched: 141 reachable commits, 750 unique blobs. Gitleaks scanned 132 non-merge changesets plus current tree; zero findings. The independent guard enumerates every commit tree (including alternate names for reused blobs), scans each reachable blob for operational DB/backup/capture/private-key content, and fails closed on external LFS/submodule or oversized data requiring review. Deleting a file from HEAD cannot hide its history. No customer database, capture/backup, keystore or private key was flagged; this is heuristic evidence, not proof of no possible PII/unknown secrets. Unreachable/deleted remote refs and external artifacts/issues are outside scope. Evidence: `evidence/public-audit-2026-10-23/`.

Tests: six new guard cases; full Python `tests` 20/20 and recovery research 19/19. No native code changed in this guard commit. Next: finish the explicit Qt service-scroll extent fix against both Windows profiles, retain original assertion, then continue bounded firmware inspection. Keep main/RC blocked until exact-head quality gates pass.

## Windows report-scroll regression fix

Parent `42f428222deefe2e26bb696108ac10db42f62453`. Original hosted Windows Qt 6.8.3 run 39901371652 attempt 2 compiled both profiles; each passed 23/24 CTest groups and failed only `test_serviceMinimumSizeOutputAndHistory` because the export report control was outside the scrolled viewport. Bind the service details ScrollView's content height explicitly to its ColumnLayout's implicit height and give the content that height, avoiding dependence on implicit extent inference. The original reachability assertion remains, with geometry diagnostics and an additional entire-control visibility assertion. Native AR/EN minimum-size runtime captures are now included automatically in CI's `smoke*.png` artifacts.

Local Linux GCC 13 / Qt 6.4 Release warnings-as-errors GUI build passes. UI suite **30 passed**, zero failures/skips, including added complete-control bounds across AR/EN and both themes (12.57 s). GUI smoke passed with the same application layout (14.38 s). Raw local UI evidence: `evidence/service-scroll-2026-10-23/`. Hosted Windows verification and packaging must pass on the containing commit; no Windows package/RC claim until then. No test disabled or timeout increased. Next: inspect the new native and public-history workflows, fix actionable failures, then continue the read-only firmware package inspector. Main/physical acceptance remains gated.
