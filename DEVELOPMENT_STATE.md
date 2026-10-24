# Active master directive — unified workspaces, 2026-10-24

This checkpoint supersedes the older forensic-feature freeze below. The current
user directive authorizes repair + Law Enforcement + Advanced Mobile Research
inside one product. No main merge or final-release acceptance is claimed.

Active local branch: `feature/unified-law-research`.
Publication target: existing PR #11, `feature/law-enforcement-case-foundation`.
Last published implementation: `4beff9ee8d1e4cac335d008c447f94ef5418fd06`.
Last published Windows UTF-8 fix: `ff2196a7aa08038d120dea6274549cbb7b4712de`.
Last published layout/source checkpoint: `db373b7b8a09652b3c07cd65f33982a429daa4bb`.
Research foundation: `372b9636fce50a66e64360eb315f12ecdfff8c45` (PR #12).
Shared USB/Device Intelligence source matches `9a4392a37b85186032ded8cdbe254b2064d8ed9b`.
The implementation commit containing this checkpoint is resolved with `git rev-parse HEAD`;
exact hosted commit/results will be recorded after publication.

## Implemented

Case SQLite/DPAPI storage, create/search/reopen/edit, device registration,
bounded protected evidence import, SHA-256 verification, declared custody,
linked audit, native role/action policy, consented PDF and synthetic training.
Real Qt AR/EN Law Enforcement and Research Center pages; source-backed Atlas/tool
catalog, compatibility/result intake, hashed evidence, pending-review classification,
atomic local research ledger, live Device Intelligence/USB/Firmware integration.
Package provenance and unified compatibility schema; CMake/Python/Qt CI gates.
Read `docs/UNIFIED_WORKSPACES.md` for trust boundaries and acceptance gaps.

## Validation at this checkpoint

- Native Law workspace: 9 passed; Research: 9 passed (includes forged promotion
  with a recomputed checksum), warnings-as-errors Release build.
- Full Qt Quick suite: 40 passed. Final Law/Research + responsive layout slice:
  4 passed, with genuine AR/EN captures.
- Initial full CTest: 33/34; only GUI smoke failed on a Law page polish loop.
  Fixed viewport/content sizing; standalone application smoke now exits 0 without
  QML warnings. Initial failure and final focused evidence are retained.
- Root Python: 29 passed; recovery Python: 19; portal: 6; original package: 2.
- Final local Release/warnings-as-errors CTest: 37/37 passed, including signed
  updater suites and GUI smoke. Qt Quick: 40 passed. Optional external ClamAV/TSK
  fixture subcases were skipped locally; their configured hosted integration gates
  remain required. At db373b7 both Linux Qt profiles and external gates passed. Windows Qt
  compilation failed on C4702 in generated Qt 6.8.3 templates; a pinned-SDK
  bytecode compiler workaround is included in the next source checkpoint.
  Exact-source Windows packaging after the workaround is still required.
- Laboratory local role/case/research focused CTest: 3/3 passed. One malformed
  zero-header generated ToolBridge object was recompiled; no source fix was needed.
- Root/public-history CI at db373b7 and Windows/Linux core jobs passed.
- Qt SDK was transient between turns; dependencies are now reconstructed outside
  the repository. Keep source/evidence checkpoints in Git, not `/tmp`.

## Remaining gates and next action

Finish full regression; publish tested tree to PR #11 with an expected-head lease;
inspect Windows/Linux jobs and repair evidence-backed failures; retain genuine
Windows captures and installer artifacts. Then implement independently reviewed
research-result promotion and authenticated institution-role provisioning.
Owned-device acceptance, external audit anchoring and institutional accreditation
remain separate unpassed gates. No connected Windows desktop/physical phone exists
in this session; use hosted Windows CI and do not claim a hardware test.

## Resume commands

```sh
git fetch origin
git status --short --branch
python3 scripts/validate_research_catalog.py
python3 -m unittest discover -s tests -v
cmake -S . -B build -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

---

## Historical checkpoints (superseded where they conflict)

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
