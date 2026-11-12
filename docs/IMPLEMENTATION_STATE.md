# NEXVARY-MR implementation state

## Baseline

- Dedicated branch: `feature/mr-government-modernization`. Base: `6f8387d9b968823d8c149597c2f61b3a84d6ed25`, the head of PR #20. Main was at `7446b17d` and is an ancestor. PRs #15, #18, #19 and #20 carry the stacked government, commercial provider, Lampyre and account-permission work. PR #17 is a separate forensic integration.
- Architecture: native C++20 Qt/QML desktop plus legacy Python tools. `IdentityService` handles account/session credentials and restrictions; `LawCaseBridge` and `LawCaseStore` own case and evidence operations; `CaseVault` provides protected storage. Existing investigator, reviewer, auditor and trainer distinctions and signed records need backward compatibility when adding new role capabilities.
- Android: `ToolBridge`, `DeviceProtocol.hpp` and the USB Doctor use ADB/Fastboot and Windows inventory. Preserve identity and authorization checks. Test unknown-model handling and authorized, unauthorized and absent states before claiming CPH2577 support.
- Intelligence: the existing Lampyre MCP client discovers tools after an approved connection. The current live execution path requires existing case and independent network/cost-policy authorization. The commercial contract checks do not constitute a deployed Nexvary backend, provider license, EUR quote or billing ledger.
- CI: `.github/workflows/cpp-qt.yml` includes Linux and Windows profiles, Qt Quick tests and Windows portable/installer smoke checks. Production installer artifacts depend on successful upstream build and verification steps.

## Changes in this checkpoint

- `qml/Main.qml`, `qml/MrPrimaryNavigation.qml` and `qml/MrHomeHero.qml` give Intelligence a dedicated top-level route. The government tab sits next to Security Dashboard. Existing route IDs are preserved.
- `qml/GovernmentAnalysisPage.qml` contains case evidence analysis. `qml/OsintWorkspace.qml` displays the bilingual Intelligence welcome and reuses the provider workflow. Leaving the route clears demo mode. No production authorization or database schema was changed.
- `qml/tests/tst_navigation.qml` exercises signed-out/no-case entry, route adjacency in both directions, back navigation, account-gated controls, existing evidence navigation and demo cleanup. `CHANGELOG.md` and `docs/WORKSPACE_QUICK_START_AR.md` describe the changes.

## Verification and next actions

- Local Linux GCC 14 Release build with warnings as errors: CTest core 6/6. Legacy Python unittest suite: 42/42. No paid provider operations were invoked.
- Native Qt UI and Windows packaging must be verified for the published commit. Physical Windows 10/11, high-DPI manual acceptance and OPPO CPH2577 USB/ADB/Fastboot remain field checks.
- Next engineering steps: validate this navigation checkpoint in native Qt, then audit account/case and Android failure paths against the existing acceptance matrices. Complete the global visual redesign and validated provider cards. Add the requested role model only with migration and authorization regression coverage.
- Commercial release remains open. Obtain an authorized provider catalog and verified quote/billing contract for a Nexvary backend. Implement idempotent transactions and reconciliation against that contract before enabling a Nexvary-priced search.
- Existing audit-store discrepancy documented in `DEVELOPMENT_STATE.md` remains a release blocker pending resolution or accepted disposition.
