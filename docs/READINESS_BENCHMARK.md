# Bounded readiness benchmark — 2026-11-02

Build the explicit test-only `mr_readiness_benchmark` target from a configured
Qt/BUILD_TESTING checkout, then run it directly:

```sh
cmake --build build --target mr_readiness_benchmark --parallel 2
./build/mr_readiness_benchmark
```

Windows multi-config executable: `build/Release/mr_readiness_benchmark.exe`.
It is not registered as a timing assertion in CTest and is not a shipped product
worker. It creates fresh synthetic local identities/cases in temporary directories,
not user cases or devices. One protected evidence source and one retained synthetic
report are used per case. No parser is timed: the fixture report is intentionally
synthetic. No external tool/network launch is required.

## Observations on this cloud host

Linux x86_64 / Ubuntu24.04, GCC13.3, Qt6.4.2, Release, production-read-only compile
profile with explicit synthetic fixtures; virtual CPU reports Intel Xeon Platinum
8573C. This is not the user's laptop or a physical Windows performance result.
One observation per input size,100 unchanged readiness reads per observation.
OS/filesystem caches were not flushed; first attachment means an empty *application*
proof cache, not a cold physical disk. Concurrent local builds existed for some
observations; there is no confidence interval/isolated-machine comparison claim.

| Protected source | Original100 display reads (ms) | Cached100 display reads, final manual target (ms) | First application attachment, final target (ms) |
|---|---:|---:|---:|
|32 bytes|556|9|5|
|1 MiB|5337|6|33|
|8 MiB|35286|8|244|

Original measurement used unchanged Government27c15d6 runtime plus a standalone
manual harness, before the readiness change. The final target measures the new
working-tree source pinned by the accompanying source hash receipt. Earlier target/harness observations measured12/7/9ms and7/11/8ms; do not combine these
single observations into statistical acceptance. Integration has separate timings.

This shows why repeated complete source hashing in a display getter was unsuitable.
The optimization does not remove full verification from startup/native actions or
persistence. It keeps only in-memory change tokens, uses the existing audit-store
exact-byte check, and still fails closed on changed stores. The SQLite connection
version is not trusted time, an external anchor or an acquisition attestation.

## Still unmeasured

Multi-case/corpus scale,8192-event/full16MiB journal status reads, sustained UI frame
latency, peak memory, cold storage, repeated distributions, loading/import/index/
search/PDF throughput, cancellation/recovery latency, power loss and physical
Windows10/11 remain open. Existing source/record/report bounds are capacity bounds,
not measured performance or large-case support. Full product benchmark and long
serialized UI soak remain required; this small slice does not close those gates.
