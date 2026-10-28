# Protected SQLite table reader prerequisite — 2026-10-28

Base: `5da89556940348f94ab53ecab78686cfd1970459`, branch
`integration/forensic-engines-audit`, draft PR #17. This checkpoint adds a
test-only internal reader, not an enabled History adapter or a new desktop feature.
All nine broad migration stages remain in-progress. No merge is authorized here.

## Behavior and boundaries

`readProtectedSqliteTable` accepts the existing protected byte request and a table
root page. It first invokes the existing SQLite family verifier and traverses only
its committed image. There is no path, live profile, SQL execution, new database,
signature authority or runtime dependency. The code is compiled into the new test
target only; no production adapter allowlist, UI, signed report or timeline changes.

Supports rowid table interior/leaf pages, bounded overflow chains, SQLite varints,
NULL, exact signed integers (represented as decimal strings), UTF-8 text and BLOBs.
It rejects index/WITHOUT ROWID pages, non-UTF-8 encodings, floating point, reserved
serial types, invalid text, out-of-range/repeated page references, overlapping
cells, inconsistent lengths and key order. Limits: existing 8 MiB family/image,
4096 rows, 64 KiB payload per record, 256 fields, 32 levels, 8 MiB total decoded
payload and existing cooperative cancellation/deadline. Errors return no partial
rows. The input bytes remain unchanged.

Payload extents refer to the DERIVED IMAGE only. They are not original main/WAL
source offsets and cannot yet be retained as forensic findings. This is not a full
B-tree/freeblock/freelist/database integrity checker or deleted-record recovery.
No CREATE TABLE SQL interpretation, URL/visit column mapping, History/Downloads
report, schema-version acceptance or case retention is implemented in this step.
Original common-acquisition/authenticity limitations still apply.

Format reference: https://www.sqlite.org/fileformat2.html (table b-trees, overflow,
varints and record format). The implementation is an MR extension; upstream pins
and scoped permissions are unchanged from the SQLite-family checkpoint.

## Verification

- GCC 13.3, Qt 6.4.2, C++20 with `-Wall -Wextra -Wpedantic -Werror`: built.
- `mr_sqlite_table_tests`: 9/9 Qt entries, including init/cleanup; 7 substantive
  test/data executions. No subcase skips.
- Existing committed WAL fixture: URL and visit fields match independent SQLite
  reference rows; a changed auxiliary hash is rejected before traversal.
- Qt SQLite runtime independently creates/reads 300 records for each page size
  512/1024/4096/65536. All 1200 decoded rows match, including Arabic text, overflow,
  BLOBs and signed 64-bit extrema. The large-page fixture does not require overflow.
- Malformed cells/records/text, overflow cycles, child cycles, invalid roots,
  unsupported encoding, cancellation/deadline, 4097 rows and an 80 KiB record reject.
- 512 deterministic single-bit mutations terminate without unsafe access; errors
  expose no partial rows. Mutations in unrelated/uninterpreted bytes may succeed;
  this is not a claim to detect all tampering or prove database integrity.
- Standalone CTest configuration: 5/5 groups passed (`ctest-results.xml`).
- Related Python oracle/integration gate: 13/13 passed.
- ASan/UBSan run with `UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=0`:
  9/9 entries, process exit 0, no sanitizer finding. Leak detection is NOT accepted:
  the initial detect_leaks=1 process exited 1 after tests because LeakSanitizer
  could not read `/proc/2/task` in this environment. Its result file is retained;
  the tool diagnostic is recorded below. Hosted sanitizer CI includes this target.
- Windows, full GUI integration and hosted exact-source CI for this new tree are
  pending; earlier checkpoint results do not certify this tree. No new packages.

Initial LeakSanitizer diagnostic:

```text
Can't open /proc/2/task for reading.
LeakSanitizer has encountered a fatal error.
LeakSanitizer does not work under ptrace (strace, gdb, etc)
```

## Failures corrected before publication

The first runtime oracle exposed a dangling Qt lazy-decoder result referencing a
temporary byte slice: invalid UTF-8/truncated Arabic text. Conversion now materializes
the QString in the same full expression as the slice. Stateless decoding additionally
rejects incomplete UTF-8. Original failure output is retained in
`initial-decoder-lifetime-failure.log`. A subsequent `-Werror` build rejected a
misleading one-line conditional; the append statement was moved onto its own line.
Tests and error checks remain enabled.

## Reproduce

```sh
cmake -S . -B build-sqlite-reader -DMR_BUILD_SQLITE_READER=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build-sqlite-reader --parallel 4
ctest --test-dir build-sqlite-reader --output-on-failure
python3 -m unittest tests.test_sqlite_family_reference tests.test_forensic_engine_integration_gate
cmake -S . -B build-sqlite-reader-asan -DMR_BUILD_SQLITE_READER=ON -DMR_WARNINGS_AS_ERRORS=ON -DMR_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-sqlite-reader-asan --target mr_sqlite_table_tests --parallel 4
UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=0 build-sqlite-reader-asan/mr_sqlite_table_tests
```

`source-sha256.json` binds implementation/test/build/workflow inputs to these local
results. This report does not inherit the previous 51/51 or Windows package acceptance.
Next gate: schema-aware bounded History extraction, original-member provenance for
record extents, then case-bound retention validation and adversarial integration tests.
Audit Store divergence, descendant containment and historical Qt SIGSEGV remain open.
