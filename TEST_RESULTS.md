# Current NPSE validation — 2026-10-24

**The earlier pending CI/runtime statements below are historical.** Final verified
results: engine source 514afe8, no-manager run 40126853530 and live-manager run
40126853544 passed. API33–36 each 4 runtime passes plus 1 intentional opt-in skip;
official/Next each 1 live Binder pass on API35. Guardian source 7f32420 and run
40126176694 passed build/lint/unit/signature/backend and 80 runtime cases on each
API33/34/35. Core remains 47 assertions and 12 JUnit cases.

Read `docs/npse/COMPLETION_EVIDENCE.md` and `docs/npse/ACCEPTANCE_STATUS.json` for
exact evidence scopes, skip accounting, inspected artifacts and physical-device
limits. Actual AR/EN and live-manager screenshots now exist. Physical OPPO,
wireless pairing, permission revocation, reboot and OEM battery checks remain
pending. Both product PRs remain reviewable; no production/main promotion.

---

## Historical checkpoints (retained, superseded where noted above)

# NPSE test evidence

This file is updated with actual run results before delivery. A workflow definition
or source implementation is not a passed test.

## Initial checkpoint

- Core standalone JVM tests: 44 assertions passed in the initial session using
  JDK 17, including typed allowlist, immutable argv, foreign UID/root refusal,
  permission/connection state changes, output limit, timeout, cancellation,
  interruption and sanitized failures. The current source reran with 47 assertions passed, adding live-child and launch-race cancellation checks.
- Initial hosted Android run 40109291557 on NPSE commit
  7bc9ad80b767b6d8d80a4d1ab6c4c38ffcd54a5a: **FAILED** at library lint.
  `StandardProvider.ownAppInfo` used API-28 longVersionCode with minSdk 26.
  A guarded API-26 fallback is included in the follow-up commit 353f7671ff0ccee490ae39c4c64d446312486f93.
- Follow-up Android run 40111757447 at 353f767: **build/unit/lint/APK permission checks passed**. Initial emulator API 34/35 tests failed at a raw JSON substring assertion; standard collection/export readiness succeeded. Android JSON escapes the slash in its display string. The assertion now parses JSON and checks the schema semantically. Final rerun pending.
- Initial public-history run 40109291525: operational-history guard 0 findings;
  Gitleaks historical scan 7 findings. No secret values are reproduced here.
  All seven were reviewed as public libsodium header SHA-256 values, not credentials. Seven exact historical fingerprints are now recorded in .gitleaksignore; no broad exclusions. Local repeated history and tree scans: **0 findings**; history guard unit tests: **6 passed**. Hosted rerun pending. See docs/npse/HISTORY_SCAN_REVIEW.md.

## Coverage limits

Android 13–16 emulator jobs exercise the app **without** a Shizuku manager;
they are not official/Next Binder interoperability tests. Permission-policy
unit checks are simulated, not a real manager revoke/death test.

Physical Android 13, 14, 15, 16; OPPO/ColorOS; wireless pairing; manager restart;
reboot; battery-policy impact; actual shell command success; live caller-UID
adversarial tests: **PENDING — no connected authorized device**.

Release signing with the existing product identity: pending. Debug APKs, if built,
are test-only; a debug signature is never represented as a production signature.

## Local packaged-source gate — 7386d23da7424470b3f9d2143d0b87e2813fb13c

- assembleDebug, assembleDebugAndroidTest, app/library unit tests, app/library
  lintDebug, npse-core check, library assembleRelease and both Maven local
  publications: **PASSED** (`BUILD SUCCESSFUL`, 168 tasks, final packaged retry).
- JUnit XML: app 8 tests, library 4 tests; 0 failures/errors. Core: 47 executable
  assertions, including cancellation of an actually running child and startup race.
- A separate Java consumer compiled and ran against the published core JAR.
  Published AAR POM retains npse-core and Shizuku dependencies. JAR MIT notice,
  AAR license assets and R8 helper preservation were verified in the archives.
- APK signature verified (v2), package com.nexvary.mr.agent, only the optional
  Shizuku API permission. Debug-only signer; no production release claim.
- APK SHA-256: 3d930263ac4d6b0bf5ee234ec0b666b7e1aa021415fde4ea078e1979489a55ef.
- Initial local offline retry lacked repository metadata; a subsequent online
  build passed. A later Gradle transform-cache metadata failure was isolated by
  moving that cached transform aside, then the complete packaged gate passed.
  Neither environment failure is represented as a source-test pass.
- Hosted public-history 40112500089 and 40112600832: **PASSED** after exact
  historical receipt-hash classification. Python CI passed at these follow-ups.
- Hosted final Android run: 40112601064 at 7386d23. API readback intermittently
  returned ReadTimeout/502; consult its actual final outcome. Final emulator
  acceptance remains pending until recorded below.

## Runtime acceptance blocker at handoff

The final hosted build check was still in_progress on runner GitHub Actions
1000025244, with steps=[] in the job metadata (job 120927615683), while the
jobs collection intermittently timed out/returned 502. No final hosted pass is
inferred. Initial build/unit/lint at 353f767 passed; the full final packaged
source gate at 7386d23 passed locally as detailed above.

A local Android 15 x86_64 emulator was installed from the SDK and started with
software acceleration (no /dev/kvm). It did not finish booting within the
bounded acceptance attempt and was stopped. Instrumentation did not run and no
functional app screenshot was produced. Missing-manager AR/EN/RTL emulator tests
remain **PENDING**, including the semantic-JSON assertion fix and new Arabic test.
No screenshot is represented as a successful runtime observation.

Next: obtain final run 40112601064 results or rerun on a working accelerated
runner, inspect each API 33/34/35/36 test outcome and capture the actual AR/EN
images, then execute the physical OPPO official/Next checklist. Do not mark
Stage 60 complete or promote this alpha to production.
