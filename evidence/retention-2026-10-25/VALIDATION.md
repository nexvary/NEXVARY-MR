# Retention validation — 2026-10-25

Release warnings-as-errors build passed. Native retention: 9/9, identity: 12/12,
case workspace: 17/17; Qt Quick: 44/44, both DPI suites passed. Final full CTest
passed 38/40; mirror and native GUI smoke returned BAD_COMMAND because generated
ELF outputs lost their execute bits in this workspace. Restoring output permissions
and rerunning exactly those two groups passed 2/2. The same source is covered by
all 40 groups across that full run and explicit retries, not a single claimed
40/40 full invocation. Initial full-run failures and final raw XML/logs are retained.
No source workaround or test suppression was used. Windows/Linux hosted exact-source
validation remains the independent publication gate.

ASan/UBSan Debug: retention, local identities and case workspace passed 3/3;
UBSAN_OPTIONS=halt_on_error=1, ASAN_OPTIONS=detect_leaks=0. No local leak-inspection
claim. Python root 35/35; pinned real Gitleaks tree/history and precise-allowlist
regression passed. Operational public-history guard reported zero findings.

Pinned sodium 1.0.22 preparation initially passed 99/101 upstream self tests;
two generated test executables lacked their execute bit. Correcting just those
output permissions yielded 101/101, then make install passed. Raw original/retry
logs are retained. An initial application compile error in a roleId edit was
corrected before accepted tests; no stale outputs are accepted as validation.

AR/EN captures are genuine native Qt UI rendered on Linux/offscreen with synthetic
case data. Both were visually inspected. The new assessment uses readable localized
labels; policy, hold, archive and Back controls were exercised by the real QML test.
Existing tests log AF_NETLINK denial from sandbox network-interface inventory;
those are environment diagnostics, not suppressed QML errors or evidence of air-gap
policy enforcement. Local tests do not establish Windows 10/11, device or institutional
acceptance. Offline government networking is still a separate implementation task.

Additional actual viewport QA passed 3/3 with the retained separate QuickTest script:
1180x760 and 1366x760, AR and EN, checking all new action bounds against the
viewport content width. The 1180 AR capture was visually inspected. This uses the
same compiled native fixture and current source QML; no production bypass is added.
Reproduce: QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software build/mr_ui_tests
-input evidence/retention-2026-10-25/tst_retention_viewport.qml -o -,txt

Hosted executable validation completed: all seven jobs in 40181030127 passed;
Windows/Linux production/laboratory each 40/40, production sanitizers 9/9,
portable isolation and installer install/run/uninstall passed. Exact source,
artifact metadata and step outcomes are in hosted-ci.json. This hosted result
supersedes the earlier local generated-output mode failure without hiding it.
