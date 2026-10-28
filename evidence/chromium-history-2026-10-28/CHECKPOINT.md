# Reconciled protected Chromium History case integration — 2026-10-28

## 1. Branch / exact source

Branch integration/forensic-engines-audit, draft PR17. Actual parent
5129a15ceb44e4f63a9a8432256cdba5d91614f5, Government unchanged
eb2322140956a55a50ee7494570f35573a1ce0c0. Resolve published HEAD with git rev-parse HEAD.
The first expected-head publication attempt found concurrent5129a15 and did NOT
update/force the branch. Its isolated local commit c67cdc1 was never the branch
HEAD. Reconciled source uses the existing ChromiumHistoryReader, not a duplicate
schema/join parser. Source/test hashes bind local receipts; exact-source hosted CI,
MSVC and newly built Windows binaries/packages remain PENDING until separately
recorded. No prior source/package result is inherited.

## 2. Upstream pins

Workbench8b35940b1c879e0ce5f5c13e0d6c89aeab023757;
Browserd7754cd8f051d285406403465b6d7daa3bc4b060;
DCd8e2baf61b43d044b33a1dd76a444d093c80023c.
Browser pinned reconstructed payload SHA256
62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226;
original nbf/parsers.py:ChromiumParser.history was re-inspected. Source/permission
review: docs/FORENSIC_SOURCE_PERMISSIONS.md; format reference
https://www.sqlite.org/fileformat2.html . No new runtime dependency, upstream store,
updater or signing authority. Original repositories and Guardian unchanged.

## 3. Actual delivered capability

Fifth native allowlisted adapter chromium-history now reuses the concurrent
ChromiumHistoryReader in production and adds existing Government main/WAL/opaque
SHM selection, strict case report/retention policy, member/revision/session/grant
revalidation and existing signed prepared-journal acceptance. Raw exact IDs/times,
URL/title/counts/transition/nullable from-visit and source byte provenance are
retained. Findings/Timeline/Reports use validated case/member/schema/time/extent
provenance. Existing four native adapters preserved. No SQL/live profiles,
decryption/Downloads/Sessions/deleted recovery/new database is enabled.

One declaration/table/join implementation: concurrent reader's explicit column
allowlist and8192ASCII/32column grammar retained, including reviewed sync columns
and AUTOINCREMENT. Its separately tested URLs-only metadata and unmatched-visit
counts remain internal; production case wrapper requires ordinary urls+visits
and refuses orphans. Schema/URL/visit page sets must not alias. Nullable referrer
stays NULL; nullable title displays empty with original bytes; case URL/count/
last-time/transition must be available. Limits:8MiB family/image,4096rows/table,
128findings,64KiB report/record and existing cooperative5s deadline. Valid large or
unsupported real profiles can reject; no partial/truncated success.

Deterministic rowid ordering, family/image/declaration hashes, schema references
and original protected main/WAL id/hash/byte extents include entire leaf cell
(rowid/local payload/first overflow pointer), record payload and overflow data.
Internal navigation/later overflow-pointer bytes are not all enumerated. No
full database integrity/authenticity/common acquisition/profile identity claim.
Exact raw Chromium microseconds retained; UTC display floors to milliseconds.
Recorded visits are not proof of human viewing or external trusted time.

## 4. Modified files

ChromiumHistory.cpp/.hpp case wrapper and retained validator; existing
ChromiumHistoryReader metadata/alias guards, SqliteTableReader visited page set;
ForensicAdapters, LawCaseBridge/LawCaseForensicAdapters, CaseTimeline;
CaseOperationIntent, Python public verifier/interoperability allowlist;
GovernmentAnalysisPage.qml and native/UI/table tests, CMakeLists;
samples/chromium-history and independent Python oracle; research/gate;
permissions/docs/state/evidence. Existing family/pageOrigins implementation is
retained from5129a15. See source-sha256.json and git diff parent..HEAD for full paths.

## 5. Reconciled local validation

- Strict GCC13/Qt6.4.2 desktop/test build passes with -Wall/-Wextra/-Wpedantic/-Werror.
- Table/History29/29: all18 concurrent History scenarios kept plus two integration
executions; prior1200row runtime oracle over four page sizes, overflow/Arabic/BLOBs,
corruption/cycles/resource limits and mutation tests retained. New fixture has
reordered columns/two visits/Arabic title updated in WAL, raw time known answer,
exact original main/WAL byte-slice checks, retained forged time/offset refusal,
missing/schema/type/orphan/129row and100row report-size rejection/128mutations.
- Native27/27: signed History retention, Findings/Timeline, auxiliary plaintext
substitution after dispatch rejected, wrong-case WAL rejected, cancellation,
logout/grant revocation/audit failure, restart without duplicate acceptance.
- Root Python57/57; immutable independent SQLite JOIN/quick_check and exact
2023-11-14T22:13:20.123456+00:00 microsecond oracle; source bytes unchanged.
- Signed interoperability75/75 after current native verifier rebuild.
- Reconciled related CTest8/8 passed (197.75s), including Qt58/58 and125/150percent groups with actual Arabic/English History controls/results. Exact reconciled receipts attached. This is selected local acceptance, not full Windows/Linux52/52 acceptance.
- Reconciled ASan/UBSan29/29 with detect_leaks=0/halt_on_error=1 passes. Local leak
inspection is INCOMPLETE: original detect_leaks=1 run ended after tests because
/proc access is restricted. Hosted workflow's normal leak gate remains enabled.

## 6. Original failures and concurrency

Parent c4cfb41 run40684287557 Windows builds failed, Linux matrix jobs cancelled.
Reviewed Laboratory122803274003 failed C4310/C2220 test fixture constants; original
log retained. Concurrent5129a15 bit-preserving fixture correction is preserved,
/WX and malformed-byte assertions remain enabled.
First local strict indentation/UTC format failures, stale native policy verifier,
bookmarks test layout timing and generated zero-byte sanitizer object/link failures
are retained. Existing actual-viewport artifactClick helper fixes ordered UI
interaction without skipping assertions; default passed before reconciliation and
must pass again after it. Fresh Ninja sanitizer build passed; zero-object cause
unproven. Reconciliation removed an unused helper flagged by -Werror. No disabled
tests or claimed shared root cause with historical product Qt/audit faults.
Pre-reconciliation receipts are explicitly historical in
pre-reconciliation-local-checkpoint.md; concurrent-CHECKPOINT.md and concurrent
source hashes preserve5129a15 review. Neither accepts this reconciled source.

## 7–9. Commit / PR / Windows packages

Resolve implementation source with git rev-parse HEAD. PR17
https://github.com/nexvary/NEXVARY-MR/pull/17 remains draft; issue16 open; no merge
or Government/main/upstream/Guardian write. Exact-source hosted CI and Windows
Portable/Installer executable acceptance are PENDING. No accepted new artifacts
claimed; e0dd artifacts accept e0dd only. Subsequent CI evidence must bind this
published commit and PR checkout tree before acceptance.

## 10–11. Open problems / next phase

All9 broad stages remain in-progress. Audit Store signed disk/memory sequence
divergence, descendant containment, original Qt SIGSEGV root/DPI soak/hardware,
product signing/clean-host/agency identity/trusted-time/independent validation
remain open. Synthetic successes do not close them. Next: exact-source hosted
CI and actual Windows runtime/package verification, then bounded Downloads on
this one protected page path. Sessions/profile and read-only DC filesystem/
partial extents/recovery remain separate reviewed increments; no destructive APIs.
