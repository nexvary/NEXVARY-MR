# Government application operation-gate evidence

Development-only synthetic fixtures. The final executable SHA and hosted run are recorded in DEVELOPMENT_STATE.md. None of these tests is external laboratory/field/institutional acceptance or a complete OS air-gap evaluation.

Earlier local failures are retained rather than silently replaced:

- `local-early-not-run.txt`: CTest was invoked before the new target had linked; it could not launch an executable. No policy case executed in that attempt. A direct UI launch also initially encountered a local lost executable-mode bit; it was corrected on the generated binary, not by changing source or permissions outside the build.
- `local-initial-delegate-failure.txt` and console: the first actual grant updated the repeater and destroyed its delegate before its trailing checkbox-reset statement completed. Moved the complete grant action into the stable panel function and reset consent before the native grant. `local-delegate-corrected.txt` preserves the subsequent passing focused flow.
- `local-initial-layout-compile-failure.txt` / `local-layout-compile-test-failure.txt`: an invalid semicolon after the new wrapped checkbox's QML object blocked QML compilation. The mistaken test command continued after the failed build; its partial results are not final-source verification. Syntax corrected and all final commands stop on build failure.
- `local-pre-final-identity-scroll-run.txt`: full UI had 49 passes/one failure because the new panel displaced the existing logout control outside the test's viewport; both DPI subsets and GUI smoke passed. Identity now precedes policy controls, the existing identity test uses actual revealed mouse clicks, and the deployed policy capture explicitly scrolls to the policy. Final rerun receipts supersede this intermediate run.

`local-native-nine-groups.*` are the initial successful scoped CTest invocation; the policy suite was then extended to include a permitted/revoked TSK child (`local-policy-ten-cases.txt`). Existing profile/platform and external-service subcase skips remain disclosed in raw engine outputs; a passed CTest group does not imply every optional subcase ran. Local AF_NETLINK permission diagnostics are environment messages, not a zero-warning claim.

All sources/data/actors in screenshots and child fixtures are synthetic. Controlled child fixtures exercise permission/start/stop paths, not real-device behavior or commercial parser accuracy. Full application composition is additionally exercised by actual deployed EXE smoke. Source/guide limits are in docs/GOVERNMENT_OFFLINE_POLICY.md.

Final local receipts: full UI **50/50**, both DPI subsets and actual GUI smoke in one passing invocation; supplementary final-caption EXE smoke passed **1/1**. Final native policy **10/10**, mirror **22/22**, and cloud group passed in separate final invocations. Python **38/38**. Earlier nine-group native results include bounded import/index/security/tool/service/TSK checks. No single complete final-source 46-group local CTest run is claimed. New hosted profiles and Windows packages must pass before current-source hosted verification is asserted.
