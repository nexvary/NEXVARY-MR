# Government Operations Phase 2 — verified retention increment (2026-10-25)

- Current Branch: `feature/government-operations`; draft/open PR #15.
- Current HEAD SHA at hosted executable validation: `8d32b4589cca586a659c5df32dc555faea060740`.
  This later checkpoint changes only documentation/test evidence; resolve its actual
  receipt HEAD with `git rev-parse HEAD`. No compiler/QML/build/packaging input changes.
- Latest Verified Executable SHA: `8d32b4589cca586a659c5df32dc555faea060740`.
- CI Status: https://github.com/nexvary/NEXVARY-MR/actions/runs/40181030127
  All seven native jobs passed. Windows production/laboratory and Linux
  production/laboratory each passed 40/40 CTest; production ASan/UBSan 9/9.
  Python 40181030140 and public-history 40181030130 passed. Portable isolation,
  installed-in-place launch and installer install/uninstall passed. Intentional
  production-disabled laboratory subcases are not counted as executed lab tests.
- Completed Government Capabilities: existing cases/devices/protected evidence,
  native RBAC/ownership/grants, local identities/password rotation/revocation,
  signed research reviews/audit anchors, SHA-256/custody, protected backup/restore,
  PDF reporting and AR/EN UI are preserved. New native immutable policies,
  assignments, legal holds/preservation exceptions, reviewer restriction release,
  UTC expiry assessment, protected-in-place archive receipts/integrity checks and
  separate-account disposal request/review workflows are implemented and tested.
  Retention data survives backup/restore and appears in reports. No bytes are deleted.
- Remaining Government Capabilities: physical disposal execution remains a separate
  reviewed implementation; authorized document/unencrypted-backup/SQLite/JSON/XML/
  CSV/log adapters, unified searchable timeline, provenance relationships and
  enforced offline network policy are next. Further device/research/training
  integration, institutional review/search/report/deployment/identity/recovery and
  Windows RC acceptance remain. Existing PCAP/EXIF/research/training are preserved.
- Known Limitations: protected-in-place archive only, no cold repository/WORM;
  local OS clock and local accounts are not externally trusted time or proof of
  independent people. Existing DPAPI/identity-registry/backup/anchor/size limitations
  remain. Bounds and disposal distinctions: `docs/EVIDENCE_RETENTION.md`.
- External Acceptance Gates: owned exact-firmware phones, independent clean Windows
  10/11, agency identities/SSO/independent reviewers, trusted publisher signing,
  external key/time custody, independent records/privacy/licensing/recovery review
  and release authorization. Hosted Windows Server tests do not satisfy these gates.
- Next Executable Task: implement the first bounded authorized artifact adapter on
  hash-verified separate working copies with parser isolation; then timeline,
  relationships and policy-enforced offline networking. Do not duplicate existing
  parsers, research, identity, custody or backup engines.
- Resume Commands:

```sh
git fetch --all --prune
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
git diff --stat origin/integration/gate-1-stages-08-60...HEAD
# Prepare Qt and pinned libsodium with scripts/prepare_sodium.py, then:
cmake -S . -B build -DMR_SODIUM_ROOT=/path/to/prepared/sodium -DMR_BUILD_GUI=ON -DMR_BUILD_UPDATE_VERIFIER=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
python3 -m unittest discover -s tests -v
```

Windows development packages from that verified source (unsigned):
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40181030127/artifacts/12593958595
- Installer + SHA256SUMS: https://github.com/nexvary/NEXVARY-MR/actions/runs/40181030127/artifacts/12593653834
- Production runtime/installer evidence: https://github.com/nexvary/NEXVARY-MR/actions/runs/40181030127/artifacts/12594008536
Provider artifact digests/expiry and actual job steps: `evidence/retention-2026-10-25/hosted-ci.json`.
These archive digests are not claimed as inner ZIP/EXE checksums.

Local evidence: strict Release build; retention 9/9, identity 12/12, case 17/17,
Qt Quick 44/44, both DPI profiles, Python 35/35, ASan/UBSan 3/3 (no local leak
inspection), pinned Gitleaks history/tree/exception regression. Local full CTest
was 38/40 due to two generated ELF outputs losing execute permission; fixing
only those output modes and rerunning those two groups passed 2/2. Raw failed
runs and retries are retained, never relabelled as a single passing full run.
The independent hosted source passed all four 40/40 profiles without that failure.
Real AR/EN Qt captures were inspected. Separate 1180/1366 viewport QA passed 3/3.
See `evidence/retention-2026-10-25/VALIDATION.md` for reproduction and diagnostics.

Branch review: PR #11 remains draft/open at `8fb9b38`, based on integration;
PR #15 remains draft/open against that case foundation. Research Program, USB
Doctor and professional UI are ancestors of the government base. NPSE remains
separate; no blanket merge, Guardian/Android-agent modification or main merge.
Historical checkpoints below do not override this verified current summary.

---

# Protected case backup/restore increment — 2026-10-25

Branch: `feature/government-operations`; review PR #15. Hosted-verified executable
source: `6d9e4c9a540b156b592dc3d10831deabad19877e`; CI 40160538371 passed all seven
jobs: Windows production/laboratory and Linux production/laboratory each 39/39,
plus 8/8 production sanitizer tests. Python and public-history checks passed on
both push and PR. This checkpoint changes only documentation/evidence after that
source; the packages below are explicitly built from the verified executable SHA.

Implemented: bounded protected case/evidence snapshot via the existing Windows
DPAPI provider; independently supplied SHA-256 on restore; original ownership/grant
checks; full audit/device/evidence validation; no overwrite; atomic case+blob SQL
restore; recovery audit records; AR/EN reports controls and retained backup digests.
Backup is permitted on suspended/closed/archived cases without enabling case edits.
The snapshot predates its source backup-export audit event. A recovery appends its
own event and can diverge from a newer retained source anchor; no silent acceptance.

Source files: `cpp/src/LawCaseBackup.cpp`, LawCaseStore/Bridge interfaces,
LawPermissions, native case/identity tests, `qml/LawEnforcementPage.qml`, UI fixtures,
CMake DPI regression and Windows guide packaging. No Guardian/Android agent change.
Limits: 32 MiB combined protected payloads, 64 MiB container, 100 recorded exports.
The identity registry/signing keys and OS recovery provider are not bundled. This is
case recovery under a supported original protection context, not generic cross-host
or full account/OS disaster recovery. Read `docs/PROTECTED_CASE_BACKUP.md` first.

Native case tests: 17 passed; identity: 12 passed; full Qt UI: 43 passed. Build is
Release with warnings treated as errors. Real AR/EN screens were inspected. Native
Qt generated a five-page synthetic Arabic PDF: all pages were visually reviewed,
and independent pdftotext extraction confirmed exact ASCII digests, including `ff`.
The PDF fix applies a fixed font without shaping only to hashes/UUIDs; Arabic text
keeps shaping. Python: 35 passed; pinned Gitleaks current tree and the real-scanner
allowlist regression passed. Full final CTest/DPI results are retained alongside
raw failures under `evidence/government-backup-2026-10-25/`.

Earlier full regression reached 35/39 with four BAD_COMMAND results because two
valid generated ELF binaries had lost execute permission. Restoring execute bits
on those build outputs yielded 4/4 affected groups passed; no source workaround.
An intermediate strict-test build failed misleading-indentation and was corrected;
tests that ran against stale outputs before its rebuild are explicitly preserved
as before-rebuild evidence, not accepted final validation. Final source rebuilt
successfully and case/identity tests passed again. The final full run passed
39/39 CTest groups, including both DPI scales and native launch. Hosted exact-source
CI also passed as recorded above and in `hosted-ci.json`.

Windows delivery from that source (unsigned development builds):
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40160538371/artifacts/12586018810
- Installer and inner-package SHA256SUMS.txt: https://github.com/nexvary/NEXVARY-MR/actions/runs/40160538371/artifacts/12586558227
- Native/installer validation: https://github.com/nexvary/NEXVARY-MR/actions/runs/40160538371/artifacts/12586723040
Hosted isolation, installed-in-place launch and install/uninstall checks passed.
Artifact archive digests and expiry (2027-01-23) are in `hosted-ci.json`; those are
not claimed to be the inner ZIP/EXE digests. No physical Windows 10/11 result inferred.

Next: native retention and
archive policy enforcement, document/authorized-backup/SQLite case adapters,
searchable timelines/relationships and policy-enforced offline mode. External
Windows 10/11/phone acceptance, institutional identity/key/time custody, independent
privacy/license/recovery review and release sign-off remain unpassed. No main merge
or final institutional release is claimed.

---

# Report and history-check follow-up — 2026-10-25

Branch: `feature/government-operations`; PR #15.
Published first implementation: `0b9bc08d1ac70a81f94c4305eefffdefd63ca793`.
All seven native jobs passed in CI 40145064984: Windows production/laboratory and
Linux production/laboratory passed 39/39 CTest groups each. Linux sanitizer gate
passed 8/8, including the case workspace. Windows portable isolation, installer
install/run/uninstall and Python passed. Artifact hashes/expiry are recorded in
`evidence/government-2026-10-25/first-native-ci.json`.

This follow-up fixes PDF presentation: Arabic labels for analysis/export records,
LTR pretty-printed technical JSON/digests, table captions across page breaks and
compact padding. The application itself generated a synthetic four-page Arabic
report containing analysis and working-copy history; all four pages were rendered
and visually inspected. Complete source/report/manifest hashes were verified in
extracted text. `pdf-visual-qa.json` records the check, and native case tests now
exercise PDF export after PCAP and EXIF analysis. No parser/acquisition capability
or protected-store schema is changed by this follow-up.

Public-history 40145065000 reported seven existing generic-key findings in the base
commit's Windows sodium header-digest receipt. They were independently matched to
public prepared headers with CRLF. Exact historical fingerprints plus exact public
values AND the single receipt path preserve all default detectors. The real pinned
scanner accepted the receipt, rejected seven changed values and rejected copied
values at an unreviewed path. Full history/current-tree scans passed locally. The
history workflow now runs that regression before scanning. See
`docs/GOVERNMENT_HISTORY_SCAN_REVIEW.md`; no secret is removed or scanner disabled.

Local follow-up: seven focused case/identity/interop/UI/DPI/smoke groups passed;
final case 13/13 and root Python 35/35 passed. Final PDF source has a separate native
case rerun and visual/text check. Exact-source hosted CI and replacement Windows
artifacts for the commit containing this checkpoint are pending publication.

Next implementation remains protected bounded case backup/restore, then retention
and document/backup/SQLite adapters. Restore must retain original case ownership,
require a current grant/identity registry, reject duplicate/overwriting cases,
validate every protected evidence blob, and commit atomically. Test malformed,
wrong-account, absent-blob and rollback-anchor scenarios before exposing controls.
Keep policy-enforced air-gap mode, independent Windows 10/11/owned phones,
institution identity/time/key custody and final release acceptance as separate gates.

---

# Government operations checkpoint — 2026-10-25

Active branch: `feature/government-operations`.
Base: PR #11 / `8fb9b38f771da91c60f3b85a2084868b16fe95d4`.
This directive makes Government/Law Enforcement the primary workspace, Research
second and repair a preserved shared-engine workspace. Guardian is independent;
no Guardian code or repository is changed. Main is unchanged.

## Implemented this batch

- Default government AR/EN navigation with twelve sections, repair workspace switch,
  distinct evidence/custody/audit views, stable legacy page APIs and history/back.
- Native independent ExportEvidence/AnalyzeEvidence permissions plus existing
  authenticated local identities and case grants; revoked grants fail before export.
- Hash-verified, bounded working-copy packages with manifest SHA-256 retained in
  protected case history/audit. No original mutation, overwrite or file execution.
- Offline standard-library verifier, native/Python interoperability gate, corruption,
  path/schema, stale-writer, closed/archive and permission regression coverage.
- Case-bound PCAP/EXIF analysis through existing parsers, deterministic report hashes,
  protected retained provenance and PDF inclusion. The government analysis route no
  longer opens the old repair/recovery case collection. No legacy parser expansion.
- Device assessment provenance fields and unknown/reported/risk boundaries; no
  inferred key availability or physical-device extraction claims.
- Windows packages include the verifier and government operating guide. Native CI
  extends sanitizer coverage to the case workspace and government UI to DPI checks.

Main changed paths: `cpp/src/LawCaseBridge.*`, `LawCaseStore.*`,
`DeviceAssessment.hpp`, `cpp/include/nexvary/LawPermissions.hpp`,
`qml/Main.qml`, `AppShell.qml`, `GovernmentAnalysisPage.qml`, Law evidence/custody
views, native identity/case/UI tests, independent verifier/interop/Python tests,
CMake, Windows packaging and native CI sanitizer/DPI gates.

Changed paths and limitations are documented in `docs/GOVERNMENT_OPERATIONS.md`;
remote branch/PR audit and raw test evidence are under
`evidence/government-2026-10-25/`. The implementation commit containing this top
record is the source checkpoint; exact hosted SHA/results follow after publication.

## Current validation

Native Release warnings-as-errors build. Case suite 13 passed; identity 11 passed;
Qt Quick 42 passed; root Python 35 passed; research catalog validation passed.
Initial full CTest: 38/39 passed; packaged GUI smoke found an omitted resource entry
for the new analysis page. CMake now includes that page. The corrected build passed all five focused interop/UI/DPI/smoke groups.
Presentation polish passed four UI/DPI/smoke groups again, and the final government
workflow passed its focused runtime check. Initial XML is retained, never reported
as a successful complete run. Native/Python package interoperability passed.
The final idle-government-shell visibility change passed its focused government UI
test and a fresh compiled application smoke (1/1). Local optional external-tool and
production-disabled laboratory subcases are listed in `local-skips.txt`; they are
not represented as executed hardware/integration tests.
Hosted exact-source Windows production/laboratory, Linux both profiles and sanitizer
checks must pass before presenting new Windows packages. Earlier baseline Windows
artifacts do not contain this batch.

## Remaining implementation and acceptance

Next: protected bounded case backup/restore; retention/archive/deletion policy;
document-specific attachments; authorized Android backup/SQLite analysis adapters;
case indexing/searchable analysis timelines and relationship views. Existing synthetic
training/research remain present; no exact-device support is promoted by synthetic data.

External acceptance: independent Windows 10/11 clean hosts, owned exact-firmware
phones, institution-verified identities/SSO/people, external key/time custody,
trusted-publisher signing, deployment recovery/privacy review and release sign-off.
Local account signatures and hosted Windows Server tests do not satisfy those gates.
This remains a development evaluation increment, not a final field-certified release.

Resume: fetch this branch, inspect actual HEAD/PR/CI and this evidence, then follow
`docs/GOVERNMENT_OPERATIONS.md`. Configure the existing pinned Qt/libsodium build;
run CTest and root Python. Preserve original protected cases and the separate identity
registry; exported unencrypted working copies require an approved destination ACL.

---

# Current validation checkpoint — local identities and signed reviews

Active branch: `feature/law-enforcement-case-foundation`; PR #11.
Latest executable source: `03da82d981e4e192b20dcf12af4aeddac16d71e0`.
Native CI: https://github.com/nexvary/NEXVARY-MR/actions/runs/40098038007
All seven native jobs passed at that exact executable source. Windows production
38/38 and Laboratory 38/38; Linux production/laboratory 38/38 each; production
sanitizers 7/7 including identity; both OS core jobs, tracking-server and configured
TLS/TSK/ClamAV integration passed. Both Python/public-history workflow events passed.
Portable isolation plus installed launch and install/uninstall lifecycle passed.
Downloaded Windows validation archives matched their GitHub SHA-256 metadata;
real production AR/EN identity screens were visually reviewed. The following
checkpoint changes documentation/evidence only, not compiler/QML/build inputs.

Installer ZIP (unsigned development build):
https://github.com/nexvary/NEXVARY-MR/actions/runs/40098038007/artifacts/12562251680
Portable ZIP (unsigned development build):
https://github.com/nexvary/NEXVARY-MR/actions/runs/40098038007/artifacts/12562251671
Windows production validation:
https://github.com/nexvary/NEXVARY-MR/actions/runs/40098038007/artifacts/12561559735
Artifact metadata/digests/expiry and job steps are retained in
`evidence/identity-2026-10-24/verified-ci.json`.

Implemented: user-chosen local administrator setup, authenticated local roles,
case ownership/grants, account disable/revocation, password rotation and session
expiry; author/reviewer Ed25519 attestations over public/synthetic evidence;
retained signed case audit anchors with explicit rollback comparison; native Qt
account/review/anchor controls; scoped reviewed architecture references; pinned
Windows crypto runtime/notices. Repair/workshop and shared device/USB/firmware
engines remain in the unified application. Agency SSO is not supplied.

Local evidence: original 37/38 all-suite run plus regenerated GUI-smoke retry 1/1,
identity 10/10, QuickTest 41/41 and Python root 29/29. Following screenshot review,
03da82d fixed language-change section selection; all four existing UI/scale/smoke
groups passed again. Corrected AR/EN real screenshots and initial diagnostics are
retained under `evidence/identity-2026-10-24`.

First native CI 40096699282 passed native tests on both Windows/Linux profiles,
including production identity sanitizers; Windows portable packaging failed at a
redundant license copy. 1f4b213 corrected the path/check; 03da82d also includes the
bilingual selector fix. The superseded intermediate run was cancelled by the
workflow concurrency policy; it is not claimed as a successful final run.

## Remaining acceptance and continuation

No physical phone or independent Windows workstation is connected. Remaining gates
are owned-phone repeatability with exact model/OS/firmware, clean Windows 10/11,
institution-verified identity/SSO and independent people/review, external key/time
custody, trusted publisher Authenticode, dependency/privacy/recovery review and
release sign-off. Local credential signatures/clock labels do not establish these.
Research approval never grants a commercial capability or Lab Verified phone entry.
`release-candidate-evidence.json` / `release-gate.json` record the unfulfilled gates;
no final institutional/commercial release or main merge is claimed.

Next work requires those actual providers/devices/review materials. Continue safe
repair improvements through the shared engines and collect exact-device evidence
rather than assigning untested support. See `docs/LOCAL_IDENTITIES.md` for local
account recovery, grants, reviewer and retained-anchor trust boundaries.

Resume (Windows PowerShell or developer shell, with Qt on PATH):

```sh
git fetch origin
git switch feature/law-enforcement-case-foundation
git status --short --branch
python scripts/prepare_sodium.py --destination build/sodium
cmake -S . -B build -DMR_SODIUM_ROOT=build/sodium -DMR_BUILD_GUI=ON -DMR_BUILD_UPDATE_VERIFIER=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
python scripts/release_gate.py evidence/identity-2026-10-24/release-candidate-evidence.json --commit 03da82d981e4e192b20dcf12af4aeddac16d71e0
```

The release gate is expected to fail while independent/physical gates are pending.
Historical checkpoints follow; this top record is the continuation reference.

---

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
