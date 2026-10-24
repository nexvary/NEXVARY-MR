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
