# Bounded synthetic case-service benchmark

Build the explicit test-only `mr_case_workload_benchmark` target, then run it
with `QT_QPA_PLATFORM=offscreen`. Windows multi-config path is
`build/Release/mr_case_workload_benchmark.exe`; Linux is
`build/mr_case_workload_benchmark`. Capture stdout as JSON. The target is excluded
from ordinary all builds and from the product bundle. Production CI explicitly
builds/runs it and preserves the JSON; it does not impose timing assertions.

Each of three sizes is repeated three times in a fresh temporary identity/case
store. Real synthetic administrator/investigator login, native case authority,
policy readiness, protected evidence registration, isolated JSON import, Findings,
literal Timeline search, PDF, immediate cancellation and clean reopen are used.
No user evidence, device, network, external grant or commercial engine is used.
The corpus has 4/16/32 objects, each with a mixed Arabic/English name, explicit
UTC time and 1024-byte payload. Sources are 4421/17687/35383 bytes. One case,
one device, one evidence item and one successful import per repetition. Rejected
input behavior is covered separately; cancellation must not retain a result.

The async deadline is 15 seconds; native worker deadlines/input/report limits
are unchanged. OS/disk caches are not flushed. Case-open is a warm action;
clean reopen includes constructing a new bridge, policy attachment and opening
that case, with the existing live identity. It is not interrupted-intent recovery,
identity startup, cold disk, restart-orphan discovery or power-loss durability.

A 10ms GUI-thread heartbeat measures event-loop gaps across these services.
It does not measure rendered frame times, input latency, visual flicker or a
complete interactive QML workload. Linux RSS is the cumulative parent-process
high-water mark, including earlier repetitions and excluding worker children.
No memory cap or large-corpus support is established. Run separately from builds
for isolated-machine comparisons; initial local observations overlap compilation.

## Initial local observation (2026-11-02)

Cloud Ubuntu24.04 x86_64, GCC13, Qt6.4.2, Release with warnings-as-errors;
AMD EPYC9V74,9 reported logical CPUs, MemTotal10206472KiB. Three repetitions/size, warm OS caches; overlapping
local compilation. Times below are medians in milliseconds, not acceptance limits.

| Source bytes | Open case | JSON import | Findings | Timeline search | PDF | Immediate cancel | Clean reopen |
|---|---:|---:|---:|---:|---:|---:|---:|
|4421|2.59|26.84|5.63|4.74|65.84|8.62|3.21|
|17687|3.43|24.55|8.98|5.50|93.94|7.48|6.47|
|35383|6.20|31.26|16.08|9.39|169.88|12.31|6.85|

Maximum observed heartbeat gaps by size were94.295/110.407/181.254ms.
Parent lifetime peak RSS reached96020KiB across all repetitions. This small
corpus already shows synchronous operations can delay the GUI-thread loop;
no universal responsiveness or performance target is claimed met. Raw samples
and exact implementation receipts belong under
`evidence/worker-acceptance-2026-11-02/`. Hosted Windows/Linux results are separate.

Still open: statistically larger corpora/many cases, sustained rendered GUI
latency, memory budgets including children, independent cold-disk tests,
interrupted recovery timing, worst-case cancellation, physical Windows10/11,
full device pipelines and institutional performance targets. This does not
replace the separate readiness-only benchmark or close historical DPI crashes.
