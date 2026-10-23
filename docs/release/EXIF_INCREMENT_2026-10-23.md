# Requirements audit and Exif increment — 2026-10-23

Base: `7c928b0d5cc3842964dcbe9f207971c09af588b5`. The commit containing this
document is the source checkpoint. Local branch: `feature/requirements-gap-exif`.
No remote commit, new PR, main merge, release or new Windows artifact is claimed.

## Implemented

- `docs/REQUIREMENTS_GAP_MATRIX.md`: 44 scoped requirements, evidence, gaps,
  priority, acceptance conditions, branch inventory and direct execution order.
- Determined the historical run `39896846776` failure through its public check
  annotation: GitHub did not start the job because recent account payments failed
  **or** the spending limit needed increasing. This is the service's diagnostic,
  not proof of which billing condition applied. No code-build error was recorded.
- Verified the baseline native run `40000060143`: all seven jobs successful,
  including Windows production/Laboratory, Linux production/Laboratory,
  both core jobs and tracking server. Its installer/portable artifacts exist and
  are not expired at review time. These artifacts predate this increment.
- Added asynchronous bounded native JPEG Exif analysis, independent reference,
  adversarial and mutation tests, case provenance/audit/export integration,
  accessible AR/EN controls, capability entry and user instructions.
- Corrected stale documentation describing scrcpy as unimplemented and native
  PCAP as absent. Existing implementations were reused, not re-created.

## Local verification

Ubuntu 24.04, GCC 13.3.0, Qt 6.4.2, Release, warnings as errors; pinned libsodium
1.0.22 prepared using the repository helper, including update-verifier tests.

| Check | Production | Laboratory |
| --- | --- | --- |
| Full CTest | 29/29, 82.18 s | 29/29, 103.37 s |
| Exif Qt cases | 18 passed / 0 skipped | 18 passed / 0 skipped |
| QML cases | 34 passed / 0 skipped | 34 passed / 0 skipped |
| GUI smoke | passed | passed |

Python foundation regression: 27/27. Tracking-portal regression: 6/6. New Exif
mutation target: 12000 deterministic cases under AddressSanitizer and UBSan,
passed with `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`.
LeakSanitizer initially failed to inspect `/proc/.../task` in this execution
environment. This is **not leak-test coverage**. The CI workflow retains normal
sanitizer behavior and now includes the Exif mutation target; no test is disabled
in CI to accommodate this local restriction.

Local integration exclusions: production intentionally skips 38 Laboratory-only
service/mirror cases. Both profiles skip five external-integration cases because
real TLS harness, ClamAV, mmls and the two independent barcode/PDF decoder tests
were not configured here. The complete output preserves those skips. Baseline
remote integration success does not replace new-head remote testing.

The first UI run found a read-only `Flow.implicitHeight` assignment; removed it
and reran both profiles successfully. Two local production build outputs also
had invalid executable data/permissions, with no proven root cause; a clean
build and full regression passed, and final binaries retain valid ELF headers.
Initial failure transcripts are retained. No assertion or timeout was relaxed.

Raw XML, test logs, independent reference and actual AR/EN Qt window captures
are in `evidence/exif-2026-10-23/` and `samples/exif-reference/`. All captured
case/device/customer content is synthetic. Screenshots are Linux Qt runtime
evidence, **not Windows or physical-device acceptance**.

## Publication and remaining work

The GitHub app requires a connection `link_id` not supplied by its exposed
connection tools; the installed plugin-id attempt returned Unknown tool.
Standard Git push dry-run cannot obtain credentials. Remote Desktop Commander
reports zero connected devices. Public Git/API reading works. These observations
are captured in `REPOSITORY_AUDIT_2026-10-23.json`. No credentials or account
settings were changed. Changes must be transferred from the local commit and
tested on Windows before integration promotion.

Next: publish the tested branch when the authorized connection is repaired and
check exact-head CI; independently implement a known-artifact authorized backup
format with stable fixtures, then a bounded ALEAPP adapter. Continue privacy/
retention and dependency audit work separately from hardware gates. Clean
Windows 10/11, authorized phone/scrcpy/driver tests, signing and reviewer approval
remain required. Stage 60 is not accepted.
