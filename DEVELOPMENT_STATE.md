# Current UI rebuild checkpoint — 2026-10-23

Resume `feature/professional-workstation-ui`. The new 12-section Qt/QML shell,
responsive dashboard/device operations, shared cards/tokens and 24 licensed color
SVG assets are implemented. Current acceptance/evidence: `docs/ui/REBUILD_ACCEPTANCE.md`.
Native services/policy remain authoritative; no main merge or Stage 60 claim.

Local production CTest 31/31; final UI regression 37 passed with zero QML warnings.
Laboratory and Windows validation are being rerun for this source increment.
GitHub connector access is now working for the NEXVARY account; the earlier
connection-failure statements below describe the preceding checkpoint only.
Remote Desktop Commander still reports no connected device. Use Windows CI,
then clean-host/physical acceptance separately. No work runs outside an active session.

---

# NEXVARY-MR continuation checkpoint

Date: 2026-10-23. Base remote commit: `7c928b0d5cc3842964dcbe9f207971c09af588b5`.
Local branch: `feature/requirements-gap-exif`. The commit containing this file
is the tested source checkpoint. Main remains unchanged. Stage 60 is not passed.

Read `docs/REQUIREMENTS_GAP_MATRIX.md` and
`docs/release/EXIF_INCREMENT_2026-10-23.md` before making changes. The source of
truth is the actual commit and raw evidence, not old conversation reports.

## Completed this increment

44-item gap matrix and branch/PR/Actions audit; proven annotation for historical
CI failure; bounded JPEG Exif allowlist; case integration and consented AR/EN UI;
tests, mutation target, independent Pillow fixture, documentation corrections.
Local Release warnings-as-errors CTest: production 29/29, Laboratory 29/29.
Exif 18 and QML 34 cases per profile; Python 27 plus portal 6; ASan/UBSan mutation
12000 passed without local leak inspection. Intentional/external skips and initial
failed runs are preserved under `evidence/exif-2026-10-23/`.

## Do not repeat or misreport

- scrcpy, barcode/device cards, AVB, classic PCAP, DPAPI update-key storage and
  authenticated journal predate this increment. Inspect existing code first.
- Baseline CI 40000060143 and its Windows artifacts passed at 7c928b0. They do
  not contain this increment; Windows CI for this local commit is still pending.
- Historical run 39896846776 was stopped by GitHub's billing/spending diagnostic
  before job execution. Do not invent a YAML/build failure for it.
- No new remote PR/commit/release exists from this session. Git push lacks
  credentials, the GitHub app connection is unusable, and no remote desktop is
  connected. Recheck actual authorization surfaces when they become available.

## Resume execution

1. Restore/fetch this local branch from the supplied Git bundle or apply its
   binary patch to the exact base. Preserve any unrelated work; never reset main.
2. Compare with any newer remote integration HEAD. Publish a development branch
   through the working authorized GitHub connection; create/update a review PR
   without merging main. Run exact-head Windows/Linux CI and fix actual failures.
3. Work on gap 19: choose one unencrypted owner-authorized backup format, provide
   known populated artifacts, reject malformed/oversized input, preserve hashes,
   and expose results through the existing case engine. Do not claim generic
   Android/iOS extraction or cryptographic bypass.
4. Then gap 20 ALEAPP external adapter and gap 23 PCAPNG, with pinned provenance,
   bounded copies/processes/output and independent fixtures. Keep device/clean-VM
   gates separate; do not postpone independent code because hardware is absent.

Build both profiles using repository `scripts/prepare_sodium.py`, then the
commands in `docs/release/DEVELOPMENT_CHECKPOINT_2026-10-23.md`. No background work
or continuation outside an active session is scheduled or claimed.
