# Protected Chromium History integration — 2026-10-28

## Branch / exact source

Base `c4cfb41737ac4e31b06007b77d23b4970933eba6`, integration/forensic-engines-audit,
draft PR17; Government remains eb2322140956a55a50ee7494570f35573a1ce0c0.
Resolve this implementation commit with git rev-parse HEAD. source-sha256.json
binds changed implementation/test/build inputs to local observations. Exact-source
hosted/MSVC/full CI and newly built Windows packages are PENDING. Parent e0dd/c4
results and old artifact bytes are not acceptance for this code.

## Upstream source review / actual work

- Workbench8b35940b1c879e0ce5f5c13e0d6c89aeab023757, Browser
d7754cd8f051d285406403465b6d7daa3bc4b060, DCd8e2baf61b43d044b33a1dd76a444d093c80023c.
- Browser archive reconstructed from previously pinned workflow payload; SHA256
62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226.
Re-inspected original nbf/parsers.py:ChromiumParser.history: selected urls/visits
join, fields and chrome_time are adapted. Module/permission/dependency review in
docs/FORENSIC_SOURCE_PERMISSIONS.md. Original CaseManager/_connect_ro, urls-only
fallback and derived URL carving are not imported. SQLite format interpretation
is an MR extension: https://www.sqlite.org/fileformat2.html .
- Fifth native adapter chromium-history is ENABLED in existing Government workflow:
protected main/WAL/opaque SHM, bounded declaration/table parsing and join,
raw exact times/counts/transitions/from-visit; deterministic rowid order;
original protected main/WAL id/hash/byte references for schema, URL and visit
record prefixes/payload extents. No SQL connection/live profile/new database,
updater/signature authority/decryption/deleted recovery is added. Existing four
adapters preserved. Generic table reader now compiled into mr_desktop.
- Existing completion boundary rechecks case/session/grant/revision/main and all
auxiliary bytes, then existing signed prepared-journal acceptance. Retained
schema/member/time/count/extent validation precedes Findings/Timeline/Reports.
History timeline labels source-recorded visits, not proof of human viewing.
Schema/pages disjoint across tables. All9 broad stages remain in-progress.
- Limits/support/rejections in docs/CHROMIUM_HISTORY.md:8MiB family/image,
4096rows/table,128findings,64KiB report,64KiB record,32levels, bounded schema
grammar and existing5s deadline. No silent truncation; large/unsupported real
profiles may reject. No universal Chromium/profile identity/integrity claim.

## Changed files

ChromiumHistory.cpp/.hpp; SqliteTableReader.cpp/.hpp (serial types, visited pages,
rowid prefix extents); SqliteEvidenceFamily.cpp/.hpp (original page source map);
ForensicAdapters, LawCaseBridge/LawCaseForensicAdapters, CaseTimeline;
CaseOperationIntent and Python policy verifier/interoperability allowlist;
GovernmentAnalysisPage.qml and existing UI/native/table tests; CMakeLists;
samples/chromium-history and independent Python fixture test;
research/integration gate, scoped permissions, documentation/state/evidence.
Full changed-input hashes are in source-sha256.json; git diff base..HEAD gives
complete paths including evidence. No Guardian/main/Government/upstream writes.

## Local tests and limits of evidence

- Strict GCC13/Qt6.4.2 C++20 -Wall/-Wextra/-Wpedantic/-Werror desktop build passes.
- Native27/27: same-case signed History retention, exact fixture values,
original bytes, post-dispatch WAL plaintext substitution (re-sealed and rejected),
cross-case WAL refusal, cancellation, logout/grant revocation/audit failure,
retained forged offsets/time/provenance refusal and restart without new acceptance.
- Table/History11/11 (includes init/cleanup and4page-size data cases): independent
SQLite fixture reordered columns, two visits and Arabic title changed in committed
WAL, raw exact microseconds, source byte slices, deterministic output; missing
schema/columns/orphan/type errors,129visit bound,100visit output bound,
unsupported declarations and128History mutations. Existing1200runtime-row oracle,
512reader mutations, overflow/cycles/limits preserved.
- Root Python57/57, including immutable independent SQL JOIN/quick_check and exact
microsecond known answer2023-11-14T22:13:20.123456+00:00. Read bytes unchanged.
- Signed interoperability75/75 after rebuilding native verifier with new allowlist.
- Related CTest8/8 passed: table/native/artifacts/timeline/signed interoperability/Qt/default and125/150percent groups (200.97s). Qt default58/58 passed; new History Arabic/English actual control/retained result captures and scaled groups passed. Standalone CTest5/5 passed. These are selected local groups, not full52/MSVC acceptance.
- Fresh Ninja ASan/UBSan11/11 with detect_leaks=0, halt_on_error=1: exit0. The
initial detect_leaks=1 completed tests but exited1 because LSan could not read
/proc/122/task under this environment. Original diagnostic retained. Local leak
acceptance remains INCOMPLETE; hosted workflow's normal leak behavior is unchanged.
No disabled test or source warning suppression.

## Original failures / corrections

- Parent c4cfb41 run40684287557: both Windows build jobs FAILED; Linux jobs CANCELLED
by matrix, core/tracking passed. Reviewed Laboratory job122803274003 failed
C4310/C2220 under MSVC /WX at malformed test char(255) constants. Original full
job log retained. Constants now char(-1), the same intended0xff byte on supported
signed-char builds, in range; assertions and /WX remain enabled. Parent full CI
never accepted.
- First local strict build rejected misleading indentation in new retained URL
consistency check; put insertion on its own line. First UTC comparison exposed
+00:00 vs required Z display; normalized with toUTC while retaining raw exact time.
Original logs retained.
- First interoperability invocation used stale pre-History native verifier; after
explicitly building mr_policy_audit_tests with current header,75/75 passed.
- Existing bookmarks UI test clicked after one layout/reveal pass following new
History selection. Reproduced in isolated ordered pair; switched it to existing
artifactClick helper (render/reveal twice and assert actual viewport before mouse
click), preserving real interaction/assertions. Full default suite then58/58.
- Initial Makefile sanitizer link saw generated zero-byte SqliteEvidenceFamily
object; preserved metadata/original logs. Fresh Ninja object/link succeeded.
Cause unproven; unrelated to product parser/audit/Qt historical root causes.

## Commits / PR / Windows artifacts

Implementation source: resolve git rev-parse HEAD; later evidence-only commits
will explicitly distinguish the tested source. PR https://github.com/nexvary/NEXVARY-MR/pull/17
remains draft, issue16 open, no merge. Current increment has NO accepted new
Windows artifacts yet. Existing e0dd artifacts validate e0dd only. Hosted exact-source
CI/package checks must be recorded before declaring this increment accepted.

## Open problems and next stage

Audit Store disk/memory signed-sequence divergence, descendant process containment,
historical Qt SIGSEGV root/DPI soak/hardware, product signing/clean-host/agency
identity/trusted-time/independent forensic validation remain open. Current synthetic
regressions do not resolve them. All9 stages remain in-progress. Next: fix/verify
exact-source CI and actual Windows binaries/packages, then bounded Downloads on
the same protected page path; Sessions/profile and DC NTFS/exFAT/FAT32 metadata,
partial extents/recovery remain separate reviewed increments. No destructive APIs.
