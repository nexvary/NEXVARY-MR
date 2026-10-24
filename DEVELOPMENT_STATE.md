# Local identity and independent-review increment — 2026-10-24

Active branch: `feature/law-enforcement-case-foundation`; PR #11.
Last published source: `8034c4d6e41a120de38f353465687cc62689765a`.
Native CI 40096699282 passed both Linux profiles (38/38 each, production sanitizers
7/7), both OS core jobs and tracking-server. Windows production CTest passed 38/38,
but portable packaging failed because this increment added a redundant copy from
the wrong license source path. The notice already resides in packaging/licenses
and is copied by the existing wildcard. The following packaging fix checks that
the resulting notice exists instead; no C++/QML behavior is changed. New exact-source
CI and artifact evidence must pass before a Windows binary is presented.

## Implemented in this increment

- Windows DPAPI local identity registry, user-chosen administrator setup, Argon2id
  credentials, per-user password-wrapped Ed25519 keys, roles/account enable/disable,
  persisted lockout, monotonic session expiry and password rotation/session revocation.
- Case ownership is atomic in the protected case payload; other users need explicit
  grants plus the existing native role policy. Default production investigator access
  was removed. Sign-out/revocation clears cached case views. Legacy cases need grants.
- Authenticated author signatures and a different local reviewer signing the original
  public/synthetic artifact hash. No self-review, mutable signed review or automatic
  Lab Verified/commercial capability promotion.
- Signed case audit anchors exported with consent. A separately retained newer anchor
  detects an older restored database; tests also cover matching newer history.
- Real Qt account administration/sign-in/password/grant UI, signed research review UI
  and retained-anchor comparison. Native and QML test fixtures use synthetic credentials.
- Scoped primary-source review for Qualcomm/QTEE, Knox, OEM/SoC bulletins and MediaTek
  Genio. Smartphone MediaTek architecture stays unverified; no IoT-to-phone inference.
- Windows packaging includes the pinned official sodium DLL and ISC notice; Linux CI
  includes identity tests in the address/undefined-behavior sanitizer run.

## Validation and limitations

Local Release validation: 37/38 groups passed in the full run; GUI smoke could not
start because its generated executable had become empty. Regeneration plus the
separate smoke retry passed (same source). Identity suite: 10/10; QuickTest: 41/41;
Python root: 29/29. Exact-source hosted Windows validation remains pending below. An initial generated executable was empty/lost its execute bit in this workspace;
regenerating that build artifact is required before treating a BAD_COMMAND as source
failure. Initial compiler diagnostics and test failures are retained in evidence.

Local accounts are not agency-verified identities/SSO. Separate local accounts are not
proof of independent people, local signatures are not trusted external timestamps,
and restoring the identity registry plus every retained anchor can evade rollback
comparison. No phone or independent Windows machine is connected; no physical-device,
trusted Authenticode or final commercial/institutional acceptance claim is made.

Next: finish exact-source native CI/package validation, then obtain real owned-device,
clean Windows 10/11 and independently reviewed release evidence. External identity,
key/time custody, trusted publisher certificate and institutional review require their
actual providers/materials. See docs/LOCAL_IDENTITIES.md for operating instructions.

Resume: `git fetch origin && git switch feature/law-enforcement-case-foundation`;
prepare libsodium with `python scripts/prepare_sodium.py --destination <prefix>`, set
`MR_SODIUM_ROOT`, configure GUI/update verifier/tests with CMake, build and run CTest.
The detailed historical checkpoints below remain evidence of earlier source revisions.

---

# Verified source checkpoint — 2026-10-24

Active branch: `feature/law-enforcement-case-foundation`; PR #11.
Last tested source: `c892faee4f4309ec074bfda64762cde28aed9da9`.
The documentation-only commit containing this final record does not change any
compiled source, QML, catalog, CMake or packaging input from that tested source.

## Completed and verified

- Native unified Law Enforcement and Research Center implementation is published.
  Case create/search/reopen, devices, protected evidence, declared custody,
  linked audit, native role policy, access assessment, training, AR/EN PDF tables,
  source-backed research catalog/intake/results/validation, and shared engine context.
- Exact-source native CI: all seven jobs passed in run 40092232679:
  https://github.com/nexvary/NEXVARY-MR/actions/runs/40092232679
- Windows production CTest: 37/37; Windows Laboratory: 37/37. Core on both OSes,
  both Linux Qt profiles, configured TSK/ClamAV gates, production sanitizer/TLS
  integrations and tracking-server passed.
- Both Python workflows and both public-history checks at c892fae passed.
- Windows portable and installer were built. Isolated portable launch, installed
  application launch, installer registration and uninstall lifecycle passed.
- Real AR/EN runtime screenshots exist in the Windows validation artifacts;
  visually reviewed Linux workspace captures are committed alongside this record.
- Native synthetic case PDF was rendered and reviewed on all three pages;
  full evidence SHA-256 was checked in extracted text. It contains no real case data.

Windows installer ZIP (unsigned development build):
https://github.com/nexvary/NEXVARY-MR/actions/runs/40092232679/artifacts/12558819701
Portable ZIP (unsigned development build):
https://github.com/nexvary/NEXVARY-MR/actions/runs/40092232679/artifacts/12558784752
Windows runtime/installer/screenshot validation:
https://github.com/nexvary/NEXVARY-MR/actions/runs/40092232679/artifacts/12559885974
These provider artifacts currently expire 2027-01-22; metadata/digests are retained
in `evidence/unified-2026-10-24/verified-ci.json`.

## Remaining acceptance gates

This is a tested development increment, not a final institutional/commercial release.
Authenticated agency identities and role provisioning, case-level multi-user grants,
independent researcher review/promotion, external audit/custody anchoring,
owned-phone acceptance and independent clean Windows 10/11 device validation remain.
Current roles are native local action policy, not deployed agency identity proof.
No protected-device extraction, PIN/FRP bypass, decryption or lab compatibility
claim was added. No main merge or final-release gate is claimed.

Next task: authenticated local/agency identity provisioning, then independently
reviewed experiment promotion linked to exact device/firmware evidence and native
capability gates. Continue repair improvements through the existing engines.

## Resume

```sh
git fetch origin
git checkout feature/law-enforcement-case-foundation
git status --short --branch
python3 scripts/validate_research_catalog.py
python3 -m unittest discover -s tests -v
cmake -S . -B build -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Read `docs/UNIFIED_WORKSPACES.md` and `verified-ci.json` before changes. Recheck
remote HEAD and existing changes. Previous failures and remediation are retained;
the initial local post-report run had two execute-bit errors in generated ELF
binaries, and both reruns passed after restoring those bits. Optional external
ClamAV/TSK subcases were skipped locally but passed in configured hosted jobs.

---

## Earlier checkpoints (historical)

# Active master directive — unified workspaces, 2026-10-24

This checkpoint supersedes the older forensic-feature freeze below. The current
user directive authorizes repair + Law Enforcement + Advanced Mobile Research
inside one product. No main merge or final-release acceptance is claimed.

Active local branch: `feature/unified-law-research`.
Publication target: existing PR #11, `feature/law-enforcement-case-foundation`.
Last published implementation: `4beff9ee8d1e4cac335d008c447f94ef5418fd06`.
Last published Windows UTF-8 fix: `ff2196a7aa08038d120dea6274549cbb7b4712de`.
Last published layout/source checkpoint: `db373b7b8a09652b3c07cd65f33982a429daa4bb`.
Last Windows compiler workaround: `37ec1e7e3550a19b306d7fe5e0a42cbe090dcfdc`.
Next source increment replaces raw PDF JSON with labelled AR/EN tables. Native
case tests after that change: 9 passed; a three-page synthetic PDF was rendered
and visually checked, including repeated table captions and complete SHA-256.
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
