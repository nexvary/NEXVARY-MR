# Stable refresh and consented screen view — evaluation receipts

Executable source: `bc8fb86ee5b145d2877c40c8cd7ed241fc09f3da`.
PR #15 remains draft/open, no main merge. Guardian is untouched.

The user reported blue activity pulses and unavailable screen view. Automatic ADB
refresh previously occupied the foreground command runner, cleared unchanged
capability/driver data and triggered shared card spinners. A separate 3-second,
512-KiB discovery reader now keeps unchanged targets/properties/status stable,
ignores superseded results and invalidates targets on failure. Identity cells are
retained across report updates and refresh keeps the last dated snapshot until
success/failure. UTC timestamps preserve ordering in Arabic RTL.

Dedicated screen view requires exact serial/owner/component approval in both
profiles. Generic device writes remain Laboratory-only. scrcpy remains a separate
upstream program, with no control/audio/clipboard synchronization, no automatic
fallback target, bounded checks/output and owned-process cleanup. No passive
acquisition or locked-device bypass is claimed. See docs/SCREEN_VIEW.md.

Windows packages prepare the fixed SHA-256-verified official scrcpy 5.0.1 archive
at build time. All original files and LICENSE.txt are retained, with file hashes in
SOURCE_PROVENANCE.json. Portable/installed smoke checks verify real app resolution,
all retained file digests and actual official executable startup via --version;
they do not address a phone. The official Linux 5.0.1 CLI was also probed locally
with --version/--help. Version/hash pins are not publisher signing or accreditation.

Local raw failures are retained rather than relabelled as passes:
- Initial fixtures exposed manual-scan invalidation and a dynamic-QML lookup error;
  corrected source/UI fixtures passed the focused suite.
- An original near-deadline ADB regression failed under load; it still crosses the
  obsolete prior deadline, with 2 seconds of startup margin for its own deadline.
- A native GUI smoke ended with code 7: its internal 20-second watchdog expired.
  It now permits a bounded 45-second capture sequence and prints diagnostics.
- One persisted generated cloud-test ELF was truncated (readelf detected missing
  section headers). Relinking repaired that output without a source change.
- A subsequent full run passed 39/40; rebuilding QML while that run was active
  replaced the app output and removed its execute mode, preventing GUI smoke from
  starting. A later full run passed the same 39 groups, but the app output had a
  zeroed ELF header; direct inspection and relinking repaired it. The isolated
  GUI smoke then passed 1/1 in 16.48 seconds. This is all 40 groups covered across
  runs, not a claimed single 40/40 invocation. These output failures are retained.

Local strict Release build, Python 38/38 and ASan/UBSan tool/mirror 2/2 passed.
Leak inspection was disabled locally; hosted sanitizer results are separate.
Final local coverage: 39/40 in `full-before-executable-relink.txt` plus GUI smoke
1/1 in `relinked-gui-smoke.txt`; native ADB 24/24, mirror lifecycle 21/21, UI
45/45 including both extra DPI groups. See raw receipts; a hosted clean build
is the independent full-suite and deployed Windows packaging gate.

Captures are real Qt application windows using generated synthetic phone/case
fixtures. The user's private photograph and actual device serial are not published.
No physical phone was connected to this environment. Frame delivery/cleanup on the
reported OPPO firmware, user-machine flicker, clean Windows 10/11, signing, agency
acceptance and independent privacy/license/records review remain external gates.
Government Phase 2 import/timeline/relationship/offline work remains pending.

## Hosted exact-source verification

Run 40190246495 passed all seven jobs. Windows production/laboratory and Linux
production/laboratory each passed 40/40 CTest; production sanitizers 11/11.
Python 40190246502 and public-history 40190246577 passed. Windows ADB 24/24,
mirror 21/21 and UI 45/45 passed in both profiles. Intentional unrelated platform/
profile skips are visible in native logs, not claimed as executed lab coverage.
Portable isolation, bundled official scrcpy version/digests, installed-in-place
runtime, installer install/uninstall and checksum steps all passed.

The actual production evidence ZIP was downloaded and matched provider archive
SHA-256 f88c571ec9ab76678c9cb99a1865910f76f187728b35532c0bb3a7656415109e.
AR/EN Windows captures were visually reviewed. Both portable/installed probes
resolved bundled ADB/scrcpy; both actual official version outputs report 5.0.1.
Installer receipt reports install/uninstall exit 0 and removed executable and
registration. See hosted-ci.json, hosted-log-excerpts.txt, windows-* receipts and
windows-screen-view-ready-{ar,en}.png. This is hosted Windows Server evidence,
not physical-phone or independent clean Windows 10/11 acceptance.
