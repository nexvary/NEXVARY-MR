# Protected Chromium History: verified source checkpoint — 2026-10-28

## 1. Branch / HEAD / source identity

Branch `integration/forensic-engines-audit`. Tested implementation `22cad4191c9b1101da5c36ac154bcdc6db720bb6`,
tree `8f96a2f559d1a9cc09f9efca26ebdc8a30559f34`. PR checkout `b1f4e8c546cd7e0fd49b6491a95dd96b43a78f7d` has the IDENTICAL tree.
Government remains `eb2322140956a55a50ee7494570f35573a1ce0c0`.
Resolve subsequent evidence-only checkpoint HEAD using git rev-parse HEAD;
its full tree differs and does not inherit exact-tree CI acceptance. checkpoint-scope.json
restricts changes to unshipped state/research/evidence, leaving executable/test/
workflow/packaging/shipped-guide inputs identical to accepted source.

## 2. Pinned original sources

| Source | Pinned commit |
|---|---|
| Workbench |8b35940b1c879e0ce5f5c13e0d6c89aeab023757|
| Browser Lab |d7754cd8f051d285406403465b6d7daa3bc4b060|
| Disk Care |d8e2baf61b43d044b33a1dd76a444d093c80023c|

Browser archive SHA25662b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226;
actual nbf/parsers.py:ChromiumParser.history reviewed. Scoped owner permission in
docs/FORENSIC_SOURCE_PERMISSIONS.md. SQLite format reference:
https://www.sqlite.org/fileformat2.html . No broad upstream license assertion,
third-party runtime import, CaseManager store, updater or signing authority.

## 3. Delivered functionality

Fifth native allowlisted adapter **chromium-history** is now in production
Government case workflow, reusing concurrent5129a15's existing ChromiumHistoryReader.
One schema/table/join implementation; original18 History scenarios retained.
Actual column ordering, selected visits.url=urls.id join, exact raw IDs/time/counts/
transition/nullable from-visit and original main/WAL/schema/data byte references.
Protected main/WAL/opaque SHM selection, completion-time case/session/grant/revision/
member hash revalidation, existing signed prepared-journal retention, validated
Findings/Timeline/Reports. Internal URLs-only observations stay separately tested;
case adapter requires both tables and rejects orphans. Table page sets cannot alias.
Deterministic rowid ordering/family-image-declaration hashes and bounded reports.

No SQL/live profile/path resolution/decryption/Downloads/Sessions/deleted recovery,
new database/updater/authority/destructive API. Existing Workbench manifest content,
Chromium bookmarks, DC MBR/EBR/GPT and SQLite family adapters preserved. Supported
schema/limits: docs/CHROMIUM_HISTORY.md.8MiB family/image,4096rows/table,128findings,
64KiB record/report, bounded8192ASCII/32column grammar and cooperative5s deadline;
large valid profiles can reject without partial findings. Raw microseconds remain
exact; display floors to milliseconds. Source-recorded visits do not establish
human viewing, identity, authenticity/common acquisition or external trusted time.

## 4. Modified files

[implementation-files.txt](implementation-files.txt) lists every implementation
path relative to actual parent5129a15. Key changes: ChromiumHistory case wrapper/
retained validator, existing reader metadata/alias guards, SqliteTableReader visited
pages; ForensicAdapters, LawCaseBridge/LawCaseForensicAdapters, CaseTimeline;
CaseOperationIntent/public verifier/interop; GovernmentAnalysisPage and native/
Qt/table tests; CMake, independent fixture/Python oracle, source permission/gates/
research/docs/state/evidence. source-sha256.json binds changed runtime/test inputs.
Concurrent original evidence retained separately; no force push or lost work.

## 5. Tests / actual evidence

Exact source [C++40688214497](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214497):
**7/7 jobs success**. Exact source [Python40688214512](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214512)
and [public-history40688214536](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214536) success.

| Scope | Actual result |
|---|---|
|Windows Production/Laboratory|52/52 CTest each; MSVC /WX passed|
|Linux Production/Laboratory|52/52 CTest each|
|Native forensic adapters, four profiles|27/27 each|
|Qt Quick, four profiles|58/58 each|
|Signed interoperability, four profiles|75/75 each|
|SQLite/History reader|Linux29/29 observed; Windows CTest target passes, subcase stdout empty|
|Production ASan/UBSan|21/21 with unchanged normal leak checking|
|DPI|default100 and125/150 groups pass; Linux19subcases per additional scale; Windows group success retained|
|GUI repeat|3 repeats per profile pass|
|Root Python local|57/57, includes immutable SQLite JOIN/quick_check/exact microseconds|
|Selected local related|8/8,197.75seconds; additional case/identity/retention/workspace passes plus separately passed policy group|

Real SQLite WAL fixture: reordered columns/two visits/Arabic title changed in WAL,
exact UTC microsecond oracle and original byte slices. Existing1200runtime rows
across4page sizes/overflow/storage classes/large IDs/18scenarios preserved. New
malformed/orphan/type/schema/bounds/output/mutation/retained-forgery tests and
case isolation, auxiliary plaintext substitution, cancellation, logout/revocation,
audit failure, signed retention/restart tested. Original regression tests remain.
Per-profile subcase-skips.txt records platform/tool/profile exclusions; zero CTest
skipped groups does NOT imply no subcase exclusions. Windows table numeric subcount
is not invented from empty stdout. Text copies normalize CRLF to LF; provider
archive metadata/digests identify original bytes. Local LSan inspection remained
unavailable; hosted21/21 is separate exact-source evidence.

## 6. Failures / causes / corrections

- c4cfb41 C++40684287557: Windows builds failed and Linux matrix jobs cancelled;
reviewed Laboratory122803274003 failed C4310/C2220 malformed fixture casts under
/WX. Concurrent5129a15 bit-preserving correction preserved; no warning suppression.
- aacd240 Python40687970417/job122815146893:1/57, Arabic JSON expected values read
using Windows locale despite UTF-8 bytes. Explicit UTF-8 reads/generator writes
fix without changing SQLite fixture bytes/parser/QML/worker/policy/assertions.
Original full log retained; aacd240 remains failed. Corrective22cad41 accepted above.
- Expected-head update detected concurrent5129a15. Removed duplicate local parser,
reused its reader and kept its tests/evidence, then reran actual reconciled source.
Unpublished c67cdc1 was never branch HEAD. Pre-reconciliation receipts are historical.
- First strict indentation/UTC formatting/unused helper, stale native verifier,
ordered bookmarks UI layout click and generated zero-byte sanitizer object/link
failures retained. Existing actual-viewport artifactClick helper used; no skipped
assertions. Fresh Ninja sanitizer passes; zero-object cause unproven.
- Additional local security aggregate4pass/1-notrun because generated policy ELF
mode0644 refused execution. Metadata/failed XML preserved, only its own execute bit
restored with unchanged SHA, then policy group passed separately. No relabelled5/5.
- First static helper invocation supplied a placeholder expected digest; correctly
refused before verification. Correct invocation uses the published provider digest
and independently verifies all entries/binaries. This was an invocation error,
not a product/source/CI failure. No runtime change or bypass.
- Historical Audit Store/Qt/descendant problems have no proven common cause here.

## 7. Commits

Actual concurrent parent5129a15ceb44e4f63a9a8432256cdba5d91614f5;
production case integration aacd2400487ef51a2f6e21d3220d37b368237593;
UTF-8 fixture correction/tested source `22cad4191c9b1101da5c36ac154bcdc6db720bb6`.
Subsequent checkpoint only records evidence/state/research; resolve HEAD from Git.

## 8. PR / tracking

[PR17](https://github.com/nexvary/NEXVARY-MR/pull/17) remains draft, Government base;
[issue16](https://github.com/nexvary/NEXVARY-MR/issues/16) remains open. No main merge,
Government/upstream/Guardian write or messages to others.

## 9. Actual Windows artifacts / binaries

- [Portable unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214497/artifacts/12817501927),
provider SHA256 `250d55d861b46d3d2814b732dbd0a480968081425c129005ae4ee1a22411adc1`.
- [Installer unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214497/artifacts/12816589052),
provider SHA256 `b831e1c38018351f8733e547628a0b312bf9c3dbe5a1bc0dd89036fc315a87b4`.
- [Windows Production receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214497/artifacts/12816554457).
- [Windows Laboratory receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40688214497/artifacts/12816368893).

Portable inner ZIP `6287230f6cece5e937fb3da8d1d57156034af7886153f27d9bcba04d51ec6317`; setup EXE `43819fe7cb8a4ee5c142f6645d8e7653d0be9507d97c54c6e86f765b6f0e711e`.
Actual main EXE `b1a0e2dacc3e8cd505c02dfa1081fc9c0e0727e289ac649f3d1cfe5f82ed546b`;
actual worker `981403457e0f4d3395bfe11bb3a266dbfb285523e7e8b42e99d8f2843f595a83`.
All1419 SHA entries/exact file set and1418 SPDX records verified; actual AMD64 main/
worker, Qt6.8.3 unsigned production profile and identical PR checkout tree checked.
Hosted portable/installed launch0 and deployed main/worker hashes match downloaded
binaries; install/uninstall0, executable and per-user registration removal verified.
Independent public-policy audits23events each. Actual setup checksum/PE checked,
not merely archive creation. provider-artifacts.json records sizes/expiry/digests.

Same Portable/provider/main/worker hashes passed synthetic main/worker direct smoke
on existing physical Windows10 build19045, SDK-free child PATH/software renderer,
exits0, six default-denied policy categories and independent23-event audit.
No physical installer, clean Windows11, hardware renderer, real device or new-History-
specific physical scenario acceptance claim. History-specific actual QML/native
scenarios are hosted tests. Static/hosted/physical JSON retained; scripts reuse
prior sqlite-family checkpoint verification helpers. Packages remain unsigned.

## 10. Open issues / stage states

All9 broad stages remain in-progress with measurable criteria. Historical signed
Audit Store disk/in-memory sequence divergence ROOT remains open; deterministic
old-snapshot injection again fails closed, not a root-cause/later rollback fix.
Process grandchildren containment and original Qt SIGSEGV root/DPI soak/hardware
remain open. Agency identity/trusted time/product signing/clean-host/independent
forensic validation remain outside this synthetic acceptance. No government
certification, full recovery or competitor-superiority claim.

## 11. Next increment

Bounded Downloads on this one protected reader/case path, preserving raw timestamps,
URL chain ordering/provenance and source-reported state/byte counts without opening
recorded paths. Pinned-source review in NEXT_DOWNLOADS_REVIEW.md, no Downloads
enabled yet. Sessions/profile and DC NTFS/exFAT/FAT32 read-only metadata/partial
extents/recovery remain separate reviewed allowlist increments; destructive APIs
stay excluded. Original stability/security blockers remain acceptance work.
