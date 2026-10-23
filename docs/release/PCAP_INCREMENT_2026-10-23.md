# Offline PCAP increment — 2026-10-23

Branch: `integration/gate-1-stages-08-60`; PR: https://github.com/nexvary/NEXVARY-MR/pull/8. Starting HEAD rechecked: `ff21fbd9eabdba910242a503445159f507beb9df`. The commit containing this document introduces the increment; PR #8 records its exact SHA and remote runs. No main merge, tag, Windows artifact or Release Candidate is asserted.

## Implemented

Original native bounded classic-PCAP summary, both byte orders and timestamp resolutions, with strict record validation and header-only Ethernet/VLAN/IP statistics. This is an offline statistics capability, not packet-payload analysis or a Wireshark replacement. PCAPNG, live capture, flow reconstruction and TLS decryption remain unsupported.

The existing RecoveryBridge job, case protection, operator/consent gate, evidence store, audit chain, source/export protections and report engines are reused. The source is read twice and its SHA-256 must match before storing a summary. Addresses and packet payload are omitted from the summary; internal case provenance can contain the source path and remains subject to existing plaintext-export consent.

The AR/EN Tools panel supports file selection, consent, analysis, cancellation and a tested View summary action opening the exact generated evidence record. Evidence filtering includes native-analysis. The forensic heading preserves its implicit text height at the supported minimum. Added a traceability registry entry with no chipset/device/firmware approval.

## Final executed validation

Environment: isolated Linux builds under `/tmp`, Qt 6.4.2, GCC 13, Release, warnings as errors. This session restored missing build dependencies; old build outputs were not used as proof of new execution.

| Check | Actual final result |
| --- | --- |
| Production CTest | **21/21 groups passed**, 78.48 seconds |
| Laboratory CTest | **21/21 groups passed**, 84.09 seconds |
| PCAP Qt cases | **16 passed**, zero failed/skipped, in each profile; totals include init/cleanup |
| PCAP mutation smoke | **30,000 deterministic cases** per normal target run |
| PCAP AddressSanitizer/UBSan target | Passed, 1.73 seconds; `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1` |
| UI Qt cases | **27 passed**, zero failed/skipped, each profile |
| Python | **39 passed**: 14 core/release/knowledge, 19 recovery, 6 portal |
| Workflow | YAML parsed; existing production-Linux instrumentation step now builds/runs both AVB and PCAP mutation targets; GitHub execution not inferred |
| Runtime screenshots | Actual full Qt windows at 1180×760, Arabic/English, synthetic authorized capture only |
| Source/diff review | Changed native/QML paths reviewed against actual starting remote content; no unexplained drift in unchanged native/QML files (some local baseline files had only an extra EOF blank line) |

Intermediate screenshot-harness failures were corrected before final full runs. Initial item-grab capture lacked a QML engine; final evidence uses the native window capture API. The Laboratory UI test binary was rebuilt with the same helper/context as the final QML test before its final 21/21 run. No test was disabled to obtain the final result.

Leak instrumentation remains unavailable in this environment, as documented at the preceding AVB checkpoint. The local address/undefined check above is not a leak-check claim. GitHub's instrumentation step leaves normal leak detection enabled.

This session's real external mmls, real ClamAV and real-server TLS cases were **skipped** because the separate tools/temporary server credentials were not provisioned. Their fake-tool/unit coverage did run; earlier real-tool/TLS evidence remains separately attributed to its earlier checkpoint. Production service tests intentionally skip 21 Laboratory-only write fixtures; Laboratory executes those synthetic cases. No physical USB or phone was exercised.

Raw final XML/logs and actual images: [`evidence/pcap-2026-10-23/`](evidence/pcap-2026-10-23/). Scope and primary references: [`docs/forensics/PCAP_SUMMARY.md`](../forensics/PCAP_SUMMARY.md).

## Stage effects and continuation

This increment adds evidence/JSON handling within 25–30, hostile-input/cancellation coverage within 13–18, and AR/EN/navigation/layout evidence within 49–51. It does **not** complete these entire stage groups. The compiled device default-deny policy is unchanged.

Previous Actions at `ff21fbd` still fail before any step, with empty runner names; cause unproven. Inspect the new commit's runs before resuming. The PR body records their observed state. Do not treat an empty-step failure as an executed compiler/test failure or success.

Still blocked for RC/stage 60: admitted exact-head CI, native clean Windows 10/11 and installer/portable/uninstall acceptance, actual authorized Android/model/USB tests, signing, complete dependency/SBOM/license audit, upgrade/rollback, privacy and independent review. No connected Windows/Android environment has been demonstrated.

Next independent work: validate offline captures against a rights-cleared independent corpus, expand unsupported capture formats only with explicit parser tests, and review local firmware file I/O using the regular-file safeguards already used by the forensic engine. Keep payload/application decoding a separate scoped capability. Fetch HEAD before editing and continue within the existing branch/PR; no background task is scheduled by this checkpoint.
