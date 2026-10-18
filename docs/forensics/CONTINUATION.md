# Forensic Lab continuation checkpoint — 2026-10-18

## Repository state inspected

- Native branch: `dev/cpp-qt-foundation` at `0bfe9b113cfac6c9f50f0e278d6faa8ff6733ffc`.
- Forensic branch: `feature/forensic-lab-foundation`, initial HEAD `b3f3066b66352133bc333aaee6e9285f88733a7b`.
- OEM Python branch: `ef302c9` (PR #2). No blind merge into the Qt runtime.
- `main`: `7446b17`.
- PR #3 remains a draft targeting the native branch.
- The initial forensic HEAD already passed Windows MSVC/Qt, core Linux/Windows, CTest/QML, portable deployment, installer installation and removal: https://github.com/nexvary/NEXVARY-MR/actions/runs/38982198811 . The prompt's missing-build description was stale.

## Implemented in this increment

The existing `RecoveryBridge`/`CaseVault`/native image scanner remain the only case and recovery engines. Added evidence UUIDs, explicit operator/authorization recording, immutable-by-API audit entries linked with SHA256, custody declarations referring to evidence IDs/hashes, embedded-evidence verification, safe bounded plaintext previews, searchable evidence lists, a searchable UTC **case activity** timeline, JSON timeline export, and escaped HTML/PDF case reports. JSON and formatted reports include their own proposed export event; failure to save the audit is explicitly returned even if an output file already exists. Existing files and original sources are never intentionally overwritten by export.

Existing v1 cases open without discarding evidence. Unchained historical audit entries are marked `legacyUnverified`; unknown historical operators remain empty. Assigned evidence IDs persist on the next successful save. Operator changes cannot retrospectively assign a collector to older evidence. Agent import now re-reads its source just like logical-file acquisition.

The native UI now has a sidebar, retained device panel toggle, Back history, licensed unmodified color SVG navigation, dark/light palettes, dedicated evidence and case-timeline tabs, and explicit custody/export confirmations. Packaged SVG runtime dependencies are required and tested. The smoke runner visits all ten pages in both languages and themes, exports synthetic HTML/PDF/timeline files, and captures forensic AR/dark and EN/light views. The CI evidence artifact retains these captures.

## Limits that must remain visible

- The audit chain detects inconsistent edits/reordering but is **not** digitally signed or independently anchored. A Windows account holder capable of rewriting/resealing the entire container can rewrite or truncate its history. Custody declarations do not independently prove a physical transfer.
- Operator identity and authorization are technician-supplied, not authentication or an electronic signature.
- Timeline covers collection/case operations, not SQLite/app/device artifact reconstruction.
- File preview displays at most 32 KiB of text as plain text; it never launches evidence. Binary evidence requires an explicitly authorized export.
- Existing limits remain: 8 MiB per embedded file, 64 MiB evidence per case, 100 evidence entries, 1,000 audit records, bounded 2 GiB raw image scanner and contiguous PNG/JPEG candidates only.
- Reports are unencrypted exports. DPAPI current-account/machine binding is unchanged.
- External forensic candidates are **not integrated**. `UPSTREAM_REVIEW.json` pins inspected upstream revisions and license sources. A custom or mixed license is not treated as MIT/Apache just because it is public.

## Verified by automated fixtures

Run native checks with `cmake -S . -B build -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON`, `cmake --build build --config Release --parallel`, `ctest --test-dir build -C Release --output-on-failure`. The forensic Qt suite exercises tampered/reordered audit entries, legacy migration, collector attribution across cases, consent boundaries, unchanged source bytes, custody persistence, escaped HTML, Arabic PDF generation, timeline export and reopening. QML tests exercise filtering, plaintext/custody confirmations, RTL/English, both themes at minimum size, Back and exit, plus existing repair workflows. Exact Windows/installer results are attached to the tested head's GitHub Actions run, not inferred from local Linux tests.

## Next engineering work

1. Inspect this commit's Windows run; fix any MSVC/Qt 6.8.3/package issue before considering merge.
2. Implement a single external-process adapter under explicit tool selection, with real upstream fixture tests, time/output bounds, cancellation and no claims of OS sandboxing without an actual sandbox.
3. Add read-only SQLite artifact and iOS backup structure parsers with immutable evidence copies, bounded results and encrypted-backup capability detection; no credential bypass.
4. Review OEM safe operations against the existing ToolBridge before porting model-aware catalogs. Chipset/OEM-specific capabilities remain device/protocol dependent.
5. Complete physical-device disconnect/authorization checks and independent clean Windows 10/11 VM acceptance. Existing fake-ADB tests are not a real USB test.

No final release, court accreditation, Android post-reset decryption or Shizuku root access is claimed.
