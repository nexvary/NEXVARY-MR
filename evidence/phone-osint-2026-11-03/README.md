# Phone OSINT integration — verification evidence

Product source: `ffa0fa0f64dd18d1df08d537e466fa60a092d56a` on `feature/government-operations`.
Local source tree: `a6c5e9025f71a2c1596289d55a83087f8bd30864` (matches the product commit).

The local full native run passed **51/51 CTest suites**, including normal GUI, 125%/150% scale and smoke tests. The main Qt UI run passed **55 tests**. Legacy Python, research and Android recovery suites passed **42 / 2 / 19** tests. The actual modified upstream engine passed the native provider's real-engine test; mock observations are explicitly synthetic and are never used as investigative findings.

The engine recipe passed module verification, upstream scanner/API tests, private-session security tests, source vendoring and govulncheck v1.9.0 with Go 1.26.10 (zero findings). The included Linux receipt predates only the checkout-byte fix; a fresh CRLF simulation produced the same executable digest. Complete current platform receipts and vendored corresponding source are included in CI engine artifacts.

Dedicated provider/engine CI: https://github.com/nexvary/NEXVARY-MR/actions/runs/41636825857
Full Windows/Linux evaluation build: https://github.com/nexvary/NEXVARY-MR/actions/runs/41636825768
Pull request: https://github.com/nexvary/NEXVARY-MR/pull/15
Technical architecture/license report: https://github.com/nexvary/NEXVARY-MR/blob/ffa0fa0f64dd18d1df08d537e466fa60a092d56a/docs/osint/PHONEINFOGA_INTEGRATION.md

## Important limits

This is phone-number OSINT, not verified subscriber identification, live tracking or telecom-record access. Generated Google links are not observed search hits. Commercial API keys are not configured/inherited; unavailable scanners report errors. OSINT Industries is disabled.

Searches require a permitted active case, a registered device, consent and independent review tied to the case UUID and exact normalized number. One approval allows one search. Results enter protected evidence/custody/audit storage and normal evidence export controls. Original approval/session/case state is rechecked; cancellation/revocation withholds results.

OSINT case evidence and the policy receipt are separate durable commits; interrupted/incomplete receipts require review. Existing institutional identity, signing, power-loss, physical Windows/device evaluation and the earlier Government audit-store lag blocker remain outside this integration's verified claims. No ministry or other authority has been contacted.

Windows final CI/artifact acceptance is recorded separately after the workflows complete; do not infer success from this local evidence alone.

## Screenshots

Actual Qt render, synthetic test profile:

![Arabic RTL](phone-osint-ar.png)

![English](phone-osint-en.png)
