# NPSE alpha checkpoint — 2026-10-24

Branch: feature/npse-shared-services; MR PR #14. Last tested engine source:
`514afe8684504ce681b74d4722d5c275cc5ca9c5`. Library binary consumed by Guardian:
`0908a3363401b2ccba4292fa82a27062ab6c908f` (production module code unchanged).
Guardian PR #13, source `7f324204e2f60ab1aa42136668856676157e45b6`.
The commit containing this documentation is a later documentation checkpoint;
inspect git log -1 for its SHA. Fetch remote heads before trusting a cached workspace.

Implemented: shared Java core + Android AAR; fixed caller-checked, shell-only
Binder helper; bounded/cancelable probes; standard diagnostics, scoped app settings,
AR/EN RTL control center/wizard, local JSON and audit. Both MR and Guardian consume
this engine. Guardian reuses backend/readiness policy and disables Sui initialization.

VERIFIED: no-manager API33–36 run 40126853530; official/Next real UID-2000 Binder,
consent, death and Standard fallback run 40126853544; Guardian run 40126176694
(80 UI cases on each API33/34/35 plus build/lint/unit/signature/backend).
Core 47 assertions, app/library 12 JUnit tests. Real AR/EN/live-manager screenshots
and CI APKs are available. Read TEST_RESULTS.md and docs/npse/COMPLETION_EVIDENCE.md.

PENDING EXTERNAL: actual OPPO/ColorOS, wireless pairing, physical permission
revocation, reboot/restart and OEM battery behavior; no authorized device is
connected. No automatic persistence, root, final release or 60/60 claim. Main
is unchanged. USB/NetScan Android integration is not applicable to their current
Windows products; do not create unsupported adapters or duplicate companions.

Next: use SHIZUKU_INTEGRATION.md's owner-device checklist when a device is
available; record results without replacing emulator evidence. Avoid redoing
completed engine/Guardian implementation. Other candidates need a concrete
benefit and separate adoption scope.

---

# Current repair/USB priority — 2026-10-24

New forensic features are paused by the current user directive. Preserve existing
forensic code and critical regression/security coverage. Resume the active
`feature/usb-doctor-intelligence` slice from PR #9 head
`2b0a83a28c1ca6b11db85c6753da935a150e9fd0` (all seven native CI jobs green in
40017343409; Python/public-history also green; Windows portable/installer exist).

USB Doctor adds a pure interpretation engine, timestamped discovery outcomes,
strict PnP inventory validation, target-scoped problem-code explanations and a
real AR/EN panel. Windows inventory no longer requires a detected phone. No
physical-cause, interface binding or commercial model acceptance is inferred.
See docs/device-services/USB_DOCTOR.md and ELITE_LAB_SCOPE.md.

Local connection tests 42 passed; UI 38 per profile without QML warnings.
Both profiles completed all 32 CTest suites after relinking one empty local
executable per profile (initial failures and reruns preserved in evidence).
Hosted exact-commit Windows/Linux CI is the next gate before promotion. Next: device field provenance, firmware risk analysis, OEM adapter
contracts, then an isolated training simulator. Do not resume the older forensic
feature backlog below while the strategic freeze remains active.

---

# Current UI rebuild checkpoint — 2026-10-23

Resume `feature/professional-workstation-ui`. The new 12-section Qt/QML shell,
responsive dashboard/device operations, shared cards/tokens and 24 licensed color
SVG assets are implemented. Current acceptance/evidence: `docs/ui/REBUILD_ACCEPTANCE.md`.
Native services/policy remain authoritative; no main merge or Stage 60 claim.

Local production CTest 31/31; final UI regression 37 passed with zero QML warnings.
Laboratory CTest also passed 31/31, UI 37 with zero QML warnings.
Review PR #9 is published; exact-source Windows CI is in progress.
A focused follow-up distinguishes authorized/unauthorized/offline/Fastboot target
states in the header and covers all four asynchronous engines.
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
