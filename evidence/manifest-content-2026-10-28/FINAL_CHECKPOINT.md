# Protected Workbench manifest content verification — accepted source checkpoint

1. **Branch and exact source.** `integration/forensic-engines-audit`; executable/test/workflow source `9864f08511c938828c4e3b37f7023af1ec79bed2`, tree `b08d721e5acc5caae99fac02eb73773d60f36aca`. PR checkout `720ccea8b17db1ea062bd14b817279529a069f91` has the identical tree. Government HEAD rechecked unchanged at `eb2322140956a55a50ee7494570f35573a1ce0c0`. Resolve the evidence-only checkpoint HEAD from this file's Git commit; results below belong to the stated acceptance source, not an untested later implementation. See `checkpoint-scope-proof.json` for the checkpoint allowlist.

2. **Pinned upstream sources.**

| Project | Inspected commit |
|---|---|
| Workbench | `8b35940b1c879e0ce5f5c13e0d6c89aeab023757` |
| Browser Lab | `d7754cd8f051d285406403465b6d7daa3bc4b060` |
| Disk Care | `d8e2baf61b43d044b33a1dd76a444d093c80023c` |

Workbench and Browser workflow-selected source archives were reconstructed and SHA-256 checked again. The original Workbench `export_validate` is structural only; content verification is an MR extension of that selected adapter. Follow-on actual Browser/FilesystemRecovery source review is recorded in `NEXT_ENGINE_REVIEW.md`. Scoped owner permission is preserved; no general upstream redistribution license is invented.

3. **Actual functionality.** Existing `workbench-export-manifest` now optionally verifies explicitly bound, already imported same-case protected payloads by SHA-256. It reports missing and mismatched files; rejects duplicate/case-aliased, traversal, absolute, reserved, ambiguous and unsupported paths; bounds entry/binding count, aggregate bytes, report size and cooperative worker lifetime. It never opens a manifest path. A real POSIX symlink with matching content still reports missing without a protected binding; the Windows fixture uses a matching regular file rather than claiming symlink privileges. Exact JSON row references, deterministic reports, per-payload ID/original/working hashes and verification statuses are retained through the existing signed case transaction. Callback revalidates all payloads and authority before retention. Bilingual Government binding/run/result controls are operational. Structural mode remains explicitly distinct.

`chromium-bookmarks` and `dc-image-partitions` are preserved and regression-tested, with no new History/WAL/session/filesystem/recovery claim. There is no new evidence database, updater authority, destructive operation or independent application. Empty evidence and direct live-folder scanning are outside this increment. Portable ASCII path policy and all bounds are documented in `../../docs/MANIFEST_CONTENT_VERIFICATION.md`.

4. **Changed files.** Core changes: `cpp/src/ForensicAdapters.{cpp,hpp}`, `cpp/src/LawCaseForensicAdapters.cpp`, `cpp/src/LawCaseBridge.hpp`, `qml/GovernmentAnalysisPage.qml`; tests in `cpp/tests/forensic_adapter_tests.cpp`, `cpp/tests/ui_tests.cpp`, `qml/tests/tst_navigation.qml` and DPI wiring in `CMakeLists.txt`. Gate criteria/negative tests: `research/forensic-engine-integration.json`, `scripts/check_forensic_engine_integration.py`, `tests/test_forensic_engine_integration_gate.py`. Complete implementation file list: `implementation-changed-files.json`; evidence/state changes are separately scoped.

5. **Exact-source acceptance.** [PR run 40659062610](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062610), attempt 1: **7/7 jobs successful**. The separately observed PR [Python](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062978) and [public-history](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062927) workflows also succeeded.

| Profile | CTest groups | Forensic native cases | Full Qt Quick | DPI 125/150 groups |
|---|---:|---:|---:|---|
| Windows Production, MSVC /WX | 51/51 | 20/20 | 56/56 | both passed |
| Windows Laboratory, MSVC /WX | 51/51 | 20/20 | 56/56 | both passed |
| Linux Production | 51/51 | 20/20 | 56/56 | both passed |
| Linux Laboratory | 51/51 | 20/20 | 56/56 | both passed |

Linux Production ASan/UBSan: **20/20 groups**, 161.77 seconds, unchanged default leak checking and UBSan halt-on-error. Independent signed native/Python interoperability: **73/73 per profile**. GUI smoke repeated three times per profile. Root Python local: **53/53**. Exact local related CTest: **9/9 groups**, including case37/audit15/artifact32/timeline26/identity12/law17, Qt56 and DPI17 each. Linux hosted DPI logs also retain17 cases each; Windows DPI group success and produced frames are verified, without inventing subcase totals absent from its captured stdout. Full downloaded provider ZIP digests and XML/native results are under `hosted/`; `hosted-summary.json` gives parsed observations.

Retained text copies normalize Windows CRLF to LF; provider ZIP digests identify the original archives. No zero-subcase-skips claim: Windows partition integration29+1 real-mmls skip; Linux30/30 with real mmls. Platform/tool/profile exclusions are retained verbatim in each `subcase-skips.txt`. The ordinary cloud TLS subcase is configured/executed separately by Linux Production's actual Gunicorn/Caddy/TLS step. No test was disabled to obtain acceptance.

6. **Failures and cause evidence.** This accepted CI attempt has no failed jobs. Parent c83 CI40658142324 was superseded/cancelled while building and is not current-source acceptance. First local link failure had an observed zero-byte generated `WorkshopBridge.cpp.o`; its metadata/log were preserved and the generated object rebuilt without unrelated source changes. Initial local Qt run lacked Dialogs; the next had missing SVG-plugin warnings; final restored-dependency runs passed. Local sanitized native20 logical cases passed, but LeakSanitizer failed to inspect `/proc/.../task`, yielding CTest exit8: that local run is **not** full sanitizer acceptance, and leak checking was not disabled. Hosted full sanitizer acceptance above is separate. First physical-host wrapper failed before launching the app because converting the Windows environment to a case-sensitive Python dict made `SystemRoot` unavailable; corrected wrapper uses the canonical environment key and a separate extraction directory. Original fixtures/scripts and redacted failure log are retained. None of these observations establishes a common cause with the original audit or Qt failures.

7. **Commits.** `c83f2d7d741e0368cfb9e31f690eeb4df9273d3c` introduces protected payload verification and stage gates; `9864f08511c938828c4e3b37f7023af1ec79bed2` corrects raw entry references and adds independent known-answer/filesystem non-resolution coverage. Following checkpoint edits only evidence/state/acceptance metadata; they do not change executable, workflow, native/QML test or packaging inputs. The changed manifest metadata was revalidated locally, separately from acceptance-source hosted results.

8. **PR/tracking.** [PR17](https://github.com/nexvary/NEXVARY-MR/pull/17) remains draft against Government; [issue16](https://github.com/nexvary/NEXVARY-MR/issues/16) remains open. No automatic main merge, Government branch write or upstream/Guardian modification occurred.

9. **Windows deliverables — actual bytes verified.**

- [Portable artifact12803802016](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062610/artifacts/12803802016).
- [Installer artifact12803737190](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062610/artifacts/12803737190).
- [Windows Production validation12804850640](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062610/artifacts/12804850640).
- [Windows Laboratory validation12803648562](https://github.com/nexvary/NEXVARY-MR/actions/runs/40659062610/artifacts/12803648562).

Downloaded provider archive hashes matched GitHub metadata. All1419 inner portable SHA-256 entries/exact file set and1418 SPDX records matched. Main/worker AMD64 PE headers, Qt6.8.3, source-equivalent PR commit and production-read-only/unsigned profile verified. Main SHA-256 `e6d92fadda8ddce9de9b6631fccf415dd587b634bf56c63824d8e3eb64ebce06`; worker `a5f3cc324ed1e5a0b670c25f3d00522ffaffe2a8bb308ce1f2652c6002b84e2b`; installer EXE `2df5b0d517273ece92c521f6403908946522562e5142f86caebd1eeb2a94d5a8`. Portable inner ZIP hash is also cross-checked against the installer's checksum file. Hosted actual portable and installed EXE/worker bytes agree; both launch and independent audit23 verification succeed; install/uninstall exit0 and final registration/executable removal are verified. Receipts and verification scripts accompany this report. Provider artifacts currently expire2027-01-26.

The same final Portable provider bytes also passed independent direct main+worker smoke on an existing Windows10 build19045 host: exit0, all1419 files/SPDX checked, SDK excluded from child PATH, software renderer, six denied policy categories/revocation and independently verified public audit23 events. No installer was executed on that existing host. `physical-result.json` and `physical-independent-audit.json` retain the sanitized receipt. This does not exercise a real phone, clean Windows installation, Windows11 or a hardware renderer. Builds remain unsigned evaluation artifacts.

10. **Open issues and stage status.** Original Audit Store signed-disk/in-memory divergence, descendant process containment and Qt SIGSEGV root cause/long soak/hardware acceptance remain **OPEN**, despite current passing regression runs. All nine broad integration stages remain `in-progress`, each with measurable criteria; the gate rejects `verified` without commit/test/receipt evidence for every criterion. This slice does not close broader Workbench review, Browser SQLite/session/profile migration, DC read-only filesystem/deleted metadata/extents/recovery, agency identity/time/custody/signing or independent institutional/comparative acceptance. No accreditation or superiority claim is made.

11. **Next implementation.** Build protected SQLite family ingestion through existing case evidence/import machinery: explicitly bound main/WAL/SHM members, exact hashes/family provenance, interruption/corruption/size/authority/cancellation/crash tests. Then port a bounded original Chromium History slice, preserving raw timestamps/row references and distinguishing urls-only fallback metadata from actual visit records. Disk Care needs a read-only metadata/extent interface separated from `Output::save` before NTFS/exFAT/FAT32 migration; no destructive API is admitted.
