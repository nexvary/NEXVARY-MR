# Native forensic integration — first migration batch, 2026-10-28

**Partial integration, not completion of the nine-stage program or government certification.** Three selected original functions are now real C++/Qt engines in the actual Government case workflow. Wider upstream audit and migration remain in progress. PR17 stays draft; no main merge, Government branch write, upstream modification/deletion or Guardian change.

## 1. Current source and HEAD

Acceptance source: `1d40b09d21c513b4bd1f4e8c73af342ebe5d20b3` (tree `88e348ddd3442c726ef08a907df64f87d6b8dc34`). PR merge checkout: `4fa473d33a596bb4de2e0db87641346871e176f8`, same tree. Government parent inspected/synchronized: `eb2322140956a55a50ee7494570f35573a1ce0c0`; original integration HEAD `08d8fb6a9a830ee607b8e5fd495593a7c0ee8740`.

The following evidence/state checkpoint HEAD is resolved by the GitHub commit containing this file. Its diff must contain only unshipped state/audit/manifest/evidence files; runtime, tests, workflows, packaging and shipped guides remain at the acceptance source. This is source identity proof, not inheritance of tests for altered executable inputs. `CURRENT_SOURCE.json` records the final acceptance disposition.

## 2. Upstream source SHAs

| Repository | Inspected pinned HEAD |
|---|---|
| NEXVARY-Digital-Forensics-Workbench | `8b35940b1c879e0ce5f5c13e0d6c89aeab023757` |
| NEXVARY-Browser-Forensics-Lab | `d7754cd8f051d285406403465b6d7daa3bc4b060` |
| NEXVARY-DC | `d8e2baf61b43d044b33a1dd76a444d093c80023c` |

Both workflow source payloads were decoded without executing bootstrap code. XZ SHA256 Workbench `9eff37d785b6e44df2e3a611f9d44387b5a78dccd9bf240a4b352edb8b14bb3e`; Browser `62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226`. Upstream CI observed at other commits does not validate these heads or MR's ports.

## 3–4. Modules audited and actually migrated

Detailed source/function/import/hash inventory: [forensic-upstream-source-inventory.json](../../research/forensic-upstream-source-inventory.json). Source→MR overlap→reuse/defer→adapter→permission→test matrix: [audit](../../docs/FORENSIC_ENGINE_INTEGRATION_AUDIT.md). Permission limits: [source permissions](../../docs/FORENSIC_SOURCE_PERMISSIONS.md).

| Selected source module | Existing MR equivalent | Actual new native adapter | Permission and evidence |
|---|---|---|---|
| Workbench `pro_tools_v1221.py:export_validate` | Existing package verification/hash/case/report engines retained | `workbench-export-manifest`: manifest shape, duplicate paths and digest syntax; explicitly **not payload verification** | Selected owner-authorized original function; native positive/negative/hash/limit tests |
| Browser `parsers.py:ChromiumParser.bookmarks`, `core.chrome_time` | Existing artifact/timeline projections reused | `chromium-bookmarks`: folder/title/URL/raw date-added/UTC; **not visits** | Same scoped permission; known timestamp, malformed/duplicate/deep/oversized JSON and actual case/UI tests |
| DC `Partitions.cpp:imageVolumes` | Existing TSK registered-case path remains separate | `dc-image-partitions`: byte-only512-sector MBR/EBR/GPT with CRC/bounds/overlap/cycle checks and backup GPT | Same scoped permission; known images, truncation, CRC corruption, mutation corpus and destructive-ID rejection |

No general original-code LICENSE was found. The explicit project-owner migration authorization permits only the selected original-code integration; it does not invent an MIT/GPL license, waive third-party rights or settle external redistribution. No upstream database, updater, PySide/lz4/TSK binary or destructive Disk Care API is imported. Audited/deferred source includes browser SQLite/profile/session/cookie engines, Workbench analytics/export/provenance overlap and DC filesystem/media-read/copy boundaries. Catalog400 IDs cycling20 operations are not400 independently migrated engines.

Unified descriptors/requests/findings/results/receipts carry case/evidence/digest/version/upstream provenance. Exact3-ID allowlist has no generic command/path/device/write interface. Existing protected evidence copy/hash and actor/case/revision/grant checks precede a bounded QtConcurrent task; callback rechecks cancellation/authority/session/source before the existing signed prepared intent and SQLite case/result/audit transaction. Strict native/Python operation kinds and conservative restart reconciliation prevent duplicate restart acceptance. Deliberate new user analyses remain separate invocations. Findings/timeline validate retained report/source binding; report generation reuses existing case reports. Bilingual Government Analysis controls run real engines and expose visible summaries plus optional details.

## 5–7. Changed files, commits and PR

Full integration file list against synchronized Government source: [integration-files.json](integration-files.json). Main implementation: `ForensicAdapters.*`, `DcImagePartitions.*`, `LawCaseForensicAdapters.cpp`, `LawCaseBridge.*`, `CaseArtifacts.cpp`, `CaseTimeline.cpp`, strict operation verifiers, CMake, Government QML and native/QML/Python tests. The Government-parent merge also preserves its transaction/security/reliability work; it is not falsely counted as newly ported forensic functionality.

Commits: `4de5c4405b09d46a412e2ea3763ff114c8d3b67c` actual engines/Government sync; `b1f31a4e00b2bf5a0b2f5d0e69da524a4f732346` actual-frame result-visibility correction; `ec8ff812beb914cbea71c6ad79cc5cada0befb84` Qt6.8 strict Windows constructor fix; `1d40b09d21c513b4bd1f4e8c73af342ebe5d20b3` explicit pre-dispatch/in-flight expiry test coverage. Final checkpoint commit is evidence/state only.

[PR17](https://github.com/nexvary/NEXVARY-MR/pull/17), [Issue16](https://github.com/nexvary/NEXVARY-MR/issues/16). No automatic merge.

## 8. Tests, failures and skips

[PR run40644129202](https://github.com/nexvary/NEXVARY-MR/actions/runs/40644129202), attempt1: **7/7 jobs successful**, exact source1d40/tree88e348. Push/PR Python and public-history workflows also successful. [Full CI metadata](final-ci.json).

| Profile | Full CTest groups | Native adapters / case transactions | Partition transactions | Qt Quick / DPI125%+150% / GUI repeats |
|---|---|---|---|---|
| Windows Production, Qt6.8.3 MSVC /WX |51/51|15/15;37/37|29 passed,1 real-mmls skip|55;both groups passed;3/3|
| Windows Laboratory, Qt6.8.3 MSVC /WX |51/51|15/15;37/37|29 passed,1 real-mmls skip|55;both groups passed;3/3|
| Linux Production, Qt6.4.2 |51/51|15/15;37/37|30/30, real mmls|55;16/16 each;3/3|
| Linux Laboratory, Qt6.4.2 |51/51|15/15;37/37|30/30, real mmls|55;16/16 each;3/3|

Linux Production **ASan/UBSan20/20**,199.80seconds, default leak checking enabled: new-adapter15/15, partition30/30 and local case transaction37/37 inside the instrumented run. Original sanitizer failure retained separately. Security14/14 each profile; signed native/Python interoperability73/73 each profile; local root Python51/51. Windows policy-audit POSIX symlink subcase and separately installed real-mmls integrations are explicitly skipped, not passes. Production service write fixtures are excluded by design and exercised in Laboratory; separate real TLS/native portal integration succeeded in Linux Production. No test gate was disabled.

Each profile retains its raw XML, native results, full-suite log, Qt/DPI totals and intentional subcase skips in `final-1d40-*` files. CTest's zero skipped groups does not mean zero skipped native subcases. Real-server TLS runs as its separate fixture; profile/platform external-tool exclusions are listed verbatim. Signed-history corpus is internally authored synthetic interoperability, not independent institutional assurance.

Historical failures remain retained: first local48/51 generated empty/corrupt/nonexecutable products; later50/51 extracted ClamAV missing its packaged CA directory (fixed by pointing to original public certs, no signature bypass); b1 Windows both failed Qt6.8 deprecated QDateTime under unchanged /WX; ec full profiles passed but overall PR40640872215 failed ASan partition expiry19/20. Original expiry failure lacked policy diagnostics, so preflight-cost expiry remains a hypothesis. Corrective test preserves1500ms expiry; accepts pre-dispatch refusal only on actual expired policy with healthy audit/no dispatch/result, and separately requires an authorized real worker-ready marker before actual expiry and refusal of all results. No production lifetime/security gate changed. Local LeakSanitizer process inspection is unavailable; local15 logical adapter passes/exit1 and leak-disabled targeted5/5 diagnostic are not full sanitizer passes.

## 9. Windows build artifacts

Build profile `production-read-only`, Qt6.8.3, unsigned, commit `4fa473d33a596bb4de2e0db87641346871e176f8`; PR checkout tree equals the acceptance source.

| Artifact | Link | Verified SHA256 |
|---|---|---|
| Portable provider ZIP | [artifact12798175757](https://github.com/nexvary/NEXVARY-MR/actions/runs/40644129202/artifacts/12798175757) | `0be711da48ec496bc3517f0fdc588a5be5705dac63aaf90487f56c5d1084d3e9` |
| Inner Portable ZIP | same artifact | `26f9e431037ffdcb6a889c2ba6def5b9155dbd72ce23948e12cad8a95e448abd` |
| Portable/installed main EXE | same deployed bytes | `f623145a559fad770383e7589259a8865b26eee01cd9aad357cc513365950e4c` |
| Import worker EXE | same deployed bytes | `79b364222ff62978303d1709e9e5c2f0b39f46375e9011310dd972d158b78d89` |
| Installer EXE | [artifact12798255603](https://github.com/nexvary/NEXVARY-MR/actions/runs/40644129202/artifacts/12798255603) | `5f30ec610c5fda75f9a0cb4f17c16b4973d21781e136b37fb166613eade2aac6` |

All1419 declared portable entries and1418 SPDX file records checked. Provider installer archive SHA256 `78d5180e6d8422340af87067a8cf55e17abdbb7ccd4c4862bf791c02df35f790`. [Downloaded byte/profile receipts](final-windows-artifact-verification.json); [Portable](final-1d40-portable.result.json), [installed](final-1d40-installed.result.json), [install/uninstall](final-1d40-installer-lifecycle.json) receipts. CI install0/run0/uninstall0, installed EXE removed and per-user registration removed; child PATH excludes Qt SDK and font override is cleared. Independent public-policy verification23/23 events for each deployed mode.

Downloaded provider archives are SHA256 checked. Every declared inner portable entry and the exact file set, BUILD_PROFILE, SPDX dependency manifest and main/worker bytes are verified. Installer/portable checksum records are cross-compared. CI install/run/uninstall and public signed-policy exports are independently checked; temporary local keys/anchors are not institutional trust roots. Packages are unsigned evaluation builds.

Physical Windows10 Home19045 existing-host test was repeated on the **final1d40** Portable bytes: main `f623145a559fad770383e7589259a8865b26eee01cd9aad357cc513365950e4c`, worker `79b364222ff62978303d1709e9e5c2f0b39f46375e9011310dd972d158b78d89`, build `4fa473d33a596bb4de2e0db87641346871e176f8`. All1419 entries independently checked on that host; direct native smoke exit0, six default-denied categories, revocation and23 independently verified signed events. Existing MR installation untouched; physical installer not run because an MR registration exists. OS refused the earlier PS1 harness; execution policy unchanged, direct native synthetic smoke used an isolated verifier environment. [Final physical receipt](physical-win10-1d40-result.json), [actual AR frame](physical-win10-1d40-ar.png). This is an existing-machine software-renderer smoke, not clean install, new-engine packaged case exercise or hardware acceptance. Prior ec8ff physical receipt/frame are retained under their original scope.

## 10. Screenshots

Actual current-source Windows Qt AR/EN frames: `final-windows-forensic-ar-150.png`, `final-windows-forensic-en-150.png` ; responsive840/1366, RTL/LTR and100/125/150% assertions run in the suite. Final physical smoke frame and retained ec Windows frames are explicitly source-scoped. The physical smoke frame shows its last synthetic board view; native/QML CI frames demonstrate forensic results. Old hidden-result screenshots are retained to show why the visibility correction was required. Captures use synthetic content, not real cases. Software/offscreen images do not establish flicker-free hardware rendering or long soak stability.

## 11. Security and reliability limitations

- Input8MiB/report64KiB/findings128, bounded traversal and cooperative5-second deadline; no OS hard memory/thread sandbox.512-byte sector images only, not all corrupt filesystems or production-size images.
- SQLite-like/corrupt inputs are rejected by the bookmark adapter; that is **not** SQLite/WAL forensic analysis. No browser passwords/cookie decryption/history/download/session support claimed. No NTFS/exFAT/FAT32/recovery engine migrated; malformed filesystem acceptance is therefore untested/unsupported.
- New engines launch no child process. Existing external workers' detached-child/product process-tree containment remains open. Existing Audit Store signed-disk/in-memory divergence remains open; new fail-closed tests do not establish root-cause repair. Historical Qt SIGSEGV/root cause and hardware/soak acceptance remain open despite current regressions passing.
- No clean physical install/uninstall, Windows11, phone/device, hardware-renderer/flicker/performance or independent agency acceptance. Publisher signing, agency identity/key/time/custody and external license/distribution/comparative review remain open. No government accreditation or superiority claim.

## 12–13. Remaining stages and next executable task

All nine broad manifest stages remain **in-progress**. Selected-module tests/package results must not promote full source audit or all engines to verified. Complete browser SQLite/profile-family review and implement coherent protected main/WAL/SHM capture/hash/sidecar provenance, fail closed on mismatch/incomplete families, read-only analysis on a derived working copy and corrupt/WAL/interruption/cross-case/authority tests before enabling history/download/cookie metadata. Keep original browser files untouched. This is the next concrete engine migration task.

Then audit/split DC read-only NTFS/exFAT/FAT32 analysis from destination-writing recovery, add independent fixtures/bounds/sector support evidence, and port only genuinely new Workbench provenance/export functionality after overlap/license review. Continue exact-source four-profile/sanitizer/Qt/DPI/package acceptance and resolve audit-store/process-tree/Qt root issues without weakening gates. Original upstream projects stay independent.
