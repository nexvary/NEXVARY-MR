# SQLite evidence family: acceptance checkpoint — 2026-10-28

## 1. Branch and exact source

Branch: `integration/forensic-engines-audit`. Tested implementation HEAD `e0ddc8c2ca9f0ec4b4c00f6e3ccec07226127716`; tree `b71f68a08fd26350b64525ea3c883ce72d65dbb8`. PR checkout `1129afad51f085ebf327d11d73ff32b26aa46d28` has the same tree. Government branch remains `eb2322140956a55a50ee7494570f35573a1ce0c0`. Resolve later evidence-only checkpoint HEAD using git rev-parse HEAD; that full tree has not inherited exact-source CI acceptance. Scope proof binds unchanged executable/test/workflow/packaging inputs to the accepted source.

## 2. Pinned upstreams

- Workbench: `8b35940b1c879e0ce5f5c13e0d6c89aeab023757`.
- Browser: `d7754cd8f051d285406403465b6d7daa3bc4b060`.
- Disk Care: `d8e2baf61b43d044b33a1dd76a444d093c80023c`.

Re-inspected Browser `nbf/core.py:CaseManager.snapshot_sqlite_family` from reconstructed pinned archive SHA256 `62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226`. Original sequential copy is not proof of atomic acquisition. Module-specific owner permission is in docs/FORENSIC_SOURCE_PERMISSIONS.md. SQLite WAL validation is an MR extension using https://www.sqlite.org/fileformat2.html#wal_format. No new runtime dependency, evidence database, updater or signature authority.

## 3. Actual delivered behavior

Fourth native adapter `browser-sqlite-family` accepts explicitly bound protected main/WAL/opaque SHM in one authorized case. It verifies member hashes/provenance, headers, frame salts and rolling checksums in both byte orders; constructs the last committed page image, excludes verified uncommitted frames, rejects malformed/truncated/stale tails and missing growth coverage, and emits deterministic bounded family/image hashes and raw member byte references. Limits: aggregate/image8MiB, SHM128KiB,4096frames, report64KiB and existing5second cooperative deadline. Main and auxiliary protected content and authority are rechecked before existing signed retention; retained projections validate case member inventory. Findings1, timeline events0 for this foundation. Bilingual existing Government controls added.

Independent Python SQLite3.53.1 serialize oracle checks exact bytes, not only our own algorithm; reference image SHA256 `c8e31031c10d76d5bd9be04ff46f79db61eb723073ee92e97ebca7a7fedc19e5`. No History/Downloads/session/SQL/B-tree/decryption capability yet. Common acquisition/authenticity remains unproven; SHM is opaque provenance, never a replay authority. Existing workbench-export-manifest content verifier, chromium-bookmarks and dc-image-partitions preserved.

## 4. Modified files

Full implementation inventory: [implementation-files.txt](implementation-files.txt). Main parser: cpp/src/SqliteEvidenceFamily.cpp/.hpp; integrations: ForensicAdapters, LawCaseForensicAdapters/LawCaseBridge, CaseArtifacts/CaseTimeline and CaseOperationIntent; UI: GovernmentAnalysisPage.qml; tests: forensic_adapter_tests, ui_tests, tst_navigation, test_sqlite_family_reference; fixtures: samples/sqlite-family; audit verifier/interoperability and integration gate updated. Documentation, state and evidence are separate checkpoint-only inputs.

## 5. Acceptance results

Exact source PR C++ [40679092338](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092338): **7/7 jobs successful**. Python [40679092358](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092358) and public-history [40679092371](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092371) successful for the same source.

| Scope | Actual results |
|---|---|
| Windows Production / Laboratory | CTest51/51 each, MSVC /WX |
| Linux Production / Laboratory | CTest51/51 each |
| Native adapters, all four profiles |25/25 each |
| Qt Quick, all four profiles |57/57 each |
| Signed interoperability, all four profiles |74/74 each |
| Production ASan/UBSan |20/20 |
| Qt DPI |default100 plus125/150 groups passed; Linux18 subcases per additional scale; three-repeat GUI passed |
| Root Python local |55/55; independent malformed/immutable SQLite oracle included |
| Related local |9/9 case/audit/artifact/timeline/identity/law/UI/DPI groups |

Native tests cover corrupt/huge/truncated evidence, growth/shrink edge cases, checksum orders, uncommitted tail,256 mutations, case isolation, signed retention/restart, post-dispatch auxiliary tampering, cancellation/logout/revocation/audit failure. Scope remains selected fixtures; original crash reconciliation and containment problems are not declared closed. Profile/tool subcase skips are listed in each subcase-skips.txt; zero CTest-group skips does not imply zero subcase exclusions. Text evidence copies normalize CRLF to LF; original provider archive hashes identify original bytes.

## 6. Failures and corrections

- fc553e6 Python40675903819/job122774112836: Windows WinError32 during oracle cleanup. sqlite3 transaction context did not close its connection. contextlib.closing fixes lifetime without suppressing cleanup/assertions. Original first-python-ci-failure.log retained.
- a25a96d C++40676204546/job122775354364: Windows50/51, native24/25. Ciphertext trailer mutation did not establish changed plaintext under DPAPI. Current test flips actual WAL byte70, re-seals via existing vault, proves !verifyEvidence before completion, keeps empty-result/no-new-analysis assertions/restoration, and guards DB cleanup. Original first-windows-native-ci-failure.log retained. Current native logs record Windows trailer decode true and Linux false; content substitution rejected on both. No executable implementation change was required by these fixture corrections.
- Initial local dependency2/101 failure remains unproven; same pinned archive independently default-configured101/101, hosted pinned preparation unchanged. Generated zero-byte object/link failure, UI warning and first unauthenticated fixture failure retained separately. No tests disabled and no association to historical Qt/audit faults asserted.
- Initial installer download ended at25MiB; rejected, resumed with HTTP206 and full33288035-byte provider SHA256 verified before any acceptance.

## 7. Commits

Implementation `fc553e643b7839d272a70b599045c28e20b5bf00`; connection fixture `a25a96d22e6557acb90f275fc4383b34e9e5b89b`; final native content-tamper fixture/source `e0ddc8c2ca9f0ec4b4c00f6e3ccec07226127716`. Later checkpoint changes only evidence/state/acceptance metadata; see checkpoint-scope.json.

## 8. PR / tracking

[PR17](https://github.com/nexvary/NEXVARY-MR/pull/17) remains draft, base Government branch. [Issue16](https://github.com/nexvary/NEXVARY-MR/issues/16) remains open. No merge to main, Government/upstream/Guardian modification or outside messaging.

## 9. Windows artifacts and actual binaries

- [Portable unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092338/artifacts/12812867812): provider SHA256 `910827009e72935d1c55e82a207a537db5d496f515efc176b2ef40ec5056cce7`.
- [Installer unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092338/artifacts/12813435105): provider SHA256 `3b924686f04a3f5a9f59e43d5e2a4f915561ddbb4510ffd128e3cf7aa237edc4`.
- [Windows Production receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092338/artifacts/12813042260).
- [Windows Laboratory receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40679092338/artifacts/12812593935).

Portable inner ZIP SHA256 `69103344b7d0e50ed24378b20f0599e20d66f31d26f715ef632b0ed378bad897`; setup executable `eeaf7086903f85caf82d00c93b80fd911ec415210f84bac5ebb9fe1505662d01`. Main executable `276a8a99be0fdb5e77429a15e1383aa80ba083cab6590668f95e7d87790a6b38`; worker `3368026087442847ab8f476ef9429407c569a5ee37b631fa703295a142e8b37e`. Full1419 SHA entries/exact file set and1418 SPDX records verified, actual AMD64 main/worker and Qt6.8.3 unsigned production/source profile checked. Hosted portable and installed launch0, matching main/worker hashes, install/uninstall0 and executable/registration removal confirmed. Independent policy23events each.

Same provider bytes pass synthetic actual main/worker launch0 on existing physical Windows10 build19045, SDK-free child PATH/software renderer, unchanged executable hashes, six default-denied policy categories and independent23-event signed audit. This is common deployed smoke; new family-specific scenarios run in hosted native/Qt tests. No physical installer/clean Windows11/hardware-renderer/device acceptance claim. Static/hosted/physical JSON and reproducible verification helpers retained here. Artifact expiry is recorded in provider-artifacts.json.

## 10. Open problems and stage status

Audit Store on-disk/in-memory signed-sequence divergence root cause; process grandchildren containment; original Qt SIGSEGV root cause/DPI soak/hardware; agency identity/trusted time/independent examiner/product signing/clean-host acceptance remain open. Passing current synthetic scenarios does not close these. All9 broad stages remain in-progress with measurable criteria; no stage marked verified from this foundation alone. No full recovery, government certification or competitor superiority claim.

## 11. Next implementation

Isolated bounded Chromium History extraction on the verified committed page image, schema/profile/raw timestamp provenance and same-case signed findings/timeline; then Downloads. Review original parser/source permissions and use malformed/large SQLite fixtures. Keep Sessions and common live acquisition deferred. DC next read-only NTFS/exFAT/FAT32 metadata/extents requires explicit allowlist and partial-recovery labels; no destructive APIs.
