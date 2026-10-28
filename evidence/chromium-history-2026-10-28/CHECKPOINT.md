# Internal Chromium History prerequisite — 2026-10-28

Base `c4cfb41737ac4e31b06007b77d23b4970933eba6`, existing integration branch and draft
PR17. Resolve the implementation commit with git rev-parse HEAD. This increment
is not case/UI enablement; all nine broad migration stages remain in-progress.
See `docs/CHROMIUM_HISTORY_READER.md` for the exact scope and limitations.

## Implemented

Schema-aware protected-byte History join, distinct URLs-only metadata, explicit
unmatched visits, raw/exact timestamps and large row IDs, nullable values, storage
class validation, source main/WAL/schema/cell/overflow provenance, deterministic
bounded report, cancellation and refusal without partial findings. The internal
reader is compiled only into its test executable. Production SQLite family
verification additionally returns an internal page-origin map, without changing
the existing persisted family report/adapter format or allowlist.

## Local exact-input verification

GCC13/Qt6.4.2 Release, strict -Wall/-Wextra/-Wpedantic/-Werror:
- SQLite/table/history suite 27/27 Qt entries (25 substantive data/test executions;
  init and cleanup included). No skips. Independent real SQLite WAL join oracle,
  reordered/current sync-field schemas, Arabic title, IDs above 2^53, NULLs,
  zero/negative/max-int64 timestamp and exact one-microsecond conversion.
- Missing/empty/unsupported visits distinguished; missing URLs, unknown/quoted/
  WITHOUT ROWID schema, text numeric impostor and BLOB URL refusal. 129-result and
  64 KiB-output overflow fail without partial reports. Actual source slices equal
  derived image slices for data/schema references and include committed WAL bytes.
- Existing 512-bit mutation corpus plus 256 new History page-image mutations,
  repeated deterministic output, cancellation/deadline and changed WAL refusal.
- Standalone CTest 5/5 groups (`ctest-results.xml`). Root Python 55/55.
- Existing Government forensic adapter regression 25/25, with real signed case
  retention, revocation and SQLite-family tamper tests; no UI change is claimed.
- ASan/UBSan: 27/27, exit0 with halt_on_error=1 and detect_leaks=0. Leak checking
  remains unaccepted locally: detect_leaks=1 cannot inspect /proc under this host;
  original diagnostic retained, no hosted sanitizer gate disabled.

Results cover the hashed source/test/build inputs in source-sha256.json. Hosted
Windows/Linux exact-source CI and package acceptance are initially pending; parent
CI results do not accept this implementation. No hardware or physical-host claim.

## Failures and correction

- Parent PR run40684287557 Windows Laboratory/job122803274003 failed compiling
  sqlite_table_tests.cpp at lines87/91: C4310 constant-to-char truncation with /WX,
  not a QML/installer error. Full decoded original log retained. Represent 0xff
  via std::bit_cast<char>(quint8(255)), preserving all malformed-byte assertions
  and /WX. That parent remains failed; exact corrective-source CI is required.
- First new fixture build had an ambiguous QVariant conversion of the literal
  -2147483648. Use std::numeric_limits<int>::min(); original build log retained.
- First nullable oracle comparison compared an invalid NULL QVariant with Qt SQL's
  typed NULL QString QVariant. Both are NULL. The assertion now checks isNull()
  for actual NULLs and exact values otherwise. Original failure retained; product
  nullable behavior and all non-NULL equality checks remain unchanged.

## Reproduce

    cmake -S . -B build-history -DMR_BUILD_SQLITE_READER=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Release
    cmake --build build-history --parallel 4
    ctest --test-dir build-history --output-on-failure
    python3 -m unittest discover -s tests -v
    cmake -S . -B build-history-asan -DMR_BUILD_SQLITE_READER=ON -DMR_WARNINGS_AS_ERRORS=ON -DMR_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build build-history-asan --target mr_sqlite_table_tests --parallel 4
    UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=0 build-history-asan/mr_sqlite_table_tests

Next: inspect exact-source CI, then retained History report validation and existing
case/worker/operation integration before Downloads. Audit Store divergence,
descendant containment, historical Qt root/soak/hardware and institutional/signing
boundaries remain OPEN. Main/Government/Guardian are unchanged; no PR merge.
