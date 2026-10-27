# Case-bound prepared intent / restart reconciliation — 2026-10-27

- Branch `feature/government-operations`; inspected base HEAD `14559c711a4c7801156c54680f2e67472e02170b`, executable parent `62251a02a37b560fafc92f892ce8b51764d5dc82`. The commit containing this entry is the new source checkpoint; resolve with `git rev-parse HEAD`. Existing PR15 stays open/draft; main, other branches and Guardian unchanged.
- Implemented first transaction adapter for **authenticated local classic-PCAP/JPEG-EXIF case analyses** through existing parsers/stores/journal. Strict signed prepared intent binds operation/invocation/case/evidence/analysis UUIDs, revision, original-case/source/report digests, registered action, requester/role/UTC and recovery state. Actual case grant/membership/source checks precede preparation and are rechecked before case mutation. Case result + existing audit + operation linkage commit in one SQLite FULL-synchronous transaction. No duplicate engine or general/private content in audit records.
- Startup verifies signed history and reconciles prepared intents with actual protected case/evidence/revision/report bytes: exact unchanged state -> failed local mutation; exactly matching committed result -> completed local mutation; ambiguity -> unknown and workspace blocked for review. No external replay/rollback. Identical terminal retries/restarts add nothing; duplicate/rebound/oversized/extra-field histories fail closed. Scope and conservative unknown behavior: `docs/GOVERNMENT_CASE_TRANSACTIONS.md`.
- Native transaction tests **16/16**, including abrupt exits before/during/after intent write, during SQLite write, after case commit, during/after terminal write; real reopened state, two recovery instances, corruption/foreign evidence/stale revision/identity, post-replace readback refusal and no-duplicate checks. Independent Python-authored signed corpus **51/51**, accepted/rejected by both verifiers. Root Python **42/42**. Release/warnings-as-errors final build passed after local generated-artifact repairs (failures retained).
- Initial full local CTest **43/49**: two Not Run from generated zero-byte executables, three QML groups from missing new policy wiring in the native fixture, and original main audit-export transient. All first failures are retained. Artifact-only relink/mode repairs and matching UI fixture wiring are complete; QML assertions/layout/crash tests are unchanged. A fixture-corrected observation is running; final readback-source full suite is still PENDING.
- **Audit transient reproduced, root OPEN:** actual signed disk prefix18 vs in-memory diagnostic22, with wrapper/signatures verified independently. Private synthetic resources preserved locally; no secret-bearing journal published. New post-atomic-replacement readback equality must succeed before a journal append is acknowledged. Deterministic older-snapshot injection verifies fail-closed refusal; no retry, silently accepted rollback, root-cause closure or power-loss guarantee is claimed. Fixed-stage `audit-readback` diagnosis added.
- Existing Qt/DPI/crash/lock/symlink regressions retained; CI ASan gate now includes new transaction suite. No Qt runtime/layout reimplementation. See `evidence/case-transactions-2026-10-27/LOCAL_FAILURES.md`.
- **New Push/PR CI and Windows packages PENDING**; never transfer old-source test/build receipts. Shipped Windows guides include transaction scope and the updated independent verifier. No new visual/hardware fix or trusted/government-ready release is claimed.
- First-priority remaining work: migrate the older RecoveryBridge partition result path through verified case/evidence membership and prepared reconciliation; its optional text case reference is still insufficient. Scanner/update/other import/export paths do not inherit this adapter. Product process-tree containment, original intermittent audit-store cause, UI soak/performance and original competitive/institutional gates remain open.
- Next: finish/repeat complete local suite after the generated-artifact repair, inspect real runtime AR/EN frames, publish source and inspect Push/PR independently, retain first hosted failures. Then continue the remaining partition transaction migration before process-tree work. Resume: `git fetch origin feature/government-operations`; `git status --short`; `cmake --build build --parallel 2`; `ctest --test-dir build --output-on-failure --output-junit full-results.xml`; `python3 -m unittest discover -s tests -v`.

# VERIFIED reliability mitigation / separate Push and PR acceptance — 2026-10-27

- Current Branch: `feature/government-operations`. **Verified executable/test/workflow source HEAD **`62251a02a37b560fafc92f892ce8b51764d5dc82`****; runtime candidate parent2c45f9f. This following checkpoint is state/evidence only, not shipped runtime/test/workflow/packaging/guide code; resolve its current HEAD with `git rev-parse HEAD`. PR15 stays draft/open on8fb9b38, no main merge/branch deletion/Guardian modification.
- **Push CI40500300494:7/7 success, attempt1. PR CI40500306659:7/7 success, attempt1.** Both Python42/public-history workflows passed too. Actual PR checkout7c8e707c084cc40b189a93e6f2270e2d4c5512b7 has **identical tree b557aaf5684cb3a47cfc03b502ef21c0a713b101** to62251a0; direct source equivalence verified. Native Windows/Linux Production/Laboratory48/48 per profile/event, full QML52 and both DPI13/13; strict GUI3 repeats/profile. Required Linux Production ASan/UBSan17/17 passed in both events with leak checks unchanged. Intentional native platform/profile exclusions remain visible; Windows policy is14passed/1POSIX-symlink skip, not a zero-exclusion claim. Full metadata/excerpts: evidence/reliability-recovery-2026-10-27/FINAL_CHECKPOINT.md and candidate-ci-final.json.
- **Reproduced failure preserved:** pre-mitigation2c45f9f PR40497742489, job122167628437, Linux Production125%: first47/48 with SIGSEGV3.25s after governmentAdbBackupControls; one GDB replay reproduced SIGSEGV255. Full stack/original log/XML retained, independently matched artifactZIP SHA. The old overall PR run was later cancelled by superseding code while its failing job/artifact remained; it is not relabelled green. Original fabe PR40454584507 also remains retained.
- Implemented runtime mitigation: stable Column positioners isolate import findings/retained-analysis delegate destruction from the enclosing layout engine; new four-cycle actual synthetic import/reset/resize/RTL/frame regression in full and both DPI groups. GDB identifies QMetaObject::cast -> QQuickLayout::effectiveSizePolicy_helper -> size/rearrange/updatePolish, not an audit-store error. **Hosted mitigation verified; precise originating delegate/upstream cause not independently proved, no universal crash-free/root-closure claim.** No delays, test disabling, coverage reduction or arbitrary CI rerun.
- Local matching candidate: full48/48(331.22s), QML52, DPI13each, GUI3/3(61.51s); Laboratory UI ASan/UBSan125%13/13(55.95s), local-only detect_leaks=0 for ptrace limits. Required CI leak checks were preserved. Older Release/O3/-g diagnostic3/3 and allocator-perturbation observations are explicitly pre-change and did not close the hosted failure. Full candidate frames inspected AR/EN locally and Windows CI. Historical hardware flicker/top-overlay root remains open; Arabic mixed-label report digest still has bidi fragmentation in the Windows frame, not declared fixed.
- **Verified Windows main EXE SHA-256 **`8c2e6202a6f5f251b5d93379188e7fe9908f32e467f1726a33d7966a16efb89e`****; import-worker92eec59e4c54a1c116907f4ec53060c38f992503e6cc8f75365db0b3b1616b66. Three downloaded providerZIP digests and all1,418 portable manifest entries/full file set checked; BUILD_PROFILE62251a0/production-read-only/unsigned. Exact main/worker bytes matched portable-isolated and installed-in-place CI receipts; install/run/uninstall/cleanup passed. Canonical shipped policy/offline/result guides and independent Python verifier matched source. Inner portableZIPcd683c4c7a0fe2bad64161c1826e438c3ddab172bf3b80ffb29977309aac5f0f; setupEXEb6e3f3b31b5a870f78343a743426375bcd25d5355a4f7c797b9f423a3c1ce1eb. These are file hashes, not commits. First-party CI/archive checks, not physical Win10/11 or institutional certification.
- Audit Store: added lock contention/no overwrite/refusal-latched-until-restart and POSIX/dangling symlink refusal tests. Historical intermittent export failure remains **OPEN, not reproduced or fixed**; no common root asserted with QML or generated-object failures. Preserve original synthetic resources at the next exact-stage failure. No private journals, keys, raw case data or real identities published.
- Remaining critical work: prepared operation intent, real case/evidence linkage and cross-store interrupted/crash reconciliation/idempotency/explicit unknown outcome; product process-tree owned cancellation and independent DNS/proxy/UNC/egress. Detached-descendant counterexamples still stand. Read-only local cgroupv2 probe is not containment. Performance, physical-device/cleanWindows, hardware renderer, trusted signing and institutional/comparative acceptance remain Pending. **No government-ready release or competitor superiority claim.**
- Next Task: maintain the original Qt reproducer/GDB gate and independent stability review; diagnose the next exact audit-store failure with preserved synthetic resources. Where that transient cannot be reproduced, continue prepared intent/case-journal reconciliation through the existing journal, then bounded product descendant cleanup. No new forensic feature expansion before the repairable acceptance blockers.
- Resume Commands: `git fetch origin feature/government-operations`; `git status --short`; `git log -4 --oneline`; `cmake --build build --parallel 2`; `ctest --test-dir build --output-on-failure --output-junit full-results.xml`; `python3 -m unittest discover -s tests -v`. Use the pinned preparation/workflow for fresh dependencies and Release/Laboratory reproduction. Preserve originals before any diagnosis. Do not transfer62251a0 results to changed executable sources.

Unsigned source62251a0 DEVELOPMENT EVALUATION ONLY:
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40500300494/artifacts/12732988421
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40500300494/artifacts/12733083275
- Windows raw/deployment receipts: https://github.com/nexvary/NEXVARY-MR/actions/runs/40500300494/artifacts/12733167963

# Hosted SIGSEGV reproduced / layout-lifetime candidate — 2026-10-27

- Branch feature/government-operations; inspected parent **2c45f9f9368708af16069e591952652f998ed19b**. This following source changes GovernmentAnalysisPage QML and adds a real lifecycle regression to full/DPI suites. Resolve candidate HEAD with git rev-parse HEAD. Main/branches/Guardian untouched; PR15 remains draft.
- **Reproduced failure and stack obtained**: source2c45f9f PR40497742489, Linux **Production**, job122167628437: first full47/48,125% SIGSEGV3.25s after governmentAdbBackupControls. GDB replay reproduced SIGSEGV(exit255), with QMetaObject::cast -> QQuickLayout::effectiveSizePolicy_helper -> grid size hints/rearrange/updatePolish -> waitForRendering. Thus not Laboratory-only. Original fabe PR failure remains retained; no CI retry used to erase failure. Artifact12730958767 ZIP SHA256 independently matched d2e272e4f1b4bc84f4ba75974dd9a987a8f791da69fd6152344c5c0eecd2a0f7. Full stack/original full log/XML/diagnostic receipt retained under reliability-recovery-2026-10-27/; no local-variable/memory/core dump or private journals published.
- Runtime **candidate mitigation, not proven root-cause closure**: import finding and retained-analysis Repeaters now live inside persistent QtQuick Column positioners. The enclosing ColumnLayout sees stable children while clearing/repopulating reports destroys delegate panels. Stack confirms a layout-polish invalid-object path; it does not identify the exact originating delegate or independently prove an upstream Qt defect. No arbitrary delay, test suppression, engine/Qt upgrade or reduced coverage.
- Added behavioral regression: four actual synthetic ADB imports, populated case/report clearing, immediate width/height/RTL changes, required rendered frames and cleared visible status. Added to both DPI subsets as well as full Qt Quick. Initial candidate targeted5/5 passed; updated full125% Laboratory ASan/UBSan group13/13 passed(55.95s), with local-only leak detection disabled due ptrace limits. Required CI leak checks unchanged. Local candidate full48-group run is in progress; do not inherit pre-change48/48/QML51/DPI12 numbers.
- Baseline only: unmodified native Release/O3/-g/GCC13.3/Qt6.4.2 Laboratory125% diagnostic repeat3/3(112.83s), and one allocator-perturbation observation passed before QML change; local failure remains non-reproduced. These passes did not prevent the exact hosted Production failure above. Generated empty object link failures retained/recompiled without runtime-source changes. Local cgroupv2 visible/read-only; no product containment proof.
- Candidate Push/PR CI: **PENDING**. Baseline2c45f9f Python42 and public-history passed in both events; PR native is RED due the stack above. Baseline Windows jobs/results do not verify this QML candidate. New Windows executable/installer byte verification remains pending; no government-ready delivery.
- Audit-store intermittent root remains OPEN separately, despite lock/symlink refusal tests and three GUI observations. Prepared intent/case/evidence linkage/crash reconciliation, product descendants/OS egress, performance and independent institutional/device/Windows field acceptance remain. No competitive superiority claim.
- Next: finish candidate full native/QML/DPI, inspect real candidate frames, verify candidate Push and PR independently. If any original-like crash remains, retain new GDB stack and refine the reproducer; do not label a green isolated retry as a fix. Resume: git fetch/status/log; cmake --build build-lab --parallel 2; ctest --test-dir build-lab --output-on-failure --output-junit layout-candidate-full.xml; ASan group and original commands/evidence below.

# Windows Python regression correction — 2026-10-27

- Branch feature/government-operations; inspected HEAD8c9ac4db2b1301f6b81cbc9b9557f6afb1ce1d14. This following commit corrects only cross-platform unit-test mocking and preserves original failure evidence. Resolve its HEAD with git rev-parse HEAD. Product runtime/QML, native tests, diagnostic helper and workflows unchanged.
- **New root cause established and fixed in the test**: source8c9ac4d Windows Python3.12.10 Push40497317603 and PR40497323726 failed42-test suite because patch.object(os, killpg) requires an attribute absent on Windows. SIGKILL is likewise POSIX-specific. Explicit create=True mocks now supply both APIs in this Linux-helper simulation; all timeout/owned-PID/original-exit assertions stay enabled on Windows. No skipped test or runtime guard weakening. Original traceback: evidence/reliability-recovery-2026-10-27/windows-python-mock-failure.txt.
- Local final Python42/42; native source-a1facce full48/48, policy15, QML51 and DPI12 each remain explicitly source-scoped. Fresh Laboratory ASan125% full12/12 passed(46.02s), detect_leaks=0 locally only; required CI leak checks unchanged. New exact Release/O3/GCC13.3/Qt6.4.2 Laboratory diagnostic build in progress, separate from RelWithDebInfo evidence.
- Push/PR native CI for8c9ac4d in progress; final corrected-test source **Push/PR PENDING**. Do not relabel pending or failed jobs as passed. Verified Windows executable SHA for the final source remains unavailable; historical fabe SHA listed below is not a new-source verification.
- SIGSEGV and historical audit-store transient still BLOCKED/OPEN: no root cause or runtime fix proved. Prepared-intent/case-journal reconciliation, product process-tree security and independent offline/performance/institutional gates remain. No government-ready build. Next: inspect final-source Windows Python and native Push/PR separately, preserve first failures/stack; continue only on verified evidence.

# Serialized reliability verification / owned diagnostic cleanup — 2026-10-27

- Branch `feature/government-operations`; native/test source **a1facce5d8f19b1cd9b24d0ef500401d49da7ba6**, parent45b45bb. This subsequent batch changes only the diagnostic helper/tests, state and synthetic evidence; resolve current HEAD with `git rev-parse HEAD`. Product runtime/QML unchanged from fabe1b4. PR#15 remains draft, no main merge/Guardian changes.
- Native final serialized Laboratory RelWithDebInfo/Qt6.4.2 full **48/48 (330.49s)**; policy **15/15**, QML **51/51**, both DPI groups **12/12**; then strict GUI repeat **3/3 (59.55s)**. No SIGSEGV/audit transient reproduced; **neither root cause found nor fixed**. Original Release flags differ from this local profile. Original failed PR40454584507 remains a historical acceptance blocker.
- Fresh Laboratory ASan/UBSan transition pair **4/4** with local-only detect_leaks=0 due ptrace restrictions; full instrumented125% DPI **12/12**, CTest1/1 (46.02s). Earlier production pair4/4 scoped separately. Required CI leak checks unchanged. GDB15.1 available but ptrace probe refused; no stack. Aggressive-GC exploratory run was intentionally interrupted before init completed (exit130), not a test result. Generated empty main/ConnectionDiagnosis objects caused local link failures; retained/rebuilt, no runtime cause inferred.
- Follow-up helper fix: launch GDB in an owned Linux session; on180s timeout kill that owned process group and bound the wait. Python regression verifies the owned PID/group and original failing CTest exit retention. This is diagnostic-fixture cleanup only; detached product descendants remain **uncontained**. No cgroup/Windows Job Object implementation or process-tree security claim.
- Python **42/42** and diff check passed. Original lock-test fixture failure retained; valid existing rejection action fixes the fixture, not Audit Store. Actual AR/EN native screenshots inspected. Evidence: evidence/reliability-recovery-2026-10-27/; private synthetic resources/keys/journals excluded.
- CI source-a1facce: Push40496007044 / PR40496012564 in progress when this follow-up was prepared; their Python and history checks passed. New helper source requires its own **Push/PR PENDING** verification. Do not transfer old results to this changed helper. Native code/tests are identical to a1facce except Python diagnostic tests; Windows executable/package bytes for the new source are not yet independently verified. Historical source-fabe executableSHA remains bf38b4780fff6ed6102926aa69d27061dcddea35b84dc539f891f252c517af9e.
- Known blockers: original SIGSEGV root/stack; intermittent audit-store root; prepared operation intent and case/evidence binding/crash reconciliation; product process tree and independent offline egress; performance; institutional/physical-device/Windows field acceptance; top-overlay root cause. No release-ready Government build.
- Next: verify new-source Push and PR independently; retain first failures/GDB evidence, then implement proven fixes. If no crash reproduced keep BLOCKED, move to prepared-intent/case-journal reconciliation. Resume with `git fetch origin feature/government-operations`, `git status --short`, `git log -3 --oneline`, `python3 -m unittest discover -s tests -v`; native commands and prior historical source-specific evidence follow below.

# Reliability recovery / diagnostic instrumentation — 2026-10-27

- Current Branch: `feature/government-operations`; inspected remote HEAD `45b45bb5d37a6a5dfdf76cedecb56a965315d538`. Resolve this subsequent batch HEAD with `git rev-parse HEAD`. PR #15 remains draft on the existing foundation. No main merge, branch deletion or Guardian modification.
- SIGSEGV **BLOCKED / root cause unknown**: original PR run 40454584507, job 122024938764 was downloaded again through the GitHub connector. Ubuntu24.04 / Qt6.4.2 / offscreen software / Laboratory / 125%, immediately after `governmentAdbBackupControls`; no original stack/core exists in that job log. Original failure evidence under partition-results-2026-10-27 remains authoritative. Successful push 40454580495 does not clear it.
- Implemented diagnostic code: Linux CI retains Release optimization and adds `-g`; installs GDB. `scripts/diagnose_ui_failure.py` runs the full CTest suite once, preserves its original log/failure list, then performs at most one GDB replay per failed UI/DPI group using its actual CTest command/environment. Replay prints stacks without local variables, uses separate output files, and cannot change the original failing exit. Normal GUI repeat remains behind the successful full-suite gate; no coverage removed or rerun-until-green.
- Regression additions: synthetic audit lock contention / unchanged bytes and anchor / refusal remains latched until restart; existing and dangling POSIX symlinks must never modify/create their target (explicit Windows shortcut exclusion). Initial new lock fixture used an unrecognized synthetic action and correctly failed journal restart verification; corrected fixture uses the existing `policy-rejection` schema. No runtime verifier or expectation was relaxed. Native final regression/full suite in progress at this commit.
- Local observations: restored Qt6.4.2 in an isolated prefix. Production source-fabe transition pair passed 4/4; the surviving source-fabe production ASan/UBSan pair passed 4/4 with leak detection disabled **only locally** because this environment denies ptrace; not an exact Laboratory sanitizer pass, and required CI leak checks unchanged. Fresh Laboratory RelWithDebInfo 125% group passed once; differs from original Release optimization and does not prove a fix. Local GDB15.1 `/bin/true` probe: `ptrace: Operation not permitted`, `No stack`. Exact hosted diagnostic reproduction remains required.
- Local build limitations: first full build failed linking main because a generated main.cpp.o was empty. Empty object retained outside the repo and rebuilt; no runtime source fix inferred. A targeted build was inadvertently started before the prior full build process had fully exited; subsequent full verification is serialized and does not treat that overlapping run as final evidence. Earlier modes/storage diagnostics remain open.
- Python **41/41** passed, including original-exit preservation even after a passing diagnostic, unavailable-tool failure, DPI environment replay and protection of original QuickTest output files. `git diff --check` passed.
- Push CI / PR CI for this batch: **PENDING**, separately required. Verified Windows executable SHA remains historical source-fabe `bf38b4780fff6ed6102926aa69d27061dcddea35b84dc539f891f252c517af9e`; no new current-source Windows executable verified. No government-ready delivery.
- Audit-store historical transient, crash-atomic case/journal linkage, process-descendant containment and independent egress remain OPEN. Viewed the original Windows receipt screenshot: top-edge overlay is visible; no visual fix claimed. No performance or competitor superiority claim.
- Next Task: inspect exact-source Push and PR diagnostics, preserve first failure, obtain usable stack/ASan reproduction and repair only proven causes. Finish serialized native regression, then journal recovery/operation linkage and process-tree lifecycle work. Do not expand forensic features while repairable acceptance blockers remain.
- Resume: `git fetch origin feature/government-operations`; `git status --short`; `git log -3 --oneline`; `cmake --build build-lab --parallel 2`; `ctest --test-dir build-lab --output-on-failure --output-junit reliability-full.xml`; `python3 -m unittest discover -s tests -v`. Qt prefix/dependencies here are temporary; use the pinned preparation script and workflow package list on a fresh host. Preserve original failed runs before diagnosis.

# Current Government checkpoint — reliability blockers OPEN (2026-10-27)

- Active Branch: `feature/government-operations`. Verified executable/workflow source HEAD **`fabe1b469ad0f7092b0654b76bc2e79d69eeeb06`**; partition implementation `0b8cc755116cbdba8fc674d3c5c04e60a978769c`, inspected start b898917. The subsequent evidence-only commit containing this checkpoint changes no runtime, QML, test source, workflow, packaging or shipped guide; resolve its current HEAD with `git rev-parse HEAD`. PR #15 stays draft/open on unmerged PR #11 (8fb9b38). Main unmerged, branches retained, Guardian untouched.
- **Acceptance blocker: PR CI is not all-green.** https://github.com/nexvary/NEXVARY-MR/actions/runs/40454584507 — Linux Laboratory `mr_ui_scale_1.25` crashed with SIGSEGV (3.29 s), last completed subtest governmentAdbBackupControls. Other six jobs passed. Original first failure retained, no job rerun requested. Merge commit 115de8ec164f31d539271f935a81c77b0379213b has the **same tree 2d10f77ed36d1db8d9e8303044486d6226c6f385** as the tested push source. This is not a code-merge difference. No stack/core was uploaded; cause/fix unknown. The evidence-only head intentionally performs no new compilation and never clears this unchanged-runtime acceptance blocker, even if the hosting UI has no new checks on a documentation head.
- **Successful push scope:** https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495 — seven jobs passed, attempt 1, exact source fabe1b4. Windows/Linux Production/Laboratory each **48/48 CTest**, three strict GUI repeats per profile, Qt Quick **51/51** plus both DPI groups, native policy **13/13**, offline **18/18**, separate signed corpus **40/40**, production ASan/UBSan **17/17 groups**. Real installed TSK: Linux **8/8**; Windows **7 passed/1 explicit installed-tool skip**. Other intentional native platform/profile exclusions stay visible in raw artifacts. Python 40454580464 and public-history 40454580507 passed. These results do not override the separate PR DPI failure.
- Verified main EXE file SHA-256 **`bf38b4780fff6ed6102926aa69d27061dcddea35b84dc539f891f252c517af9e`**, import worker **`84e589ffac759beeee6dc9600108cb8743cf1d34c71f8777daf158318767a3e5`** (file hashes, not Git SHAs). Six provider ZIP digests and all **1,418** portable manifest entries, exact source/profile and identical before/after portable/installed tested bytes, independent audit receipts and installer install/run/uninstall/cleanup were checked before local runtime loss. Shipped result/audit/offline guides and Python verifier matched canonical LF source. Inner portable SHA-256 `e5aa9eab37e82769e597c163e885bdd7da92f72e1a635eec67fe162691d55cd7`; setup `cb94961ed19232b4adea53d5768de4128a219212214c7e666afea188cefe6524`. Unsigned development evaluation only.
- Implemented/Tested: third registered result kind, RecoveryBridge authorized local raw-image/mmls partition analysis, alongside existing file scan/signature-update kinds. Actual approval/requester/declared case/category correlation, mandatory source/output SHA-256 and exact exported existing evidence payload bytes. Background candidate waits for callback authorization/cancellation check. Running and queued-completion revoke, cancellation/logout/expiry, case-write/restore failure and corrupt-journal refusal passed. AR/EN native receipt projection is real; its UI fixture deliberately uses a known synthetic digest. No new acquisition/device capability.
- **Observed NOT contained:** controlled marker-only detached analyzer descendants survived parent-worker revocation/return in hosted Windows and Linux fixtures while the receipt was revoked/case candidate refused. Ready/trigger handshakes establish order; ten-second self-exit, no network calls. This concrete process-tree counterexample is not universal OS/network or institutional verification. Fresh local ptrace capability was denied before tracing; DNS/UDP/proxy/complete egress/UNC or mounted shares/browser/export still need controlled independent OS observation and institutional controls.
- Older GUI audit failure remains OPEN separately: implementation source 0b8cc75 local full 48/48 (282.76 s) passed, then repeat **run 2** failed at policy-audit-export (16): healthy=0, authenticated administrator, sequence=70, audit-store. Original logs and a private synthetic copy preceded further runs. New fixed read/digest/seal/symlink/open/permissions/write/replace diagnostics and bounded synthetic-only metadata preserve fail-closed conditions without raw reasons/paths/content/keys or retries. Six isolated later runs passed (three reused, three fresh); no root cause/fix or common cause with earlier artifact setup failures is proved.
- Local completed scope: CMake/Werror, targeted 4/4 (32.08 s), final repaired full **48/48** (290.61 s), UI51 and both DPI groups, Python38/catalog/history/scanner mutation checks. A first final run could not start five groups because three generated ELF test files were 0644 after relink; only modes changed, hashes unchanged, failures retained in completed observations. Earlier dependency mode repair preceded full upstream 101/101. No source/test/pin/expectation weakening or causal GUI explanation. Latest raw local copies may be unavailable after cloud transport loss; completed aggregates and recovered hosted originals are distinguished.
- Additional UI sanitizer investigation is **not a pass**: a local production-debug/Qt6.4.2/125% pair rejected imports and reported LeakSanitizer fatal process-inspection limits. No useful original SIGSEGV stack. A separate address-only alternative had no verified result when the cloud exec-server disconnected. Required CI leak checks were not changed. Local profile differs from the failed laboratory profile. First next task is an actionable exact-profile stack/ASan reproduction on a compatible runner, not retrying the PR to obtain green.
- Limited existing-host observation: exact current portable on one Windows 10 Home 19045/64-bit host, fresh synthetic data, SDK-excluded process PATH, software Qt renderer. Held OS exit **0**, unchanged exact main/worker hashes and native default-deny/revocation passed. Separate Python/cryptography50.0.2 in cached isolated temporary environment verified **23 portal-policy events**; global Python unchanged. No security/partition worker records in that smoke history; CI tests those separately. Anchor same-process, not institutional custody. **75 owned-PID TCP snapshots** saw loopback/no external TCP; transient traffic can be missed and DNS/UDP/proxy/descendant/share/browser/export containment is unproved. No installer/drivers/system-policy changes on the host. Actual owned-window AR/EN frames inspected.
- Remaining: only three kinds; case reference is declared scope, not membership/access/equality with recovery case. Case/journal writes are **not crash-atomic**, restore can fail, interrupted outcome unknown; large-case GUI persistence latency unbenchmarked. Local identity/key/time/anchor/rollback/current-account compromise/capacity limits remain (8,192 events/16 MiB/64 unfinished/32 UI rows). Clean Windows10/11, actual Windows mmls/phones/firmware/OPPO/hardware-renderer/flicker, institutional signing/identity/key/time/reviewer/security/records acceptance remain Pending. Full OS air-gap and protected locked-device extraction are not supported claims. A scripted Windows CI receipt frame has a clipped top overlay; cause/operator reproducibility unknown, no UI fix claimed.
- Competitive gap reduced: one more existing worker has independently checkable approval-to-export correlation and adverse lifecycle/storage checks. No competitor executable/equivalent benchmark, parity/superiority or final government release/accreditation. Existing dated XAMN/Oxygen comparison axes reused without a fresh vendor review; cost/TCO unmeasured.
- Next executable task: capture the PR DPI crash stack on exact laboratory/Qt6.4.2/125% transition; retain failure and investigate init/cleanup/next artifact transition before expanding engines. Also use fixed audit stages on the next reproduced export failure. Then add explicit operation linkage/prepared intent and interrupted case/journal reconciliation; controlled OS boundaries/large-case latency follow.
- Session continuity: cloud execution transport became unavailable during additional diagnosis. Source/tests remain published; this checkpoint recovers verified outputs and selected original reports/captures through GitHub and the scoped test host. No completed address-only run, new local final history scan or raw private resource custody after disconnect is claimed. Earlier draft checkpoint tree scan found no secrets; newly recovered evidence was reviewed to omit raw histories/anchors/keys/private paths. Resume preserves any surviving local evidence copies before updating.

Source-fabe evaluation downloads:

- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495/artifacts/12712258823
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495/artifacts/12712169416
- Windows Production raw reports/deployment/captures: https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495/artifacts/12712263818
- Windows Laboratory: https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495/artifacts/12712915294
- Linux Production including sanitizer report: https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495/artifacts/12712710446
- Linux Laboratory: https://github.com/nexvary/NEXVARY-MR/actions/runs/40454580495/artifacts/12712341242
- **Failed PR laboratory evidence:** https://github.com/nexvary/NEXVARY-MR/actions/runs/40454584507/artifacts/12711844069

Resume (preserve this folder's surviving uncommitted copies before fast-forward):

```sh
git fetch origin feature/government-operations
git switch feature/government-operations
git status --short
git stash push --include-untracked -m 'preserve local diagnostic copies' -- evidence/partition-results-2026-10-27
git merge --ff-only origin/feature/government-operations
git log -4 --oneline
cmake --build build --parallel 2
MR_TSK_MMLS_PATH=/usr/bin/mmls ctest --test-dir build --output-on-failure --output-junit full-results.xml
ctest --test-dir build -R '^mr_gui_smoke$' --repeat until-fail:3 --output-on-failure
python3 -m unittest discover -s tests -v
```

Scratch/dependencies/private snapshots may be unavailable or pruned; restore source from GitHub and inspect pinned dependency checks. Do not inherit a pass across a changed source. Historical checkpoints below are source-specific; their pending labels do not supersede current evidence or clear current blockers.

# GUI storage diagnosis / descendant boundary — follow-up checkpoint (2026-10-27)

- Active Branch: `feature/government-operations`; published partition implementation `0b8cc755116cbdba8fc674d3c5c04e60a978769c` from parent b898917. Resolve the next follow-up source HEAD with `git rev-parse HEAD`. PR #15 remains draft/open on unmerged PR #11. No main merge, branch deletion or Guardian change.
- Reliability OPEN: source 0b8cc75 local full **48/48** (282.76 s), Qt Quick **51/51** and both DPI groups passed; the subsequent GUI repeat stopped on **run 2** at policy-audit-export, exit 16. Context: journal healthy=0, authenticated=1, role=administrator, sequence=70, class=audit-store. This narrows the observed failure to the audit-store path; it does not prove a root cause or the cause of earlier artifact setup failures. Failure XML/console and a private synthetic resource copy were retained before another GUI run. Raw histories/keys/cases are not published.
- Implemented follow-up: preserve fail-closed audit guards and distinguish fixed read/empty, unexpected digest and sealed-write stages (seal/symlink/open/permissions/write/replace). Synthetic GUI failure diagnostics report only allowlisted classes plus exists/regular/symlink/size/opened metadata; no raw error text, paths, reasons, identities beyond allowlisted role, contents, keys or silent retry. No functionality fix or historical-resolution claim. Bounded controlled reuse/fresh GUI observations and new hosted gates are pending.
- Boundary TESTED on local Linux: a first-party marker-only detached descendant remained alive after external-analyzer permission revocation, worker return with a revoked receipt and refusal of the case candidate. A handshake and a trigger written only after worker return order the observation without a sleep guess. Fixture lifetime is bounded to ten seconds; no network calls. This is a concrete counterexample to process-tree containment, not general network or independent institutional certification. The fresh strace capability probe was denied before tracing (PTRACE_TRACEME/PTRACE_SETOPTIONS); DNS/proxy/complete egress/share/browser/export observations still need a trace-capable authorized host and OS controls.
- Latest targeted follow-up: CMake/Werror build; **4/4 CTest** (32.08 s), policy **13/13**, offline **18/18**, real installed Linux TSK **8/8**, independent corpus **40/40**. Controlled original-source full/GUI failure and descendant result are preserved separately. Windows/Linux final-source full suites, ASan/UBSan, repeat/DPI/deployed packages and current executable digests are PENDING; do not inherit the old source-9f packages or Win10 test.
- Partition-result functionality remains the three-kind increment described below: signed approval correlation and exact exported evidence payload digest; final callback rechecks authority before exposure. Case and journal stores are not crash-atomic; best-effort restore can fail, and latency for large-case GUI persistence is not benchmarked. All existing identity/key/time/case-scope/rollback/capacity, external-tool truth, Windows/phone/institutional/signing and competitor-comparison limits remain. No containment, parity/superiority or final acceptance claim.
- Next: finish the bounded diagnostic observations and final-source full/CI/Windows bundle and real capture verification; preserve any failure without retry-until-pass. Then investigate the exact storage stage on a failing runner, cross-store interrupted-result reconciliation and remaining independent offline boundaries before broadening forensic engines.
- Resume: `git fetch origin feature/government-operations`; `git status --short`; `git log -4 --oneline`; `cmake --build build --parallel 2`; `MR_TSK_MMLS_PATH=/usr/bin/mmls ctest --test-dir build --output-on-failure`; `ctest --test-dir build -R '^mr_gui_smoke$' --repeat until-fail:3 --output-on-failure`. Dependency rebuild failure/repair evidence remains below; no source/check/pin was weakened.

Historical source-specific checkpoints follow; pending labels below do not supersede later verified evidence.

# Signed partition-analysis results — implementation checkpoint (2026-10-27)

- Active Branch: `feature/government-operations`; inspected remote parent/current pre-implementation HEAD `b89891700b21dd8ec4151a8182e97fa5bbe1f5d1`. Resolve the subsequent implementation commit with `git rev-parse HEAD`. PR #15 remains open/draft on unmerged PR #11 (`8fb9b38`); main unmerged, other branches retained, Guardian untouched.
- Implemented: extend existing signed operation-result protocol to RecoveryBridge `partition-analysis` / `external-analysis`, alongside scan and signature-update kinds. Approval/requester/declared case bindings remain unchanged. Mandatory source/report SHA-256; output digest matches the exact existing exported partition-provenance evidence payload. AR/EN Government Administration labels the third kind. No new acquisition, device support or elevated permission.
- Implemented: audited partition workers hold their candidate until the GUI callback rechecks authorization/cancellation. A fully finished worker with its callback still queued cannot commit its candidate after revocation. Case and policy history are separate stores, not crash-atomic: success requires both writes; failed terminal acceptance attempts a case restore, and failed restore is reported. Mid-write crash/outcome, journal capacity/key/time/identity/rollback and declared-case-not-access limits remain documented in GOVERNMENT_OPERATION_RESULTS.md. No OS sandbox/air-gap or external-tool truth claim.
- Tested locally: CMake/Werror build; policy/offline/TSK/interoperability **4/4 CTest** (29.82 s); native policy **13/13**, offline **17/17**, independent signed corpus **40/40**; real separately installed Linux mmls with known-answer synthetic MBR and exact export-byte digest. Running cancellation/logout/expiry/revoke, queued-completion revoke, case-write/restore failure and corrupt-journal refusal passed. Python **38/38**; research catalog and pinned scanner exception mutation corpus passed. Complete local full suite, QML/DPI/repeats and new exact-source hosted Windows/Linux/sanitizers/deployment are **PENDING**.
- Fresh workspace recovery: scratch was pruned, so source was re-cloned; the original historical GUI runtime snapshot is unavailable. Historical GUI root cause remains OPEN. Earlier two failure logs/stages and eight subsequent passes remain in operation-results-2026-10-27; no fix inferred. Fresh pinned dependency preparation first failed scalarmult8 (detailed log unavailable after temporary cleanup); persistent investigation then proved two other generated ELF files had mode 0644 and tests could not execute (126). Only those two modes were repaired, original failures retained, and complete upstream dependency tests passed **101/101**. No source/test/expected-output or signed dependency pin was weakened; no causal link to the historical GUI failure is claimed.
- Latest prior verified executable/workflow source remains `9f2438d8c1e1367834fe75526b7f9f3cee0e3846`, CI 40414566542. Its executable SHA-256 is `30dcbfdc401ac968d7282789a6010fef8492eb8360c8b81c4b376348780faca2`; its earlier Windows packages and limited Win10 test do not contain/inherit this partition-result increment. Independent OS DNS/proxy/descendant/share/browser/export, clean Win10/11, actual TSK on Windows/phone and institutional custody/security/signing/acceptance remain external gates. No competitor executable/comparative superiority result.
- Competitive gap reduced: one more existing authorized worker now has independently checkable authorization-to-result/export correlation; cost/TCO unmeasured. Exact dated XAMN/Oxygen comparison axes are reused without claiming a new source review or parity.
- Next executable task: finish full/QML/GUI-repeat and exact-source CI, inspect failures, verify new Windows bundle/installed byte receipts and actual frames, then update this first block. Retain GUI investigation; next trust work is cross-store interrupted-result reconciliation and independent controlled offline-boundary observation, followed by another existing worker or one bounded forensic-analysis gap.
- Resume: `git fetch origin feature/government-operations`; `git switch feature/government-operations`; `git status --short`; `git log -4 --oneline`; `cmake --build build --parallel 2`; `MR_TSK_MMLS_PATH=/usr/bin/mmls ctest --test-dir build --output-on-failure`; `ctest --test-dir build -R '^mr_gui_smoke$' --repeat until-fail:3 --output-on-failure`; `python3 -m unittest discover -s tests -v`.

Historical checkpoints below apply only to their pinned source and do not supersede this implementation's pending full/hosted gates.

# Government operation results — verified source / limited Windows host checkpoint (2026-10-26 UTC)

- Active Branch: `feature/government-operations`. Current verified executable/workflow source HEAD: **`9f2438d8c1e1367834fe75526b7f9f3cee0e3846`**; implementation `6d63b4d76a82f2ca5f4943429b6fce166ecc47f7`. The subsequent commit containing this evidence checkpoint changes only DEVELOPMENT_STATE.md and non-shipped evidence; resolve that current branch HEAD with `git rev-parse HEAD` or GitHub. Its binaries remain explicitly sourced from 9f2438d. PR #15 is open/draft on unmerged PR #11 (`8fb9b38`); main unmerged, branches retained, Guardian untouched.
- Latest CI: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542 — **all seven jobs passed, attempt 1, source 9f2438d**. Windows/Linux Production/Laboratory each **48/48 CTest**; Production ASan/UBSan **17/17**; Qt Quick **51/51** and existing 125%/150% DPI groups; three consecutive GUI repeats for each of the four profiles. Native policy **13/13**, offline **11/11**, separate-language signed corpus **35/35**. Existing intentional native subcase platform/profile/service exclusions remain visible in raw artifacts; this is not a claim of zero excluded native subcases. Python 40414566529 and public-history 40414566488 also succeeded; PR checks succeeded.
- Verified executable byte SHA-256: **`30dcbfdc401ac968d7282789a6010fef8492eb8360c8b81c4b376348780faca2`**. Import worker byte SHA-256: **`3d2ff92436a4afbd294f665f73fc7fd890295c1be96475871335d87dbb04c533`**. These are file hashes, not Git SHAs. Downloaded provider ZIP digests, all **1,418** inner manifest entries, source/profile, identical portable/installed tested bytes, independent policy receipts and install/run/uninstall/cleanup were independently checked before local environment loss. Shipped operation-result/policy/offline guides and Python verifier matched canonical LF source. Exact package hashes are in `evidence/operation-results-2026-10-27/windows-delivery-verification.json`.
- Implemented: signed native operation-start/result records extend the EXISTING policy audit for **SecurityBridge ClamAV file scans and FreshClam updates only**. Operation UUID, requester, actual approval sequence/hash, declared case/category/kind, local times, result/failure classification and measured source/report hashes correlate authorization with callback outcomes. Start commits before dispatch; terminal records are system observations. Original revocation/expiry/logout prevents a later completed verdict even after regrant. Unfinished restart records become process-interrupted failures with unknown external outcome; no auto-resume. Completed scan export bytes hash to their receipt. Native and independent Python verification enforce exact fields, immutable bindings, unique start/result and closure-compatible outcomes, including adversarial histories with valid signatures.
- Implemented/Tested UI: real bounded owner/reviewer operation-receipt projection in Arabic/English Government Administration, cleared on logout. Actual Qt Quick frames with synthetic native receipts were inspected and retained as `windows-ci-operation-receipts-{ar,en}.png`. These UI fixtures deliberately use a known synthetic digest; they do not claim an actual scan occurred in that pictured UI. Actual native worker/cancel/revoke/logout/active-expiry tests are separate.
- Reliability **OPEN**: original exact historical runtime snapshot is unavailable. Baseline 6e399f2 full **48/48** and three GUI repeats passed. The operation-result implementation then passed a full **48/48** after explicitly repairing generated zero/non-executable local build outputs, with initial failures preserved. Subsequent GUI repeats stopped on run 3 at artifact-synthetic-source-setup (exit 6), and a narrower investigation stopped on run 3 at policy-audit-export (exit 16). No common root cause or causal link to generated-output corruption is established. Fixed-literal case/device/file stages and allowlisted audit health/session/role/sequence/error-class context were added without case content, reasons or credentials. Eight local follow-up repeats and hosted repeats passed; **passing repeats do not prove resolution**. Failures remain in this evidence folder; no silent retry-until-pass gate was introduced.
- Hosted dependency reliability: source 6d63b4d Linux production attempt 1 timed out before compilation during slow Azure mirror package transfer. Source 9f2438d preserves signed APT authentication, bounded retries/IO deadlines and the 10-minute step limit, preferring official HTTPS Ubuntu mirror priorities. Its production log downloaded **71.6 MB in 12 seconds** and completed all gates. The old failure excerpt is retained separately.
- **Limited controlled Windows host test, not general Field Verified**: one existing Windows 10 Home **19045**, 64-bit, ran the exact source-9f unsigned portable with new synthetic data, SDK-excluded process PATH and Qt Quick software rendering. Actual held OS process handle observed exit **0**; main/worker hashes were unchanged, six startup categories denied and native signed-policy/revocation checks passed. Separate Python/cryptography **50.0.2** in an isolated temporary environment verified the synthetic portal history (**23 events**); global Python was unchanged. That history contains no security worker receipts; the operation-result corpus is separately tested in CI. The anchor was retained by the smoke process, not an institution. Public receipts omit host identifiers/private paths; raw journals/anchors remain private.
- Independent observation limits: **73 owned-PID TCP snapshots** on the source-9f host saw the expected loopback and no external TCP connection. Polling can miss transient traffic. DNS/UDP/proxy/descendant/share/browser/export/complete egress containment remain **Pending**, not inferred from absence. The local strace capability probe was denied before tracing. No firewall/system policy was changed. Software-renderer captures do not prove hardware-renderer/flicker acceptance. No clean Windows, Windows 11, installer-on-this-host, phone/OPPO or institutional acceptance is claimed.
- Remaining limits: two receipt worker kinds only; other engines keep their existing independent gates/results. Optional case UUID is declared scope, not file membership or access. Started means native dispatch, not attested OS launch; callbacks/hashes do not certify external-tool truth or physical events. Local identity/key/time/anchor/rollback/current-account compromise, 8,192 events/16 MiB, 64 unfinished operations/32 UI rows and incomplete audit-write recovery limits remain. Full OS containment, publisher signing, institutional custody/security review and equivalent competitor benchmarks remain external gates. No parity/superiority or final government release claim.
- Competitive gap reduced: authorization-to-outcome correlation and independently checkable report digest for two existing workers, with real AR/EN UI and adverse lifecycle tests. Existing XAMN/Oxygen dated source records remain comparison axes; no competitor executable was tested. Matrix/roadmap changes are in implementation source 6d63b4d.
- Session continuity: the local build environment disconnected **after** successful downloaded-bundle/guide verification. Already published source/failures remain on GitHub. This checkpoint was saved through the GitHub connector from retained verified results and scoped test-host files; no additional local build or final local history scan is claimed after disconnection. Some extra local evidence copies may remain uncommitted when that workspace returns; preserve/compare them before updating. Selected public aggregate receipts, actual app-only frames and hosted test records are committed here; full hosted artifacts remain linked below.
- Next executable task: keep GUI root-cause investigation open and use the new exact-stage/context diagnostics on the next failure, preserving failing run resources. Expand native operation-result coverage to one additional existing worker (authorized local external partition analysis is a candidate), with cancellation/logout/revoke/expiry/interruption and report-byte checks. Continue controlled independent DNS/proxy/descendant/share/browser/export observations on a trace-capable authorized host; do not treat this TCP sample as containment proof. Broader artifact/device/research work follows the trust priorities.

Unsigned evaluation downloads for source 9f2438d:

- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542/artifacts/12695188108
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542/artifacts/12694978216
- Windows Production raw tests/captures/deployment receipts: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542/artifacts/12694473681
- Windows Laboratory raw tests/captures: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542/artifacts/12695153272
- Linux Production including sanitizers: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542/artifacts/12696151398
- Linux Laboratory: https://github.com/nexvary/NEXVARY-MR/actions/runs/40414566542/artifacts/12696136361

Resume commands (preserve disconnected-workspace copies before fast-forward):

```sh
git fetch origin feature/government-operations
git switch feature/government-operations
git status --short
# If this disconnected workspace still has this batch's local evidence copies:
git stash push --include-untracked -m 'preserve local receipt copies before connector checkpoint' -- evidence/operation-results-2026-10-27
git merge --ff-only origin/feature/government-operations
git log -4 --oneline
# Inspect any preserved stash before selectively restoring; do not overwrite current evidence.
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure --output-junit full-results.xml
ctest --test-dir build -R '^mr_gui_smoke$' --repeat until-fail:3 --output-on-failure --output-junit gui-repeat-results.xml
python -m unittest discover -s tests -v
```

Historical checkpoints below apply only to their named source and do not supersede this verified block.

# Government operation results — historical implementation checkpoint (2026-10-27)

- Active Branch: `feature/government-operations`; inspected remote HEAD **`6e399f29c6bf0c77bfcabd5696793ed933b4508e`** before this batch. Resolve the subsequent implementation HEAD with `git rev-parse HEAD`. PR #15 remains draft/open on PR #11 (`8fb9b38`), main unmerged and Guardian untouched.
- Reliability inspection: this retained local workspace can build/run again. Latest baseline source 6e399f2 (native source 1fb519d) passed **48/48 CTest**, followed by **three consecutive GUI smoke passes** with the new fixed-stage/exit diagnostics. Original full-suite-only failure did not reproduce; no root cause or fix is claimed. The exact historical runtime/resource snapshot is not available. Separate XML preserves the full suite and repeat runs; no retry-until-pass gate. Baseline executable byte digest was not captured before the subsequent build and is not retrospectively asserted.
- Implemented: native operation-start/result extension of the EXISTING signed policy journal for **SecurityBridge local ClamAV scans and FreshClam updates only**. UUID/requester/actual approval sequence+hash/declared case/category/kind/start/completion/status/failure class/source and output SHA-256 correlate decisions with callback outcomes. Start commits before worker dispatch. Terminal records are signed system observations; post-revoke/expiry/logout results cannot become completed verdicts. Unfinished records after restart become process-interrupted failures with unknown external outcome; no auto-resume.
- Implemented/Tested locally: native plus independent Python verification of exact receipt fields, decision references, principal/case/kind bindings, unique start/terminal result and closure-compatible status. Signed adversarial histories test fabricated approval references, extra content/path fields, terminal without start, duplicate terminal, mutated case/source digest and success after revocation. Completed scan JSON export bytes hash to the retained receipt digest. Native scan/update worker tests cover completion, engine failure, cancellation, revocation, logout and expiry while running. Real AR/EN Government Administration receipt projection is role/owner scoped and clears on logout; full/DPI UI fixtures include it.
- Follow-up observations: exact operation-result source 6d63b4d passed a complete local **48/48** after explicitly repairing generated non-executable/zero-length build outputs (initial start failures retained separately). The subsequent GUI repeat stopped on its third run: `artifact-synthetic-source-setup`, exit 6. A narrower-stage investigation then stopped at `policy-audit-export`, exit 16. These reproduce GUI-smoke symptoms but do not establish a shared historical root cause. Split case/device/file stages and a fixed-class audit-export context (health/authenticated/allowlisted role/sequence; no reasons/content/credentials) were added; **eight consecutive** local runs of that follow-up passed. Passing repeats do not prove resolution.
- Hosted 6d63b4d Windows production/laboratory and Linux laboratory passed their complete gates, including portable/installer production deployment. Linux production timed out at the 10-minute dependency-install step while downloading slowly from azure.archive.ubuntu.com, before compilation; logs were inspected and one job-only rerun was requested. This follow-up prefers the official HTTPS Ubuntu archives using documented APT mirror priorities, retaining the existing signed package authentication, IO timeouts/retries and step deadline. Its new exact-source full CI is pending; earlier Windows source remains explicitly 6d63b4d.
- Local checks so far: CMake/Werror build passed; targeted policy/offline/interop CTest **3/3** (28.49 s), policy **13/13**, offline **11/11**, independent corpus **35/35**, focused real QML receipt flow **3/3**, Python **38/38**, research validators passed. Exact final full suite/GUI repeats, hosted Windows/Linux/sanitizers, actual Windows captures, provider/inner package and tested-EXE digests are **PENDING**. Do not attribute earlier packages to the new operation-result implementation.
- Completed earlier work remains: reviewed signed exceptions, protected cases/evidence/custody/reviews/retention/import/timeline/index/research/training. This batch does not rebuild them. Coverage/contract: `docs/GOVERNMENT_OPERATION_RESULTS.md`, shipped with Windows alongside the updated standalone verifier.
- External gate: a fresh minimal strace capability probe was denied PTRACE_TRACEME/PTRACE_SETOPTIONS before tracing. No usable independent syscall capture exists. DNS/proxy/descendant/mounted-share/browser/full egress/export containment still needs a controlled trace-capable host and enterprise OS controls. The application remains default-deny at its existing boundaries; no OS air-gap is claimed.
- Limits: only two worker kinds have completion receipts; other engines retain their existing separate gates/results. Optional case UUID is inherited declared scope, not file membership or case access. Started means native dispatch, not OS child-launch attestation. Native callback outcomes/hashes do not certify an external tool or physical event. Existing local account/time/key/anchor/rollback/current-account compromise limits and 8,192-event/16-MiB bounds remain. At most 64 unfinished operations/32 visible receipts. Audit/write failure prevents dispatch or refuses a verdict, but cannot guarantee a final record; cancellation does not undo external effects. No publisher signing, field Windows 10/11/phone or institutional/competitor acceptance.
- Competitive gap reduced: MR now correlates two native worker outcomes with their authorization and independently verifiable report digest. Existing dated XAMN audit and separate Oxygen Review Center log/role source records are comparison axes, not equivalent workloads or proof of superiority. No competitor executable was tested.
- Next executable task: finish exact-source full local/CI/GUI/DPI/sanitizer/deployment gates, independently download/verify current Windows bundles/receipts/frames and update this first block and PR #15. Then broaden operation-result coverage one engine at a time and build controlled independent offline-boundary observations; keep the historical GUI failure open until reproduced/proven.
- Resume: `git fetch origin feature/government-operations`; `git switch feature/government-operations`; `git status --short`; `git log -4 --oneline`; `cmake --build build --parallel 2`; `ctest --test-dir build --output-on-failure`; `ctest --test-dir build -R '^mr_gui_smoke$' --repeat until-fail:3 --output-on-failure`; `python -m unittest discover -s tests -v`.

Historical checkpoints below apply only to their named source and do not supersede this current block.

# Government GUI smoke diagnostics — hosted verified checkpoint (2026-10-26)

- Current Branch: `feature/government-operations`; PR #15 open/draft on unmerged PR #11. Current HEAD SHA at executable validation: **`1fb519de373ad9324e5ad690155685fdbcf7ddb8`**. Resolve the later documentation/evidence receipt HEAD with `git rev-parse HEAD`; the receipt changes no compiler/QML/build/workflow/packaging/shipped-guide input.
- Latest Verified Executable SHA: **`1fb519de373ad9324e5ad690155685fdbcf7ddb8`**. Smoke diagnostics ac70ecda1824596e78653d8023d96ae51005590b; safe scanner location metadata 351dd877fb0854ec4c00e04e93dad0fe7dbba95c; independently checked exact source-digest exceptions 1fb519de373ad9324e5ad690155685fdbcf7ddb8.
- CI Status: https://github.com/nexvary/NEXVARY-MR/actions/runs/40365707609 — **all seven jobs succeeded**. Windows/Linux production/laboratory each passed **48/48 CTest**, followed by **three consecutive GUI-smoke passes per profile**. Production ASan/UBSan **17/17**, Windows Qt Quick **50/50** per profile plus existing DPI subsets. Intentional unrelated native platform/profile subcase exclusions remain labelled in raw reports. Python 40365707727 passed groups 38/2/19; public-history 40365707481 passed full-history/current-tree scans and the independent real-scanner mutation corpus.
- Completed Government Capabilities: existing protected cases, identities/permissions, retention/archiving/review, authorized imports, timeline/provenance finding search/relationships, research/training and reviewed signed operation-policy workflow remain intact. This batch adds fixed-literal smoke failure stages and event-loop exit codes, including policy-audit export/retained-anchor verification/receipt writing and final revocation. Diagnostics do not print credentials, case content, reasons or raw policy histories. CI preserves separate full-suite and repeat logs; any repeat failure fails the existing test gate and blocks Windows packaging.
- Completed scanner correction: safe locations traced twelve matches to four public source SHA-256 values repeated in three preceding 29ead974 input manifests. Each value matched independently fetched source and all three pinned Git versions via Python/hashlib in CI. The exception requires exact path AND exact digest. Seven existing changed-value/one-path negatives and twelve new changed-value/three-path negatives all remained detectable. No broad file/directory/rule exclusion or history rewrite.
- Remaining Government Capabilities: reproduce/diagnose the historical original-runner full-suite-only GUI smoke failure; independent controlled OS boundary observations and complete operation-result receipts; remaining artifact/device/research/training/deployment and institutional release work. These hosted passes do not establish that the original local failure is fixed.
- Known Limitations: local execution currently cannot start a shell; **no new local build/CTest pass is claimed**. No current artifact ZIP was independently downloaded/digest-checked and no current screenshot was visually inspected in this unavailable environment. CI generated actual application captures and passed deployed application/import-worker/source/profile/independent-audit/portable/installer checks. GitHub-reported provider ZIP digests are recorded as metadata, not independently verified inner package/executable hashes.
- External Acceptance Gates: actual authorized exact phones/firmware/OPPO display and flicker, clean physical Windows 10/11, agency identities/reviewers/key/time custody, publisher signing and independent security/privacy/license/records/recovery acceptance. No OS-air-gap or final government release is claimed; Guardian untouched and main unmerged.
- Next Executable Task: run the new stage/exit diagnostics on the original failing local runner when available. Continue independent operation-result receipt coverage and controlled OS-child/DNS/proxy/share observations on an authorized trace-capable host; do not repeat completed import/timeline/index engines.
- Resume Commands:

```sh
git fetch --all --prune
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
gh run view 40365707609 --repo nexvary/NEXVARY-MR
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure --output-junit full-results.xml
ctest --test-dir build -R '^mr_gui_smoke$' --repeat until-fail:3 --output-on-failure --output-junit gui-repeat-results.xml
```

Unsigned evaluation packages and actual hosted captures from this source:

- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40365707609/artifacts/12672015696
- Installer + SHA256SUMS: https://github.com/nexvary/NEXVARY-MR/actions/runs/40365707609/artifacts/12670363454
- Windows production raw reports/captures/deployment receipts: https://github.com/nexvary/NEXVARY-MR/actions/runs/40365707609/artifacts/12670538265

The production job passed actual isolated portable and installed-in-place GUI smoke, independent policy verification and installer install/run/uninstall/cleanup gates. Evidence metadata, exact seven-file source delta and selected log excerpts: `evidence/gui-smoke-diagnostics-2026-10-26/`. Full raw reports/captures stay in the linked artifacts until their recorded expiry. The earlier ac70ecda/351dd877 native runs were superseded; their history failures are retained as provenance of the reviewed public-digest correction.

Historical checkpoints below apply only to their stated source SHAs.

# Government policy trust — hosted/deployed verified; local GUI follow-up (2026-10-26)

- Active Branch: `feature/government-operations`. Current published source HEAD: **`8e7e17467dd6a6ea34d01115d396e5c19b9f6f6e`**; later evidence-only checkpoint HEAD is resolved with `git rev-parse HEAD`. PR #15 remains open/draft on PR #11's unmerged `feature/law-enforcement-case-foundation` (`8fb9b38`). No main merge, other-branch deletion, new Government product or Guardian change.
- Implemented follow-up: diagnosed actual job 121551321861 logs and verified the downloaded failed Windows artifact ZIP against provider SHA-256. PowerShell returned two Python application matches; 6597367 selects the first executable before PATH isolation while retaining mandatory independent signature verification. 8e7e174 validates the packaged source/profile against the workflow, compares copied application/import-worker bytes with the supplied bundle, hashes both before/after deployed smoke, and records both SHA-256 values in portable/installed receipts. These are unsigned CI evaluation metadata/integrity receipts, not publisher signatures or reproducible-build attestation.
- Completed features remain protected durable Ed25519/hash-linked policy history, credential-signed human decisions, explicitly attributed system observations, separate-account Request → Review → Approve/Reject → one-time requester activation → Revoke/Expire, consented new-file-only public export, retained anchors and native/independent Python verification. Bounds/identity/time/rollback/category scope are in `docs/GOVERNMENT_POLICY_AUDIT.md`; none are removed by packaging verification.
- Latest exact-source push CI: https://github.com/nexvary/NEXVARY-MR/actions/runs/40356252239 — **SUCCESS, all seven jobs**. Windows/Linux production/laboratory each passed **48/48 CTest**; production **17/17 ASan/UBSan**. Native policy 11/11, offline 10/10, Qt Quick 50/50 and independent corpus 26/26 passed in their scopes; platform/profile subcase exclusions remain visible in raw reports. Selected push-run artifact metadata and deployed source both match 8e7e174; the PR event run is distinct.
- Local follow-up: CMake build passed (existing exact native inputs, no work needed), targeted policy/offline/interoperability CTest 3/3 (17.09 s), Python 38/38, catalog/registry validators passed. Final local full CTest ran twice: **47/48 both times**, GUI smoke failed in the full runner even after preserving the old synthetic profile and using a new one. Fresh standalone and repeated standalone smoke both exited 0; the exact CTest command run directly from the same directory also exited 0. An isolated CTest GUI retry then passed 1/1. Cause is not established; neither old profile nor netlink warnings are a proven explanation. Raw failed XML/logs remain retained; no final local full 48/48 is claimed. `verified-source-inputs-8e7e174.json` pins 267 native/QML/build/workflow/packaging/shipped-guide/test/resource inputs; only the packaging smoke script differs from 8e6c759. Earlier complete local/hosted results remain attributed to their actual sources below.
- Failed Windows 8e6c759 export independently verified locally: 23 events and retained prefix, valid signatures/lifecycle. Public receipt retains provider/file hashes and result; raw exported history/anchor stay in downloaded validation files. Automatic review rejected publishing raw signed history because it might expose identities/reasons/operational details; source publication omits that history and uses hash/result receipts.
- Independent OS observation attempt: local `strace` was denied `PTRACE_TRACEME/PTRACE_SETOPTIONS` before test execution. This is an **External Gate**, not a passing containment test. No usable syscall capture exists. Native controlled gate/child/socket tests remain separate application-level evidence. DNS/proxy/UNC/mounted-share/browser/descendant containment still requires an authorized trace/capture-capable host and enterprise OS controls; MR is not an OS air gap.
- Competitive position: operational-policy audit/delegation and deployment traceability gaps reduced within MR only. Official XAMN 7.3 audit and separate Oxygen Review Center role/logging claims were rechecked with dated/version/component scope; no competitor executable, institutional acceptance or comparative superiority/parity is claimed.
- External blockers: independent security/records/recovery/privacy/license review, institutional identity/reviewer/key/time custody and revision-floor enforcement, scalable history rotation/complete operation completion receipts, clean physical Windows 10/11, authorized exact phone/firmware/OPPO display verification and publisher signing. Do not treat Windows Server CI or different local accounts as institutional/field acceptance.
- Verified Windows executable SHA-256: **`5bbede44f688cbaccac2f635fa58a7f009c65543cea797be1829cb6631c18118`**; import worker **`3620f23e5227807d1c5954ec410b9719163575ddc0f77de87a095f729cf15d0a`**. Both match downloaded portable bytes and actual portable/installed before/after smoke receipts. Provider ZIP digests and **1,417 inner file checksums**, source/profile metadata, installer install/run/uninstall + executable/registry cleanup, canonical-LF policy guides/verifier and actual AR/EN Windows frames were checked. Separate local cryptography verification of both downloaded deployed histories passed (23 events each). These are virtual-desktop synthetic CI observations, not clean physical Windows/phone acceptance.
- Verified unsigned development-evaluation downloads, source 8e7e174:
  - Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40356252239/artifacts/12666447327 — inner ZIP SHA-256 `3cfb5269dfde942983438f26fe4868e1fe3d7800aa5553174d633422406a4cd3`.
  - Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40356252239/artifacts/12667027083 — setup SHA-256 `76b6f7be96e2a3a32ff93c787fdd1cbe6889c97160545b945f65ef87b53b421a`.
  - Raw Windows evidence/screenshots: https://github.com/nexvary/NEXVARY-MR/actions/runs/40356252239/artifacts/12667847127. File/provider hashes and independent read-only verifier are in `evidence/policy-trust-2026-10-26/`.
- Next executable task: reproduce/diagnose the local full-run-only GUI smoke failure with precise synthetic-stage/return-code diagnostics, keeping policy and verification gates intact; hosted/deployed current-source checks passed. Then controlled independent OS-child/DNS/proxy/share observations on a trace-capable host and operation-result receipt coverage before broader analysis/device intelligence. Do not claim OS containment from the blocked local observer.
- Resume commands (authenticated GitHub CLI or equivalent connector):
  ```sh
  git fetch origin feature/government-operations
  git switch feature/government-operations
  git status --short
  git log -4 --oneline
  gh run view 40356252239 --repo nexvary/NEXVARY-MR
  gh run download 40356252239 --repo nexvary/NEXVARY-MR -n NEXVARY-MR-Windows-production-validation-evidence -D windows-evidence-8e7e174
  cmake --build build --parallel 2
  ctest --test-dir build --output-on-failure
  python -m unittest discover -s tests -v
  ```

Historical checkpoints below retain the status/access available at that time and apply only to their named sources; this current block takes precedence.

# Government signed-policy trust — implemented; Windows portable gate blocked (2026-10-26)

- Active Branch: `feature/government-operations`; latest published source/packaging HEAD **`8e6c759a9c896486eda24fa0db94f23a5ca0f8f3`**. Implementation cf2a93c, eager UTF-8 validation 82c31b3, Windows verifier absolute-path correction 8e6c759. Resolve the later receipt commit with `git rev-parse HEAD`; **267 native/QML/build/workflow/packaging/shipped-guide/test/resource inputs are pinned unchanged** in `evidence/policy-trust-2026-10-26/source-inputs.json`.
- Implemented: protected atomic Ed25519/hash-linked durable policy history and credential-signed human decisions, explicit signed system observations, separate-account Request → Review → Approve/Reject → one-time requester activation → Revoke/Expire, reviewer enablement/key/role recheck, default denial and no automatic resume. Direct grants disabled. New-file-only consented public audit export, retained prefix anchors, native verification receipts and independent Python/cryptography implementation. Actual AR/EN controls use real engines; native offline/cloud/mirror and deployed smoke exercise the complete reviewed protocol.
- **Latest native CI https://github.com/nexvary/NEXVARY-MR/actions/runs/40316704075: 6/7 jobs successful; overall FAILED.** Both core jobs, tracking server, Linux production/laboratory and Windows laboratory succeeded. Windows production built successfully and passed its CTest step, then **Verify portable in isolated path with spaces failed**. Installer creation/deployed install and package publication were skipped. Do not claim successful current Windows packaging, a verified Windows executable digest or current Portable/Installer download links.
- Linux production evidence ZIP was downloaded and its provider SHA-256 **e7018bfb118af442d9d0db161fde4b851730bdea0f6663e625ea33a1d5e8306a** matched. Actual reports: **48/48 CTest**, **17/17 ASan/UBSan** with normal hosted leak checking, policy **11/11**, offline **10/10**, Qt Quick **50/50**, independent verifier corpus **26/26**. Hosted verified-TLS integration passed in the completed production job. Raw selected reports are retained under `evidence/policy-trust-2026-10-26/linux-production-*`. Optional unrelated platform/profile/external-tool subcase exclusions remain visible in raw reports; zero skipped groups is not a zero-subcase-skip claim.
- Local final JUnit: **48/48, 331 s** after generated artifact repair; Python **38/38**. Earlier first implementation full 48/48 (252 s), cf2a93c final full 48/48 (338.55 s), obsolete-anchor fixture failure/correction and later execution-permission/non-ELF generated mirror failures are retained with scope notes. Earlier overlapping local builds occurred; only generated outputs were repaired/relinked. The final full JUnit is complete; no complete synchronized local console transcript is claimed. CTest XML `system-out` previews are explicitly labelled previews. Local executable SHA-256 is in `verification-status.json`; no Windows executable digest has been inspected for this failed packaging run.
- Current access blocker: GitHub connector is no longer installed/available in this resumed session; discovery confirmed it and a GitHub suggestion was issued. Public repository/jobs/annotations remain readable. Full job-log download returned **403** and artifact ZIP download **401**. Public annotations say only exit code 1. **Root cause is not yet established; do not guess or weaken portable-isolation checks.** The previous absolute-Python-path correction is already in 8e6c759 and must not be repeated as if still pending.
- Next executable task after authenticated GitHub access is restored: download production evidence artifact **12648610995** (provider SHA-256 **ed2d3397866d4292c1e844e6bb8b136fee5c4d3d9871db724c1dd6978fabd619**) and failed job **121551321861** logs. Inspect `portable.stderr.txt`, `portable.stdout.txt`, deployed policy/audit receipts and the PowerShell failure. Fix the demonstrated cause, rerun native Windows/Linux/sanitizer/portable/installer gates, verify actual EXEs/provider + inner digests/shipped guides, review real Windows captures and update PR #15. Then continue controlled OS-child/DNS/proxy/share containment observations and comprehensive completion receipts before broader analysis coverage.
- Source review refreshed using accessible official MSAB XAMN 7.3 historical logging and Oxygen Review Center role/log documentation. Exact product/version/date and blocked 403/502 source access are recorded. Registry source/axis/reference checks and Python 38/38 passed. Missing cryptographic/edition details remain Unknown; no vendor executable, comparative parity or superiority established.
- Limits: local DPAPI/credential keys/accounts/clocks only; different accounts are not independent people. Category + requester scope is not a destination/tool/case allowlist. Optional case UUIDs grant no case access. Gate results do not prove all external operation completion. Bounded 8,192 events/16 MiB/64 open requests/32 visible rows; snapshot writes per permit. Corruption/stale writer/write/capacity failure denies permissions and can prevent its own closure record. Whole-store/key reset or valid snapshot rollback after restart needs an independently retained anchor; current-account/process compromise, external revision floor/key/time custody, scalable rotation and complete recovery remain open. See GOVERNMENT_POLICY_AUDIT.md.
- External gates: actual clean Windows 10/11, authorized exact phones/firmware/OPPO display, independent security/privacy/license/records/recovery review, institutional identity/reviewer/key/time custody and publisher signing remain Pending. No connected Desktop Commander devices were found. Hosted Windows Server is not field acceptance; application gates are not an OS air gap. PR #15 remains draft on PR #11; main unmerged, Guardian untouched.

Authenticated resume examples (or use the restored GitHub connector):
```sh
git fetch origin feature/government-operations
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
gh run view 40316704075 --job 121551321861 --log-failed
gh run download 40316704075 -n NEXVARY-MR-Windows-production-validation-evidence --dir windows-policy-failure
# Inspect actual failure evidence before editing; do not lower acceptance checks.
```

Historical implementation and verification checkpoints below apply only to their named source; they do not supersede the failed current Windows packaging status.

# Government signed policy audit / reviewed exceptions — implementation checkpoint (2026-10-26)

- Active branch: `feature/government-operations`; inspected remote parent `6afe552bbb0e395d8b4e602a4399d61a5ce4e180`. PR #15 remains draft on PR #11; no main merge or Guardian changes. Resolve the commit containing this checkpoint separately from its parent.
- Implemented: protected atomic Ed25519/hash-linked policy history; credential-signed request/review/approve/reject/activate/revoke and verifier decisions; explicit system expiry/logout/restart/emergency-stop/gate/denial observations. Direct grants are disabled. A different administrator/reviewer must review and decide; only the original requester can activate once. Current requester role and reviewer key/enablement/role are rechecked. Existing case/source/device/compiled-policy gates remain independent.
- Implemented: bounded public export without overwrite after explicit consent; retained `{journalId, publicKey, sequence, hash}` anchor and in-app verification receipt; separate Python/cryptography verifier requiring no Qt/database/private key. AR/EN request/review/activation/stop/audit controls are wired to native engines. Existing native offline/cloud/mirror integration fixtures now use the full reviewed protocol; actual Windows portable/installed smoke requires signed public history and separate verifier results.
- Local initial implementation run: 48/48 CTest groups (252 s), Qt Quick 50/50 and both DPI subsets; Python 38/38. Initial native policy suite and 18-check cross-language verification passed. Exact raw outputs and real initial component screenshots: `evidence/policy-trust-2026-10-26/local-initial-*`. Later stronger lifecycle/key/JSON validation, independently signed producer negatives, rejection observations and deployed audit receipts require the final-source checks below; do not attribute the initial run to them.
- Final targeted checks: offline/cloud/mirror, cross-language verification (26 cases) and actual deployed GUI smoke passed. The first final native audit fixture had an obsolete export-head expectation after denial recording was added; original failure and the corrected 11/11 native result are retained. Both corrected audit/interoperability CTest groups passed. Synthetic deployed output writing was made repeatable without weakening new-file-only operator exports. A final complete 48-group run and current-source hosted Windows/Linux/sanitizers/packaging remain Pending at this implementation checkpoint. Latest prior fully hosted-verified executable remains `7cfa1a65bd4aba84664c791f1feb17f0e344918b`, CI 40268766117. Its Windows packages do not contain the new reviewed audit protocol.
- Scope/limits: local DPAPI/credential identities and clocks, category + requester only (not destination/tool/case confinement), 8,192 events/16 MiB/64 open requests/32 UI rows, snapshot writes per gate decision. Gate results do not establish every external operation's completion. Corruption/stale writer/write/capacity failure denies exceptions; failure may prevent its own final closure record. Whole-store/key reset or old valid snapshot after restart needs an independently retained anchor; current-account/process compromise is outside protection. No institutional identity/independent-person/external-time/HSM/OS-air-gap/rotation/recovery claim. See `docs/GOVERNMENT_POLICY_AUDIT.md`.
- Competitive gap reduced: durable independently checkable operational decisions and reviewed activation within MR. No competitor executable or comparative superiority established. Existing official vendor claims retain their original source classifications; no vendor-source revalidation is implied by this implementation.
- External gates: no connected Desktop Commander devices; actual Windows 10/11, exact authorized phones/firmware/OPPO display, agency identity/key/time/reviewer custody, publisher signing and independent security/privacy/license/records/recovery assessment remain Pending. Hosted Windows Server is not field acceptance.
- Next executable task: finish current-source local/CI/deployed audit verification and package proof; then independent controlled OS-child/DNS/proxy/share boundary observations and comprehensive operation-result receipts, followed by broader authorized artifact coverage. Do not stop at an external gate or rebuild completed case/research engines.

Resume (use your prepared reviewed libsodium prefix):
```sh
git fetch origin feature/government-operations
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
cmake -S . -B build -G Ninja -DMR_BUILD_GUI=ON -DMR_BUILD_UPDATE_VERIFIER=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON -DCMAKE_BUILD_TYPE=Release -DMR_SODIUM_ROOT=/path/to/prepared-sodium
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure --output-junit trust-results.xml
```

Historical verified checkpoints follow; they apply only to their pinned source and do not supersede this implementation's pending verification.

# Government application operation policy — verified checkpoint (2026-10-26)

- Latest verified executable/packaging/workflow source: **`7cfa1a65bd4aba84664c791f1feb17f0e344918b`**; native/QML implementation a9f160b. Starting HEAD was 17ef2fa. This later documentation/evidence receipt changes none of the **40 pinned runtime/QML/test/workflow/packaging/shipped-guide inputs** in evidence/offline-policy-2026-10-26/verified-source-inputs.json. Resolve receipt HEAD separately from executable source.
- Native CI **https://github.com/nexvary/NEXVARY-MR/actions/runs/40268766117 — all seven jobs passed**. Windows/Linux production/laboratory each **46/46 CTest**. Linux production **16/16 ASan/UBSan**, normal leak checking, plus real verified-TLS Python-service integration. Python 40268766137 and public-history 40268766104 passed. Core Windows/Linux each 4/4. Windows raw reports: policy **10/10**, mirror **22/22**, Qt Quick **50/50**, imports 36/36, finding index 32/32 and timeline 26/26; complete 125%/150% DPI subsets passed. Raw optional external-service/platform/profile exclusions remain visible (e.g. Windows cloud 7 passed/1 skipped); passing groups are not zero-subcase-skip claims.
- Implemented/Tested: six native application operation categories start denied even with saved cloud settings and after changing workspaces. Cloud publish/listener/FreshClam/device tools and scrcpy/external analyzers/browser handoff require explicit category permission. Local administrator + reason + consent; memory-only grants expire within ten minutes/logout/identity change, new operations recheck the session, active grants have a one-second event-loop monitor. Revocation aborts owned requests/listeners/clients and cooperatively cancels host workers/processes; regrant does not automatically resume old work. Existing case grants, legal consent and compiled production/laboratory command restrictions remain. Native local case/import/index/timeline operations stay available.
- Actual **portable and installed EXEs** proved denied startup APIs, used the same administrator gate for a synthetic loopback portal grant, revoked it, then completed native local recovery/import/Arabic finding-index processing while default-denied. Both six-category policy receipts, both AR/EN captures, SDK/PATH isolation, install/uninstall exit 0 and executable/per-user registry cleanup passed. All three downloaded provider ZIP digests and both inner SHA256SUMS matched; both shipped guides matched canonical LF source. Actual deployed virtual-desktop frames are **1028x749**; the separate Qt Quick exception-control frame is 1536x980. All data is synthetic; no generated imagery, physical-phone or Windows 10/11 field acceptance is implied.
- Local checks: nine targeted native groups passed; final policy 10/10, mirror 22/22, cloud saved-settings/held-GET/denied-change debounce and native URL-handler fixtures passed. Full UI 50/50, both DPI subsets, deployed GUI smoke and final-caption smoke passed; Python 38/38. Earlier delegate/syntax/identity-scroll failures and launch-order/environment problems were fixed and retained with scope notes. No single complete final-source 46-group local run is claimed; hosted profiles each passed 46/46. Raw evidence: evidence/offline-policy-2026-10-26/.
- Scope: application API/owned host-process gates, **not OS air-gap containment**. Broad category exceptions are not origin/tool attestations; grant context is transient, not a signed durable audit. Already transmitted/published data, browser launches, child descendants and device/database changes cannot be recalled. Mounted/UNC/proxy/DNS/privileged-process containment, controlled export and agency exception delegation remain open. Unsupported protected local identity fails closed. See the shipped GOVERNMENT_OFFLINE_POLICY.md for exact limits.
- Competitive review: official dated Inseyets offline-configuration, XAMN Viewer 8.1 offline-map and AXIOM 8.6/Copilot feature statements were inspected and classified **Officially Claimed**. Current Inseyets page direct access returned 403; the dated official PDF was available. Offline geographic mapping and semantic Q&A/deepfake classification remain Unsupported in NEXVARY. No licensed competitor was executed and **no superiority/parity is established**. Matrix/roadmap and research-source-review.json retain source/date/scope/access limits.
- External gates: read-only Desktop Commander inventory found no connected devices in this task. Clean physical Windows 10/11, exact authorized phones/OPPO display/flicker, independent security/privacy/license/records/recovery review, agency identity/key/time custody and publisher signing remain Pending. PR #15 stays draft/open on the unmerged PR #11 foundation; main unmerged, Guardian untouched.
- Next executable task: durable independently verifiable signed policy decision records and reviewed exception delegation; independent OS egress/descendant/share testing before any air-gap acceptance claim, then institutional report/review and broader artifact coverage. No final government release/accreditation claim.

Verified **unsigned development-evaluation** packages, source 7cfa1a6:
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40268766117/artifacts/12627004522 — inner SHA-256 `ba060c6c94301983ae384cac3ab1b3fb7844093f1ca70e8562f5f55cbb67c764`.
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40268766117/artifacts/12627851489 — setup SHA-256 `67d27b7b950255913fcb7aef39da1f47d6e463c620f56d3a41bc8c9f6f63fc99`.
- Windows raw tests/deployment/frames: https://github.com/nexvary/NEXVARY-MR/actions/runs/40268766117/artifacts/12626959662.

Historical implementation/checkpoint entries below apply only to their named source; they do not supersede this verified scope.

# Government application offline gate — implemented / local checks passed (2026-10-26)

- Inspected actual remote HEAD **17ef2fadf804d1679581487ca53d6c998a0c1c29** before continuing. Prior index executable da857f9 / CI 40261921553 remains the last fully hosted-verified source until this batch passes. PR #15 remains draft; Guardian untouched, no main merge.
- Implemented native process-composed default-deny categories for cloud publishing, tracking listener (including loopback), FreshClam downloads, external device tools/background discovery/scrcpy, external ClamAV/TSK and HTTP/HTTPS browser handoff. Saved cloud settings and workspace changes do not grant permission. Local first-party case/import/index/timeline processing stays available.
- Local administrator + explicit per-category approval + reason; memory-only exceptions expire at ten minutes/logout/identity change. New operation/session rechecks plus one-second active-grant monitoring. Revoke aborts active HTTP/listener clients, cancels owned workers/host processes, clears cloud in-sync and prevents unsolicited regrant resume. Existing consents/compiled command restrictions remain.
- Implemented AR/EN Administration/Settings controls and clear default-deny/exception state. Added synthetic policy/child/socket tests, saved-credential/in-flight HTTP tests, UI actual-click/size/DPI tests and mandatory deployed EXE denial/grant/revoke receipts/screenshots. Local checks: initial nine native CTest groups passed (67.66 s), final policy 10/10 including permitted HTTP handoff to an in-process fixture, final mirror 22/22 including owned-viewer revoke/no-resume, and final cloud tests passed including saved settings/held GET/denied-change debounce. Full Qt Quick 50/50, both DPI subsets and actual GUI smoke passed; supplementary final-caption EXE smoke passed 1/1. Python 38/38. Earlier delegate/syntax/identity-scroll failures and corrections are retained in evidence/offline-policy-2026-10-26/. No single full 46-group local run is claimed. New exact-source hosted CI/packages are Pending.
- CI follow-up: include the Government branch in the native workflow push subscription (previously only main/dev), and publish mandatory deployed Windows policy receipts/screenshots plus the shipped offline guide. Native/QML engines remain the a9f160b implementation; exact final source/packaging/workflow CI is Pending.
- Scope: application API/owned host-process gate only; no OS/descendant/share/proxy/browser sandbox or complete air-gap claim. Exceptions are broad category-level, not destination allowlists; reasons/principal/expiry are transient, not signed durable audit. Already-started changes/transmission cannot be undone. Details: docs/GOVERNMENT_OFFLINE_POLICY.md.
- Competitive gap reduced in application-level operational control only; no competitor executable/comparative superiority. External clean Windows/phone/security/institutional acceptance remains open. Next: durable signed policy decision records and independent OS egress/child/share containment validation before an air-gap claim; then signed institutional reporting and broader analysis coverage.

# Government finding index and provenance — verified checkpoint (2026-10-26)

- Branch `feature/government-operations`; PR #15 remains open/draft on the unmerged PR #11 foundation. Inspected starting HEAD `30d0991f53763b64e2ed910e7fa9c3551d37ced4`. Latest verified executable/test source **`da857f96349d0e7e90b75924642f93239d350f7d`**, initial implementation 40ed778. Resolve the later documentation/evidence receipt HEAD separately; its native/QML/test/build/workflow/packaging/shipped-guide inputs are unchanged (pinned in evidence/artifact-index-2026-10-26/verified-source-inputs.json).
- CI https://github.com/nexvary/NEXVARY-MR/actions/runs/40261921553 — **all seven jobs passed**. Windows/Linux production/laboratory each **45/45 CTest**; production **15/15 ASan/UBSan** with leak checking enabled. Python 40261921555 and public-history 40261921670 passed. Actual Windows reports confirm artifact tests **32/32**, Qt Quick **49/49**, imports **36/36**, timeline **26/26**, and complete 125%/150% DPI subsets. Existing intentional unrelated native platform/profile subcase exclusions remain visible in raw reports.
- Implemented/Tested: original asynchronous, bounded in-memory retained-import finding index with literal Arabic/English search, deterministic case-scoped graph IDs and typed case/device/evidence/analysis/finding paths. Registered evidence/complete SQLite BLOB/ADB member recorded-digest groups are explicit claims only. Equal names/labels never infer people, events or ownership. Exact report hash/schema/adapter/version/source size/hash/membership/provenance/duplicate-key/count checks run even after the row cap. UI paging, selected actual native path, query/case/logout clearing, cancellation, grant/actor/revision completion checks and nearby rejection feedback work. Case changes reset the analysis scroll. No new database/schema or Guardian changes.
- Independent synthetic topology: 6 findings, 19 nodes, 20 expected typed edges, two digest groups; native comparison found **0 false/0 missed structural links on that corpus only**. Arabic name and equal device/file-label negatives do not link identities; case-scoped IDs do not cross-link cases. This is an internally authored known-answer fixture, not external independent laboratory verification or general forensic accuracy.
- Scope/limits: 100 device/1,000 evidence metadata/100-report snapshot, 64 KiB/report, 128 findings/report, query 256 code units, 2,048 retained matching rows chosen before ID sorting, graph 8,192 nodes/12,288 edges, 64 references/digest group. UI shows 12 findings/page and eight groups/four references each. Counts disclose matching/omitted, already-truncated, rejected and non-import reports. No current original-byte reread, source-time authentication, full-device/app parsing, persistent/full-text/semantic index, general interactive graph canvas, signed index review/export or enforced offline/OS sandbox.
- Verified Windows deployment: **actual portable and installed EXEs** created a synthetic case, invoked the deployed import child, rebuilt one Arabic finding with the independently expected fixture SHA-256, **6 nodes/5 edges and a five-node path**, and rendered Government QML AR/EN. SDK/PATH isolation, install/run/uninstall exit 0, executable/registry cleanup passed. All three downloaded provider ZIP digests and both inner SHA256SUMS matched. Shipped guide matched canonical LF source; original CRLF Windows bytes retained. Actual virtual-desktop EXE captures are 1028x749; the selected-path Qt Quick frame is 1536x980. These are real Qt frames with synthetic data, not generated images or physical-phone evidence.
- Local checks: artifact 32/32, timeline 26/26, import 36/36, Python 38/38 and eight targeted CTest groups passed, including UI 49/49 and both DPI subsets. Supplementary final-main Government GUI smoke passed 1/1; the later nearby-error feedback flow passed 3/3. Initial strict test-indentation and pagination/scroll failures remain retained with corrections. No single final-feedback-source local full 45/45 run is claimed; all hosted profiles passed 45/45. Synthetic Windows 12,800-observation native worker benchmark logged 287 ms (2,048 retained), local source stage 563 ms; not UI/case/acquisition/competitor or general throughput measurements.
- Competitive position: reduced bounded finding-search/recorded-provenance gaps. No competing executable was run and no comparative superiority/parity is established. Full semantic/entity coverage and persistent indexing remain Pending/Unsupported as scoped; controlled development evaluation only, no government accreditation/readiness claim.
- External gates remain: actual clean Windows 10/11, exact authorized phones/firmware and OPPO display/flicker, independent security/privacy/license/records/recovery review, agency identity/reviewer/key/time custody and publisher signing. Read-only Desktop Commander inventory found no connected devices; no physical Windows/phone field test was performed. Main remains unmerged; Guardian untouched.
- Next executable task: implement an explicit, default-deny Government offline policy at existing application network/child operation boundaries; test permitted/denied egress and clear permission/rejection behavior. Do not describe application-level policy as a complete OS air-gap sandbox. Then full-text/semantic coverage and signed institutional reporting/review with independent acceptance.

Verified **unsigned development evaluation** downloads from source da857f9:
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40261921553/artifacts/12625806134 — inner SHA-256 `234ea883ee546dc1438bbb2b1e7bf82121c1f95f7a8409c758bbb5dc5721b5f1`.
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40261921553/artifacts/12625686633 — setup SHA-256 `dd387bc58cbae6964436847dc681d6c3ea710ef3cf6ff3f0f6dda580bf19376a`.
- Raw Windows test/deployment/screenshots: https://github.com/nexvary/NEXVARY-MR/actions/runs/40261921553/artifacts/12625686643. Receipts, hashes and selected raw reports/images: evidence/artifact-index-2026-10-26/.

Historical checkpoints below apply only to their pinned sources. Do not attribute old ADB/timeline packages to this index or internal fixture testing to field/institutional acceptance.

# Finding-index feedback verification follow-up (2026-10-26)

- Implementation source `40ed778a6104def4ca5aaa6e1fbe1cb1b7d18b59`, CI 40261659517, is superseded by this small UI follow-up. It adds a plain-text native error message beside the artifact build controls so rejected builds do not require scrolling to another panel. The actual UI test now asserts native query-limit rejection, visible feedback and cleared results; focused flow passed 3/3 locally. No native index/graph, executable-smoke or packaging code changed.
- The new exact-source CI must finish before current-source packages or full hosted verification are claimed. Earlier local/native/UI checks and explicit bounds remain in the implementation checkpoint below.

# Government continuation — finding index and typed provenance (2026-10-26)

- Inspected starting remote HEAD `30d0991f53763b64e2ed910e7fa9c3551d37ced4`; prior executable ef86c995 / CI 40229014198 remains successful. PR #15 stays draft on the unmerged PR #11 foundation. No Guardian or main merge.
- Implemented in this source: bounded asynchronous retained-import finding index with literal Arabic/English search, explicit omitted/truncated/rejected/unsupported counts, deterministic case-scoped graph IDs and typed case/device/evidence/analysis/finding paths. Recorded source/SQLite-BLOB/ADB-member digest groups never infer identity/people/events from equal labels. Native validation checks exact report bytes/hash, source size/hash/membership, case/device/evidence provenance, schema/adapter/version/count/duplicate JSON keys, including reports beyond the 2,048 retained-row cap. No new case database/schema, current-byte reread, persistent/full-text index, semantic entity inference or source-time claim.
- Native limits: 100 devices/1,000 evidence metadata/100 reports, 64 KiB/report, 128 retained findings/report, query 256 code units, 2,048 matching rows, 8,192 nodes/12,288 edges and 64 recorded references/digest group. UI: 12 rows/page, selectable five-node path, eight digest groups/four references per group; all narrower scope/caps explicit. Worker uses value snapshots/atomic cancellation; completion rechecks actor/session/grant/revision. Clears stale/cancelled/case-switched/logged-out results. External grant changes use existing refresh/operation checks, not a new push identity service.
- Local checks: strict Release native build; artifact tests **32/32**, timeline **26/26**, imports **36/36**, independent structural fixture validation and Python **38/38**. Eight targeted CTest groups passed in one invocation, including Qt Quick **49/49**, both DPI subsets and initial GUI smoke. The later main-executable Government fixture addition passed its supplementary GUI smoke **1/1** (20.15 seconds), exercising actual native import/index and AR/EN packaged QML captures. Windows scripts include independent deployed-fixture source hash/node/edge/receipt/image checks; hosted execution is pending.
- Independent fixture: six observations/19 nodes/20 explicit typed edges/two digest groups, repeated Arabic names/equal device/file labels as negatives. Native comparison found zero false/missed structural links on that fixture only; no general forensic/identity accuracy or competitor superiority claim. Synthetic 12,800-observation worker benchmark logged 563 ms locally (not UI/case/competitor throughput). Initial local strict-test indentation and UI pagination/scroll failures are retained with their correction outcomes. Case switching now resets the analysis scroll; UI pagination checks wait for rendered target positions and retain actual mouse clicks/assertions.
- Current source CI, Windows/Linux profiles, ASan/UBSan, portable/install lifecycle, current package checksums and actual Windows frames are **Pending** until pinned hosted results are reviewed. Prior ADB packages do not contain this index. Main/worker/Government QML packaged smoke now checks the new feature rather than relying only on source-loaded Qt Quick tests.
- Remaining: enforced offline networking; full/persistent/semantic artifact coverage and independently justified entity relationships; timeline export/signed institutional review; broader backup/app/document adapters; complete research/training/deployment/identity/recovery acceptance. Clean Windows 10/11, exact authorized phone/firmware/OPPO display, independent security/privacy/license/records/recovery review, external identity/key/time custody and trusted publisher signing remain external gates. No final governmental readiness/accreditation claim.
- Next: inspect exact-source CI, fix failures and retain actual deployed Windows index receipts/captures/packages. Then enforce and test explicit offline egress policy using existing network/child boundaries without claiming a complete OS air-gap sandbox.

# Government legacy ADB inventory — verified delivery checkpoint (2026-10-25)

- Branch: `feature/government-operations`; PR #15 remains open/draft on the unmerged PR #11 foundation. Inspected starting HEAD `46bec6856e694055260448620e804e06cc108f67`. Latest verified executable/test source: **`ef86c995ebf93bcf47e0d25ff7e8353cb117046e`**. Resolve the later documentation/evidence receipt HEAD separately. Native/QML/tests/build/workflow/packaging and shipped-guide inputs are unchanged by this receipt; hashes are retained in evidence/adb-backup-2026-10-25/verified-source-inputs.json.
- Exact-source CI: https://github.com/nexvary/NEXVARY-MR/actions/runs/40229014198 — **all seven jobs passed**. Windows/Linux production/laboratory each **43/43 CTest**; production **14/14 ASan/UBSan** including ADB interop, without disabling leak checks. Python 40229014210 and public-history 40229014172 passed. Raw production Windows reports: imports **36/36**, Qt Quick **48/48**, independent Python/native populated ADB interop **52/52**; complete 125%/150% DPI subsets passed. Intentional unrelated native platform/profile subcase exclusions remain in raw reports.
- Implemented/Tested: original `adb-backup-v5` inventory in the existing protected authorized-import workflow. Exact version-5 unencrypted/uncompressed USTAR app members; paths/package/domain/type/size/member SHA-256/absolute source offsets/raw untrusted mtime. Full validation before success, including omitted members; no extraction/execution/app decoding/time inference. Existing consent, source/working-copy hashes, fixed child/deadline/cancel, session/role/grant/actor/revision checks, deterministic report/audit/protected restore are preserved. No new database/schema or Guardian changes.
- Source limits: 4 MiB, 4,096 members, 128 retained findings, 255 UTF-8 bytes/path, 60 KiB parser report. Compressed/encrypted/other-version/PAX/GNU/link/shared-storage variants remain Unsupported. Package/path grammar is intentionally restrictive. Source hashes and member metadata do not authenticate source time, manifests, signatures or legal authority. Process separation is not OS/network containment; cleanup is not secure erasure.
- Windows delivery Verified in hosted synthetic scope: portable isolated-copy and installed-in-place app/actual parser child passed; both deployed ADB reports matched all four independently populated members' paths/content hashes/sizes/offsets. Installer install/uninstall exit 0, executable and per-user registration removed. All three provider archive digests verified; both inner package digests matched SHA256SUMS.txt. Shipped guide differs from Git LF bytes only by Windows CRLF checkout; canonical text matched. Genuine final Windows AR/EN captures were visually reviewed and retained with raw reports/deployment receipts in evidence/adb-backup-2026-10-25/.
- Local checks: six targeted CTest groups passed in one invocation (including UI 48/48, both DPI subsets and GUI smoke), native imports 36/36, interop 52/52 and Python 38/38. A final English punctuation flow passed. Default local LeakSanitizer could not inspect /proc tasks; original environment failure retained. Repeating only the worker corpus with local leak inspection disabled passed ASan/UBSan 52 checks; full hosted sanitizer validation subsequently passed 14/14 with leak inspection enabled. Do not describe the local default leak check as passing.
- Internal benchmark: synthetic 4,096-member Linux Release worker, including process startup, three runs **53.155/55.754/59.261 ms**, largest child RSS 18,212 KiB. Excludes protected case read/decrypt/copy, UI and case commit. Protocol/input hash/raw measurements retained; no competitor or general throughput/long-session claim.
- Competitive position: one bounded member-inventory gap is reduced; no competitor was executed and no comparative superiority/parity is established. Broad backup/application decoding, typed artifact/index/provenance relationships, enforced offline networking, timeline export/signed institutional review, complete training/research/deployment/identity/recovery acceptance remain Pending. No governmental release/accreditation claim.
- External gates: clean Windows 10/11, exact authorized phone/firmware including OPPO screen/flicker, agency identities/independent reviewers, trusted publisher signing, external key/time custody, independent security/privacy/license/records/recovery assessment. Hosted Windows Server 2022 is not field or institutional acceptance. Main remains unmerged; Guardian untouched.
- Next executable task: typed provenance relationships and literal artifact indexing over existing protected source/report/evidence IDs, with truth-labelled positive/negative fixtures, reproducible rebuilds, Arabic search, cancellation/revocation and no cross-case leakage or person inference from equal labels. Then actual offline policy enforcement tests.

Verified **unsigned development evaluation** downloads from source ef86c995:
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40229014198/artifacts/12612599276 — inner SHA-256 `7ee75f085e1f84cbe0c24d430b33edb0ec4195f61e5d39efa4f3d9390c2eb979`.
- Installer: https://github.com/nexvary/NEXVARY-MR/actions/runs/40229014198/artifacts/12612419887 — setup SHA-256 `30ff42930238ecb20ea8f81611b36f5a613c34624aba1b6c88081e46e1ef82ce`.
- Windows production raw reports, deployment and AR/EN/DPI captures: https://github.com/nexvary/NEXVARY-MR/actions/runs/40229014198/artifacts/12612693896.

All historical checkpoints below apply only to their pinned sources. The historical unpublished backup checkout is absent from this session; no unrelated worktree was overwritten or merged.

# Government continuation — bounded legacy ADB inventory (2026-10-25)

- Inspected starting remote HEAD: `46bec6856e694055260448620e804e06cc108f67`, unchanged from the timeline receipt. Historical CI 40223653000 remains successful for source/test `3bf0485`; its packages do not include this new adapter. The historical unpublished backup checkout is absent from this session's filesystem; current branches/worktrees contain no such prototype. No other worktree or Guardian was modified.
- Implemented in this commit: original inventory-only `adb-backup-v5` adapter within the existing protected authorized-import pipeline, worker and Government UI. Exact version-5 unencrypted/uncompressed USTAR app members only. Member path/package/domain/type/size/content SHA-256/source offset/raw mtime; deterministic report and preserved source/provenance. Validates all members before success, including omitted findings, without extraction/execution/app decoding. Existing consent, hashes, process deadline/cancel, session/role/grant/actor/revision checks, audit and protected restore remain shared.
- Bounds: 4 MiB input, 4,096 members, 128 retained findings, 255 UTF-8 bytes/path, 60 KiB report. Strict checksum/numeric/UTF-8/path/type/padding/end validation; duplicate/file-parent conflicts rejected. Compressed/encrypted/other-version/PAX/GNU/link/shared-storage formats are Unsupported, not partially accepted. Raw mtime is not verified event time and is not fed to the timestamp timeline.
- Local verification so far: strict Release native builds; 52 independent Python/native interoperability checks passed; native import suite 36/36 passed, including populated ADB protected restore, repeatability, preservation, cancellation, stale revision, logout and revoked grants; Python 38/38 passed. Six targeted CTest groups passed (import, interoperability, full Qt Quick 48/48, 125%/150% subsets and GUI smoke). Actual synthetic Linux AR/EN frames visually reviewed at normal/small widths. A final English punctuation correction is also checked through the focused UI flow. Final exact-source hosted Windows/Linux, sanitizer and deployed-worker/package results are pending; do not attribute earlier timeline green CI/packages to this source.
- Added acceptance/provenance guide, deterministic independently generated populated Python USTAR fixture, cross-language positives/negatives and full-scan/limit checks. New worker is exercised from both portable and installed Windows locations by CI; guide ships in the bundle. Documentation does not establish a passing execution.
- Competitive position: a scoped backup-inventory gap is reduced; no competitor was run, no superiority/parity claim. Broad backup/app decoding, typed artifact relationships/indexing, enforced offline policy, signed institutional timeline/report review remain Pending. Clean Windows 10/11/authorized phones, independent security/privacy/license/records/recovery review, publisher signing and agency acceptance remain external gates. No final governmental release; no main merge.
- Next: inspect exact-source CI, fix failures and retain real captures/checksums/deployment receipts. Then typed provenance relationships through the existing case reports, followed by enforced offline networking.

# Government Operations — verified timestamp timeline checkpoint (2026-10-25)

- Current Branch: `feature/government-operations`; PR #15 remains open/draft on PR #11's unmerged foundation.
- Latest Verified Source/Test SHA: `3bf0485e9b45174a4a6ae2ade8322aebba3166d1`. Runtime/native/packaging inputs are unchanged from `b60d20fdca871b7fd169d3c17023977e4e9e7ed7`; the follow-up fixed test interaction timing. The later commit containing this receipt changes documentation/evidence only; resolve its current HEAD separately.
- CI Status: https://github.com/nexvary/NEXVARY-MR/actions/runs/40223653000 — **all seven jobs passed**. Windows/Linux production/laboratory each passed **42/42 CTest**. Production ASan/UBSan passed **13/13**. Python 40223653112 and public-history 40223652991 passed. Actual Windows raw reports confirm timeline **26/26**, imports **32/32**, Qt Quick **47/47**, and complete 125%/150% DPI subsets. Intentional unrelated platform/profile subcase exclusions remain visible in raw reports.
- Implemented/Tested: original asynchronous case timestamp-literal projection from existing protected authorized-import reports, strict explicit-offset UTC normalization, preserved unresolved values/reasons, stable source/report/evidence/device/run references, literal search, bounded pagination, cooperative cancellation and completion-time identity/role/grant/revision checks. AR/EN explanations, isolated display direction for technical IDs, wrapped-text geometry/header invariance and global cancel/exit are integrated. Existing cases, retention/holds/archive/disposal review, protected recovery, native identities/RBAC/signatures/anchors, imports/PCAP/EXIF, research, USB/device/mirror functions remain covered.
- Competitive work: 14-product/tool-family x 20-axis primary-source registry; scoped source classes/Unknown cells; measurable comparison and maintenance roadmap; 20 government acceptance gates. NIST v3.3 requirements mapped to actual timeline/SQLite gaps. No licensed competitor was exercised and no comparative superiority is established.
- Windows delivery: production portable isolation, installed-in-place launch, native parser child, installer install/run/uninstall and checksums passed. All three downloaded provider ZIP digests matched; inner portable ZIP/setup EXE also matched SHA256SUMS. Actual final Windows AR/EN 100% captures were visually reviewed and retained, alongside earlier labelled local Linux captures. Source/artifact/expiry/raw results/deployed receipts: `evidence/timeline-2026-10-25/`.
- Local boundary: final full local JUnit passed 41/42; generated zero-byte application prevented smoke launch. Relinked smoke passed 1/1. Native 26/26, imports 32/32, Python 38/38 and corrected full UI/DPI passed across staged runs. This is not a claimed single final-source local 42/42 invocation. Each hosted clean profile passed 42/42.
- Known limits: 4 MiB per import, 128 findings/import, 100 retained analyses and 2,048 matching projected observations. Source/report hashes do not establish source-time authenticity. No epoch/timezone guessing, full application activity reconstruction, person/social graph, current source-byte reread, timeline export or signed institutional timeline review. SQLite remains a bounded standalone sample without WAL. Temporary cleanup is not secure erasure; child process isolation is not OS/network containment. Local clocks/identities/current-account DPAPI retain their documented boundaries.
- Remaining Government Capabilities: documented lawful backup/container/PDF/DOCX adapters; typed artifact/index/provenance relationships; enforced offline networking; timeline reporting/institutional review; full research promotion/training/deployment/identity/recovery acceptance. Physical disposal execution remains Unsupported.
- External Acceptance Gates: exact authorized phones/firmware (including actual OPPO screen view/flicker), independent clean Windows 10/11, agency identities/reviewers, trusted publisher signing, independent security/privacy/license/records/recovery review and external key/time custody. Hosted Windows Server 2022 is not field or institutional acceptance.
- Next Executable Task: inspect current branches/worktrees for unpublished backup prototypes before repeating implementation; integrate one documented owner-authorized unencrypted format with independent populated fixtures, preservation/hash/isolation/cancel/revocation gates. Then typed provenance relationships and actual offline policy tests. No Guardian work or main merge.

Unsigned Windows evaluation packages from that verified source:
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40223653000/artifacts/12610642695
- Installer + SHA256SUMS: https://github.com/nexvary/NEXVARY-MR/actions/runs/40223653000/artifacts/12610717511
- Production deployment/screenshots/raw evidence: https://github.com/nexvary/NEXVARY-MR/actions/runs/40223653000/artifacts/12610637728

Resume:
```sh
git fetch --all --prune
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
python3 -m unittest discover -s tests -v
```

The receipt is for a controlled development evaluation, not a final governmental release.
Historical results below remain attached only to their stated source/checkpoints.

# Government timeline Windows pagination-test correction — 2026-10-25

- Executable source `b60d20fdca871b7fd169d3c17023977e4e9e7ed7`: CI 40222114350 passed both Linux profiles (42/42 each), production ASan/UBSan (13/13), Windows laboratory (42/42), both core jobs and tracking server. Windows production passed 41/42; its full UI test failed the new pagination enabled-state assertion (46/47). Both production DPI subsets passed. Native timeline 26/26 and imports 32/32 passed on Windows.
- Follow-up changes only the QML test interaction: render after revealing Next, wait for page 1 and disabled Next, reveal/render Previous at its newly laid-out position, then wait for page 0 and disabled Previous. It retains all page and enabled-state checks. No application/native/packaging input changes. New exact-source CI is required before claiming a complete passing pipeline or producing new packages.
- Actual Windows production reports and 125% AR/EN captures were downloaded; the provider ZIP SHA-256 matched. Initial failure and boundary are retained in evidence/timeline-2026-10-25/windows-pagination-failure.json. Windows native timeline/import and all earlier source bounds remain unchanged.

# Government timeline visual acceptance correction — 2026-10-25

- Source parent: `a123da537ef4ee14ed9d5506ba5c23325645461d`; previous CI 40221662336 is superseded by the commit containing this correction. Native executable inputs remain those corrected at `02324429`; this correction changes only timeline QML/display tests and retained evidence.
- Actual local image inspection found visually reordered UUID digits next to Arabic labels and insufficient allocation for wrapped explanatory text. Display-only direction isolation preserves UUID order; wrapped scope/summary/reason minimum heights and geometry checks prevent scope/search overlap. Captures now include the timeline panel title/scope and preserve header pixels.
- Local verification: native timeline 26/26 and Python 38/38 passed. All 42 CTest groups passed across staged runs after relinking damaged generated outputs and correcting the delegate lookup assertion; no single final-source local 42/42 invocation is claimed. Qt Quick 47/47 and both complete DPI subsets passed after delegate correction. Final visual controls passed in AR/EN at three window sizes and DPI 1/1.25/1.5. Original failures and stage distinctions are retained in evidence/timeline-2026-10-25/. Exact-source hosted Windows/Linux, sanitizers and packaging remain required.
- Two reviewed screenshots retained in this commit are genuine local Linux Qt frames with synthetic data. They are not Windows/hardware captures. Earlier Windows artifacts remain historical until current exact-source CI produces new ones.
- Current slice implements bounded timestamp-literal projection plus source/acceptance/benchmark matrices. Broader backup/container/document parsing, typed relationships/indexing, enforced offline networking, timeline export/institutional review and external acceptance remain pending. Main and Guardian are unchanged.

# Government timeline UI verification follow-up — 2026-10-25

- Published source before follow-up: `02324429ffd0c33f5838d076556966a6a2b7ea5a`, hosted CI 40221370260 in progress. Native timeline 26/26, import 32/32 and Python 38/38 passed locally. Re-linking repaired generated identity/security/application outputs.
- The UI translation assertion failed because object-name traversal returned null for a generated Repeater label. The follow-up reads the rendered delegate's actual text through Repeater.itemAt and an alias, preserving the language/uncertainty assertion rather than removing it. Focused AR/EN/multi-size timeline controls passed after the correction; final UI/DPI regression and new exact-source CI remain required.
- Public-source review now explicitly maps NIST v3.3 timeline/SQLite requirements to current scope/gaps. Requested independent report PDFs for AXIOM/Inseyets returned HTTP 403; no unreviewed results are credited.

# Government timeline build repair — 2026-10-25

- Latest published source before this correction: `3c23960bc0ed22aac1796fa20f3e9a056da7899d`, CI 40203548197. Both core jobs, tracking server, Python and public-history passed. All four GUI profiles failed compilation: Linux rejected misleading indentation in the new UI fixture; Windows Qt 6.8.3 rejected the deprecated QDateTime TimeSpec constructor under /WX.
- This source correction retains warnings-as-errors, separates the fixture close statement and constructs UTC through the Qt 6.4-compatible QTimeZone(0) API with explicit toUTC(). It also clears a retained projection when a new build request discovers an externally changed case, and translates time uncertainty into AR/EN instead of exposing reason codes.
- Local execution is now available again in an isolated checkout; unrelated uncommitted ADB backup/Firmware work in other checkouts is preserved. Initial local checks after the two compile repairs passed timeline 26/26, import 32/32, Qt Quick 47/47 and five targeted CTest groups including 1.25/1.5 DPI. Later stale-view/localization corrections passed the final native timeline suite; full CTest is running. Local generated outputs for identity/UI/application unexpectedly became zero-byte non-executables, so affected tests cannot start until relinked. No full local 42/42 pass is claimed. New exact-source hosted CI remains required. These initial local results are not attributed to the failed published source.
- No new Windows package or final-source passing test is claimed until results are recorded below. Competitive capability registry (14 products x 20 dimensions), source classification, maintenance/benchmark roadmap and 20 institutional acceptance gates are added. Unknown competitor cells remain explicit; no superiority is established.

# Government Technology Superiority — timeline implementation checkpoint (2026-10-25)

- Branch: `feature/government-operations`; existing draft PR #15. Inspected starting HEAD: `e76d20a3a9f610dd2f3ee991b9705d7cd0845fae`; verified earlier executable: `feed968f905cbc3d2960e94cfa208139b7642b53`.
- Implemented in the commit containing this entry: original bounded asynchronous timestamp projection from existing protected authorized-import records, report/source provenance, strict explicit-offset UTC normalization, unresolved time states, deterministic ordering, literal search, cancellation, case revision/identity/grant rechecks, AR/EN paging and global cancellation. No schema migration or duplicate case/evidence database.
- New verification: pending hosted CI for this source. No local build/test was executed: the execution environment is unavailable and Remote Desktop Commander reports no connected devices. Added native timeline tests, 12,800-observation synthetic benchmark, protected-backup reconstruction, cancellation/revocation/stale-case checks, AR/EN multi-size/DPI UI tests/capture hooks and sanitizer inclusion.
- Timeline scope: imported timestamp literals only, not full activity reconstruction, packet/EXIF timelines, semantic application decoders, human/social links, current evidence-byte verification or authenticated source time. Projection is read-only/recomputed, bounded to 2,048 retained matching observations, with omissions/truncated-import counts. See docs/CASE_TIMELINE.md.
- Next: inspect exact-source CI and fix failures; retain hosted raw results/package metadata and genuine captures. Then authorized backup/container/document adapters, typed artifact correlation, provenance relationships, enforced offline policy and timeline reporting/review. Institutional review, research promotion and training/deployment remain open.
- No competitive superiority or governmental readiness claim. Main is unmerged; Guardian is untouched. Earlier checkpoint results below belong only to their pinned SHAs.

# Government Operations Phase 2 — verified authorized import checkpoint (2026-10-25)

- Current Branch: `feature/government-operations`; PR #15 stays open/draft on PR #11's foundation. Current HEAD SHA at executable validation: `feed968f905cbc3d2960e94cfa208139b7642b53`. Resolve the later documentation/evidence receipt HEAD with `git rev-parse HEAD`; this receipt changes no compiler/QML/build/packaging input.
- Latest Verified Executable SHA: `feed968f905cbc3d2960e94cfa208139b7642b53`. Core adapters/native lifecycle: `f5eedb7b2b261a309075af64abc82bf5a39dce03`; UI/deployment: `a02c8e3a91e509fd2e3d53ddadf0141381434f48`; temporary-root policy: `73e328063fd50cdf990a0da41e9cb23309bcd525`; header/pixel follow-up: `feed968f905cbc3d2960e94cfa208139b7642b53`.
- CI Status: https://github.com/nexvary/NEXVARY-MR/actions/runs/40197393089 — all seven jobs passed. Windows/Linux production/laboratory each passed 41/41 CTest; production ASan/UBSan 12/12. Windows import 32/32 and QuickTest 46/46 passed in both profiles, including AR/EN/DPI header-pixel invariance. Python 40197393117 and public-history 40197393070 passed. Deployed importer, portable isolation and installer install/run/uninstall passed. Unrelated intentional platform/profile skips remain labelled in raw reports.
- Completed Government Capabilities: existing protected cases, native identities/RBAC/grants/rotation/revocation, custody/SHA-256, signatures/anchors, protected backup/restore, retention/holds/archiving/disposal review and reports remain intact. Six original native adapters import UTF-8 documents, JSON, XML, CSV, standalone SQLite samples and exported Android threadtime logs from separate verified working copies. Child isolation, limits, source references, native consent/purpose/grant/revision rechecks, success/rejection audit, deterministic reports and backup/PDF persistence are integrated. Existing PCAP/EXIF engines remain. AR/EN import/review/cancel, Government global progress/exit cancellation and header/breadcrumb stacking are verified. Prior quiet device refresh and consented screen view remain covered.
- Remaining Government Capabilities: vendor unencrypted-backup/container and PDF/DOCX adapters, unified UTC timeline, provenance relationships and enforced offline networking; then institutional review/search/report/audit/training/research/deployment/enterprise identity/recovery and Windows RC acceptance. Physical disposal execution remains unimplemented.
- Known Limitations: import 4 MiB, 128 findings, shortened values and bounded SQLite samples; no WAL acquisition, archive extraction, encrypted-backup decryption or locked-phone acquisition. Child isolation is not OS/network containment. Temporary plaintext removal is not secure erasure; a system crash can leave traces. Explicit network temporary roots are rejected, while OS-mounted network volumes remain a deployment boundary. Real protected cases require Windows current-account DPAPI; Linux cases in CI are synthetic. Existing bounded storage/backups, local clock/accounts and screen-frame/cleanup limits remain.
- External Acceptance Gates: exact-firmware phones, independent clean Windows 10/11, agency authorization/identities/reviewers, trusted publisher signing, external key/time custody and independent privacy/license/records/recovery review. Hosted Windows Server 2022 does not satisfy these gates. No final institutional release is claimed.
- Next Executable Task: complete manifest-based authorized unencrypted-backup import and remaining document adapters on verified separate copies, then implement a unified UTC timeline with preserved source timezones/confidence/references, provenance relationships and actual offline network enforcement tests. No Guardian work or main merge.
- Resume Commands:

```sh
git fetch --all --prune
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
python3 -m unittest discover -s tests -v
```

Verified-source unsigned Windows evaluation packages:
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40197393089/artifacts/12600059436
- Installer + SHA256SUMS: https://github.com/nexvary/NEXVARY-MR/actions/runs/40197393089/artifacts/12599939687
- Production deployment/screenshots/raw reports: https://github.com/nexvary/NEXVARY-MR/actions/runs/40197393089/artifacts/12601325004

The final production evidence ZIP was downloaded and matched its provider archive SHA-256 f009b0ff6f73dc1d198f671a0dd949677668746241de5f4a1f32fabea4d1e1a0. Actual final AR/EN Windows captures were visually reviewed: earlier navigation overpaint is absent. Both deployed importer reports contain the expected synthetic finding/source hash; installer install/uninstall exit 0 and cleanup were inspected. Metadata, expiry/digests and log excerpts are in `evidence/authorized-import-2026-10-25/`. Provider ZIP digests are not inner portable/installer checksums.

Local validation: initial integration a02c8e3 passed strict Release and CTest 41/41 in one invocation, import 32/32, QuickTest 46/46 plus both DPI profiles and Python 38/38. The final importer guard passed import/ASan 32/32 (local leak inspection disabled); its full run passed 39/41 because two generated outputs could not start. Catalog relink and isolated security rerun passed; the first security retry also had a fixture child-start/lock failure. Header follow-up passed UI/DPI/native smoke 4/4. This is not a claimed single final-source local 41/41 invocation; all hosted clean builds passed 41/41.

The local execution environment became unavailable after all source commits, hosted checks and final Windows evidence inspection completed. This documentation receipt was finished through GitHub APIs. Verbose local scratch logs/final standalone PNGs could not all be copied into Git; complete hosted raw reports and final Windows PNGs remain in the linked Actions evidence artifact (expiry recorded in hosted-ci.json). Initial synthetic local captures already uploaded are retained here and explicitly labelled a02c8e3. User flow and trust boundary: `docs/AUTHORIZED_IMPORT.md`. Historical checkpoints below apply only to their stated SHAs.

# Government operations — verified device stability checkpoint (2026-10-25)

- Current Branch: `feature/government-operations`; PR #15 open/draft, based on the unmerged PR #11 foundation. Current HEAD SHA at executable validation: `bc8fb86ee5b145d2877c40c8cd7ed241fc09f3da`. Resolve the later documentation/evidence receipt HEAD with `git rev-parse HEAD`; it changes no compiler/QML/build/packaging input.
- Latest Verified Executable SHA: `bc8fb86ee5b145d2877c40c8cd7ed241fc09f3da` (implementation `f5481fe825b0c20a64159941251875dbb0891018`, then Arabic UTC direction fix).
- CI Status: https://github.com/nexvary/NEXVARY-MR/actions/runs/40190246495 — all seven jobs passed. Windows production/laboratory and Linux production/laboratory each passed 40/40 CTest; production ASan/UBSan 11/11. Python 40190246502 and public-history 40190246577 passed. Native ADB 24/24, mirror 21/21 and Qt Quick 45/45 passed in both Windows profiles; intentional unrelated profile/platform skips remain labelled in raw logs. Portable and installed-in-place tool resolution, official scrcpy startup/file hashes, installer install/run/uninstall and checksums passed.
- Completed Government Capabilities: protected cases, native identity/RBAC/ownership/grants, rotation/revocation, evidence/custody/hash, signed reviews/anchors, protected backup/restore, reports, retention/holds/archive/disposal review remain intact. This increment fixes foreground activity pulses during background discovery, stale/cancelled polling races, per-card progress, stable identity cells and dated refresh snapshots. Dedicated exact-serial/owner/component-consented screen view is available in both profiles, with pinned official Windows tools. Generic service writes retain Laboratory gating. Arabic UTC timestamps stay ordered.
- Remaining Government Capabilities: bounded authorized import adapters on verified separate working copies, unified timeline, provenance relationships and enforced offline networking; then institutional review/search/report/audit, further training/research integration, deployment/enterprise identity/recovery and Windows RC. Physical disposal execution remains unimplemented.
- Known Limitations: screen view opens a separate scrcpy window; no control/audio/clipboard synchronization and no passive acquisition claim. No physical phone was connected. Actual frame delivery/cleanup on the reported OPPO firmware and user-machine graphics/flicker require field acceptance. Exact version/hash pinning is not publisher signing or independent licensing accreditation. Existing bounded protected-case/local-clock/local-account limitations remain.
- External Acceptance Gates: exact-firmware phones, independent clean Windows 10/11, agency identities/independent reviewers, external key/time custody, trusted publisher signing, independent privacy/license/records/recovery review and release authorization. Hosted Windows Server 2022 does not satisfy these gates.
- Next Executable Task: implement bounded authorized document import using hash-before-processing, separate verified working copies, provenance, parser isolation and structured findings; preserve existing PCAP/EXIF/backup/research/USB/NPSE engines. Then timeline, relationships and enforced offline policy. No Guardian work or main merge.
- Resume Commands:

```sh
git fetch --all --prune
git switch feature/government-operations
git status --short --branch
git log -1 --format=fuller
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
python3 -m unittest discover -s tests -v
```

Windows development packages from that verified source (unsigned):
- Portable: https://github.com/nexvary/NEXVARY-MR/actions/runs/40190246495/artifacts/12597977041
- Installer + SHA256SUMS: https://github.com/nexvary/NEXVARY-MR/actions/runs/40190246495/artifacts/12597992089
- Production deployment/screenshots/evidence: https://github.com/nexvary/NEXVARY-MR/actions/runs/40190246495/artifacts/12597977051

The production evidence artifact was downloaded, its provider archive digest matched, actual AR/EN Windows captures reviewed, and bundled tool-resolution/version/installer receipts inspected. Full metadata, expiry/digests, local raw failures and genuine synthetic captures: `evidence/device-stability-2026-10-25/`. Local strict Release build, Python 38/38, ASan/UBSan tool/mirror 2/2 (leak inspection disabled) and all 40 CTest groups across runs passed: 39/40 plus GUI smoke 1/1 after repairing a damaged generated ELF output, not a claimed single local 40/40 invocation. Hosted clean builds each passed 40/40. User steps and consent/provenance limits: `docs/SCREEN_VIEW.md`. Historical checkpoints below apply only to their stated SHAs.

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
