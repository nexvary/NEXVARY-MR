# Official-tool reference increment — 2026-10-23

Implementation checkpoint: `a1d964ab8364dca50fbcdb8296a0bc2dc98990a3`, branch `integration/gate-1-stages-08-60`, PR #8. This follow-up changes fixtures/tests, CMake test input configuration, provenance and CI instrumentation; the application parser/UI/operation policy remain unchanged from that checkpoint. It does not grant physical model approval or RC status.

## Actual reference experiment

Downloaded the public AOSP avbtool source via HTTPS from tag `android-15.0.0_r1`. Observed source SHA-256: `78a50c8620985709f2e131cf948dcd66ecc43f09ed735e09d75751a7e21e4eb8`; tool reports `avbtool 1.3.0`. This is a locally recorded digest, not an independently verified upstream release signature. No third-party tool source is bundled.

`research/advanced-mobile/generate_avb_reference.py` generates harmless synthetic metadata with one property, an unsigned case, and a SHA256_RSA2048 case using an ephemeral fixture-only key. It alters one signature byte for a third case. Official `verify_image` accepted the signed case and rejected the corrupt case. The temporary key was removed; no production key, certificate or phone data was used.

Native tests compare all three files with expected rollback index/location, flags, algorithm, minimum version, descriptor count and SHA-256, retaining `signatureVerified`, `oemTrustVerified` and compatibility as false. Readable envelopes are deliberately distinct from cryptographic verification. These three fixtures do not establish general differential-format coverage or a trusted firmware pipeline.

## Executed checks

- Production and Laboratory AVB Qt tests: **20 passed**, zero failed/skipped in each profile (including Qt initialization/cleanup).
- Python core/release/knowledge regression: **14 passed** after provenance changes.
- Workflow YAML parsed and the new production-Linux ASan/UBSan mutation step inspected. Remote execution of that step is **unproven**; its default leak instrumentation remains enabled on GitHub.
- `git diff --check` passed.
- The previous full **19/19 CTest** per profile and **39 Python** checks belong to application implementation `a1d964a`; they are not silently re-labelled as newly executed full checks in this follow-up. Only affected target tests were rerun here because runtime application code did not change.

Reference images, hashes and exact official-tool output: [`samples/avb-reference/`](../../samples/avb-reference/). Native logs: [`evidence/avb-reference-2026-10-23/`](evidence/avb-reference-2026-10-23/).

## Remote state and resumption

At `a1d964a`, Actions runs `39869181773` (native), `39869181742` and `39869176767` (Python) failed before any job step and reported empty runner names. One core job was cancelled. Root cause remains unproven. Full observed job state is saved in `evidence/avb-2026-10-23/remote-ci-a1d964a.json`. Desktop Commander inventory reported no connected device; no alternative Windows build or phone test was possible.

Fetch integration HEAD and PR #8 before resuming. The PR body records the latest follow-up commit and its Actions results. Keep all stage/RC blockers from `AVB_INCREMENT_2026-10-23.md`: exact-head CI, clean Windows 10/11 and package lifecycle, actual authorized Android tests, signing, complete dependency/license/SBOM review, upgrade/rollback, privacy and independent review.

Next available firmware task: rights-cleared stable OEM fixtures and a separately scoped signature/OEM-key verification design. Current output must remain untrusted and must never authorize flashing. Broader PCAP/backup parser PoCs remain independent work; they are not claimed implemented. There is no scheduled background task in this checkpoint.
