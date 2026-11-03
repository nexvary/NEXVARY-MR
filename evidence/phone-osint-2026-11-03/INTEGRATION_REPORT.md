# NEXVARY-MR — Phone Number OSINT evaluation

Product commit: `a9652a4b0ce789ff0a6ac629a0945e4db58827cc`  
Branch: `feature/government-operations`  
Review: https://github.com/nexvary/NEXVARY-MR/pull/15

## Delivered capability

An independent Qt/C++ provider starts a reviewed PhoneInfoga build as an owned process, proves the private session before transmitting a number, uses loopback-only REST, applies startup/scanner timeouts and cancellation, and stops the process after each request. No shell or background daemon is used. Executable SHA-256 is checked and the verified bytes are copied into a private workspace before execution.

A single normalized international number requires authenticated case access, a registered device, consent and an independently reviewed exception for that exact case UUID and number. An approval is consumed after one search. Session loss, revocation, cancellation and case changes withhold stale results. A 25 ms cancellation poll and a 50 ms authority monitor bound the response to changes.

The bilingual Arabic RTL/English workspace distinguishes offline numbering-plan metadata, generated search links, unverified public observations and failures/unavailable scanners. It retains source attribution, timestamps, raw-response hashes and a protected JSON evidence report linked to the original operation. Existing custody/audit/working-copy export controls apply. OSINT Industries remains disabled; service API credentials are intentionally not inherited.

## Validation

| Check | Result |
|---|---|
| Full local native CTest, including scale 125%/150% and GUI smoke | 51/51 passed on the final source, including the exact-target guard |
| Main Qt interface tests | 55 passed |
| Legacy Python / research / recovery | 42 / 2 / 19 passed |
| Actual patched upstream engine through native provider | Passed |
| Cancellation / missing scanner / malformed or oversized response / wrong session proof | Passed |
| Case binding, single-use approval, protected retention and revocation | Passed |
| Go module verification, scanner/API tests, session tests | Passed locally and in Windows/Linux CI |
| govulncheck v1.9.0, pinned Go 1.26.10 and updated module overlay | Zero findings locally and in Windows/Linux CI |
| Reviewed Windows engine and real provider lifecycle | Passed; artifact published |
| Full Windows evaluation build | Passed: 54/54 CTest suites, portable smoke, installer installation/runtime/uninstall |

Reviewed Windows engine: https://github.com/nexvary/NEXVARY-MR/actions/runs/41638905805/artifacts/13231778856

Dedicated engine/provider CI: https://github.com/nexvary/NEXVARY-MR/actions/runs/41638905805  
Full Windows/Linux PR build (all seven jobs passed): https://github.com/nexvary/NEXVARY-MR/actions/runs/41638917548

The PR merge commit `823b5bcd9fa85a5d6e40869c80d0a195955af28d` has the same source tree `6c6e4518996047b1f472f30039e91222bd86d37b` as the product branch commit.

Windows installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/41638917548/artifacts/13232583870

Windows portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/41638917548/artifacts/13232519086

Windows installers/binaries are unsigned evaluation artifacts; physical Windows 10/11 and real-device validation remain separate.

## Use the evaluation build

1. Extract the Windows application bundle and the separately packaged reviewed Windows engine. Keep the engine license and corresponding-source archive.
2. Open an authorized case and register/select its target device record.
3. Request `external-analysis` using the actual case UUID and the exact normalized number (for example `+12025550100`) in the reason. Complete independent review/approval, then activate it as the requester.
4. Open **Forensic Analysis → Phone number OSINT Intelligence**. Enter the engine's absolute path and `executableSha256` from its trusted `build-receipt.json`.
5. Enter one international number, select scanners and confirm consent. Start with `local` and `googlesearch`; commercial-key scanners explicitly fail when keys are unavailable.
6. Inspect classifications and sources. Export the retained JSON report from the normal evidence workspace using its existing authorization controls.

## Scope and limitations

Results are not verified subscriber identities, live device locations or telecom records. Numbering-plan geography is not device location; generated Google links are not observed search hits. A zero-finding vulnerability scan is not a security guarantee, and an owned local process is not an OS or outbound-network sandbox.

PhoneInfoga is unmaintained upstream. The distribution uses pinned source plus documented GPL-3.0 modifications, updated dependencies, a complete vendored corresponding-source archive, license and binary/source SHA-256 receipts. Exact golden-fixture bytes are restored from the pinned Git archive and checked against Git blob digests; Windows checkout line endings cannot silently change reference results.

OSINT evidence and policy receipts are separate durable commits; interrupted/incomplete receipts require review. This integration does not close the pre-existing Government evaluation blockers, including institutional identity/signing, power-loss and physical Windows/device evaluation or the earlier audit-store lag investigation. No authority has been contacted and the Government PR remains draft.
