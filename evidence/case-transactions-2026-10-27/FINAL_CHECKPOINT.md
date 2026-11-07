# Verified source and separate Push / PR acceptance — 2026-10-27

Executable/test/workflow/packaging/shipped-guide source:
`7844f01bd1427a4f37c702dfc63c50ea74247f94`.
Actual PR checkout: `3efe5f9f48c0581bae4d89a43c209920345e13e4`.
Both trees: `e00050af34e2a3d71566d76660e80cf89eeaf1b6`.
The following checkpoint commit changes state, unshipped comparison documentation
and synthetic evidence only. Resolve current HEAD with `git rev-parse HEAD`.

## Implemented scope

Authenticated local PCAP/EXIF case analyses now use a signed prepared intent in
the existing journal; actual case/evidence/source/revision bindings; a single
SQLite case/result/audit transaction; conservative crash reconciliation; immutable,
idempotent terminal decisions; and fail-closed post-replacement journal readback.
No external action is replayed. Unknown outcomes block the case workspace for review.
Existing import/parser/QML/layout engines and original negative tests were retained.

This is the first local case adapter. The older RecoveryBridge partition-result
store, scanner/update and other import/export paths are **not** declared migrated.
No comparative superiority, institutional approval or universal crash-free claim.

## Exact-source hosted verification

| Event | Run | Outcome |
|---|---|---|
| Push | https://github.com/nexvary/NEXVARY-MR/actions/runs/40569670742 | 7/7 jobs, first attempt, success |
| PR | https://github.com/nexvary/NEXVARY-MR/actions/runs/40569681905 | 7/7 jobs, first attempt, success |

Windows/Linux Production/Laboratory: 49/49 CTest groups per profile/event.
Production Linux ASan/UBSan: 18/18 in both events, required leak checks retained.
Qt Quick: 52; each DPI group: 13. Strict GUI smoke: three sequential passes per
profile/event. New native transaction suite: 16 cases, including seven abrupt
interruption points and deterministic stale readback refusal. Independent native /
Python signed-history corpus: 51. Root Python: 42. Platform/profile-specific
subcase exclusions remain visible in raw reports (including the POSIX symlink
subcase on Windows); CTest-group success is not a zero-skips claim.

## Original local failures remain retained

Initial full local regression: 43/49. Generated zero-byte/non-executable build
artifacts and missing new policy wiring in the native UI fixture were corrected,
without changing QML layout/assertions or weakening tests. Fixture-corrected
observation: 48/49, with the original audit export failure still reproduced.

Original signed on-disk prefix 18 vs in-memory 22 was independently verified;
a second retained observation had 34 vs 41. The storage/rollback mechanism is
**unresolved**. Readback equality is a tested additional acceptance gate for an
immediate divergent replacement, not proof of fixing a later rollback or power loss.
Private originals were preserved separately (40 synthetic files); only sanitized
observations/digests, public CI receipts and synthetic frames are committed.
Private archive SHA256: `2c83d50fa40f744242fb06a9a281e54565ca3eaeaf49e7059c65c46295ec63f3`.

Final-source local full run remained 48/49 because earlier failed synthetic data
retained a nonterminal request. Its actual refusal was "Close the existing request
for this category before requesting again." No product request was silently
closed. After preserving the failed fixture, an isolated fresh smoke passed and
three sequential CTest smokes passed (60.94s). The failed full run is not relabelled
49/49; hosted first-attempt full runs independently passed 49/49.

## Downloaded Windows PR archives and byte verification

Verified provider archive digests, all 1,419 portable manifest file hashes and
complete file set; actual portable/installed EXE and import-worker receipts match
those bytes. Production-read-only, Qt 6.8.3, unsigned. Shipped transaction/policy /
result guides and independent Python verifier match source after checkout line
ending normalization. Installed-in-place launch, install/uninstall and registry /
EXE cleanup passed. These are first-party hosted Windows receipts and independently
read downloaded bytes, not physical clean Windows 10/11 field acceptance.

- Main EXE: `a1047f8d5399ce3eab2e1bc86a68ca0fb385ea59f1aaa7a0898c0d50f673b675`.
- Worker: `3cafdd977ac143c4872c2beaeacce0748a55aaba767651ee15ce1b0ec7f4d74c`.
- Portable inner ZIP: `e0753dee8293a1193cc63fbb250ac16b1579d8c0f07705097aa1ac119d1a62d1`.
- Installer EXE: `ec428a0fe05a026924264fc3512774924bd8d7ab4208d756d081caf467daa16c`.
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40569681905/artifacts/12764269447
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40569681905/artifacts/12765890247
- Raw deployment evidence: https://github.com/nexvary/NEXVARY-MR/actions/runs/40569681905/artifacts/12766010091

These are the downloaded/byte-verified **PR** packages (tree-equivalent to source).
The separate Push also passed; its package hashes are not substituted for these.
Genuine Linux and Windows AR/EN frames were inspected; existing Arabic mixed-label
bidi and hardware-renderer observations are not declared fixed.

## Remaining gates and next executable work

First: migrate the RecoveryBridge partition result path through real institutional
case/evidence ownership and this intent/reconciliation protocol. Its optional text
case reference remains insufficient. Then product-owned descendant containment /
revocation and independent offline egress validation; continue original audit
snapshot-lag diagnosis with preserved inputs and precise failure stage evidence.
GUI soak and measured performance/size/latency, independent forensic reference
validation and comparative benchmarks remain pending.

Clean physical Windows 10/11, owned phones, hardware renderer, trusted code signing,
institutional identity/key/time custody, independent security review and institutional
acceptance remain external unpassed gates. No main merge, branch deletion or
Guardian modification. PR15 remains open/draft.
