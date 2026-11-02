# Worker acceptance and synthetic storage boundaries — 2026-11-02

Government runtime a23c9d8; subsequent isolated-supervisor acceptance tests
published as78f876d. Actual integration5a4efc9 already contains a23c9d8, with
merge-base a23c9d8. No duplicate transfer or PR/main merge was performed.

A stronger existing import cancellation regression failed against unchanged
published runtime: JSON and ADB v5 both retained a rejected result after explicit
cancellation. `import-cancel-baseline.txt` preserves that product failure.
The earlier `first-missing-sqlite-plugin.txt` is a distinct environment failure,
corrected by supplying the existing Qt plugin prefix; it is not the product
counterexample. The fix clears original import context before stop so the late
callback cannot persist either success or rejection. Regression compares the
entire active case and a fresh on-disk case reader before/after cancellation.
Ordinary parser rejection remains audited/persisted; original grant checks stay.
`import-cancel-fixed.txt` records the corrected two adapter rows plus setup/cleanup.

`audit-boundaries.txt` records reconstructed synthetic fixtures, not the missing
original private135/139 bytes. Four process exits cover before-write,
before-replace, after-replace/before-memory-ack and acknowledged boundaries.
Each retains a public signed135-event prefix and recovers exactly138 or139,
with no replay.150 consecutive acknowledged snapshots compare live export,
anchor and a fresh reader.7/7 targeted rows pass. No historical root cause
was reproduced/closed, no disk-cache/power-loss durability claim, and no signing
key/protected snapshot is published. Existing contention/symlink tests retained.

`supervisor-deployment.txt` records16/16 Linux tests, including missing,
non-executable, symlink and zero-exit/no-receipt helpers plus the two retained
escape negatives. No hostile-same-user isolation, cgroups or restart cleanup
proof is claimed. Receipts remain runtime observations, not independent durable
signed forensic evidence.

`workload-first.json` measures three repetitions of three declared small corpora.
The initial strict benchmark build failed misleading-indentation (retained in
`first-benchmark-build.txt`), corrected without reducing warning policy.
See docs/CASE_WORKLOAD_BENCHMARK.md for medians, event-loop/RSS scope and limits.
No new visual fix or acceptance, large-case support, institutional accreditation,
trusted publisher signing, independent competitor result or physical host proof.

A real Linux directory-write permission withdrawal at the before-replacement
checkpoint also refused the append at the replacement stage, kept the prior
bytes/anchor and required reopening.3/3 targeted rows passed (including fixture
setup/cleanup); the expected lock-removal permission warning is preserved in
`permission-transition.txt`. Windows ACL transition is explicitly not covered.

Initial full local CTest46/51 passed;5 failures remain retained as failures.
Four UI groups could not resolve the absent QtQuick.Controls QML plugin; the
forensic generated executable was an empty non-executable file (hash/size kept).
Serialized artifact-only relink passed the forensic group. Official pinned Qt6.4.2
QML runtime packages were downloaded/extracted to a separate local prefix for
replay; no source test, assertion or deadline was removed. The earlier manual
forensic replay omitted offscreen plugin setup and failed before tests; the
following CTest replay supplied its original offscreen environment and passed.
Neither observation closes the historical Audit Store or125%DPI crash root.

Integration5a4efc9 independently completed Push41440908672 and PR41440914461.
Its downloaded PR package correctly identifies37bda8f, whose full tree matches
5a4efc9; the first verifier invocation expected the head SHA and refused that
profile, then the actual PR source was verified explicitly. Source equivalence
is recorded separately.1420 checksummed files,1419 SPDX files,54 Windows groups,
84 independent signed interop checks and actual portable/installed main+worker
byte equality passed. This package predates the cancellation correction and is
not delivered or relabelled as the later source.

After runtime restoration the four original UI groups passed4/4 in191.22s,
including default54/54,125%46.31s,150%49.42s and actual main smoke19.68s.
Actual Arabic125% and English150% import frames were inspected and retained.
No visual source change or historical crash-root fix is asserted. Additional
serialized repeat observation is tracked separately and preserves deadlines.
