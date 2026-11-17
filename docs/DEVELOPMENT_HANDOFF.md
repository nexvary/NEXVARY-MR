# NEXVARY-MR Development Handoff

## Current state — 2026-11-17

### Branch
`feat/intelligence-ux-polish-20261117`
Base: `e561322` (fix/intelligence-p0-completion = PR #24 head, Windows 0.4.10)
Remote: `git@github.com:nexvary/NEXVARY-MR.git`

### Latest commit
`b592acf` — feat: replace pending-result button with prominent alert banner + test

### Commits above base (7 total)
| SHA | Summary |
|-----|---------|
| `b592acf` | Pending-result alert banner + test |
| `159db9c` | CI: add branch to push trigger |
| `6ef0b40` | Config: portable paths in opencode.json |
| `0004b31` | Test: 5 new QML tests (11 total in IntelligenceCards) |
| `d23140d` | Feat: findings summary row in results panel |
| `edde1a9` | Config: fix permission rule order (deny last) |
| `f897373` | Feat: Intelligence UX — header, price banner, graph |

### Push status
Pushed to GitHub. Deploy key: `~/.ssh/nexvary_mr_deploy` (Ed25519).
Remote tracking confirmed: `origin/feat/intelligence-ux-polish-20261117`.

---

## CI results (run 44110539719, SHA b592acf)

| Job | Result |
|-----|--------|
| core (windows-2022) | **SUCCESS** |
| core (ubuntu-latest) | **SUCCESS** |
| tracking-server | **SUCCESS** |
| qt (production) | **FAILURE** — QML assertions |
| qt (laboratory) | **FAILURE** — QML assertions |
| linux-forensic-integration (production) | **FAILURE** — Configure and build |
| linux-forensic-integration (laboratory) | **FAILURE** — Configure and build |

### linux-forensic failure note
This job was **skipped** on `fix/intelligence-p0-completion` (the base) but **runs** on our branch (no skip condition). The failure is pre-existing on commits between the last known-good run (8d06d862 on feature/government-operations) and our base. The actual compiler failure is LampyreConnection.cpp:619: misleading indentation after a one-line conditional. Compare that line against the base before classifying inheritance. Windows also reports unified-search, default-card and owner-layout assertion failures. OsintAnalysisPane graph delegates emit undefined-property warnings.

---

## Release blockers

1. **Audit-store discrepancy** — 135 vs 139 records. Root cause in `DEVELOPMENT_STATE.md` marked CRITICAL/OPEN. Must be resolved with evidence before any release.
2. **linux-forensic CI build failure** — pre-existing; blocks clean CI on this branch.
3. **Windows 10/11 physical acceptance** — hosted CI only; no clean consumer hardware test.
4. **Authenticode signing** — packages unsigned; no certificate provisioned.

---

## Completed work (this branch)

- NEXVARY Intelligence UX: always-visible EUR 14.00 price banner, Nexvary branding header, accent-blue category cards, dynamic circular relationship graph (n nodes), findings summary row, pending-result alert banner.
- 11 QML test functions in `tst_intelligence_cards.qml` covering price display, phone validation, category filter, consent flow, pending-result UX.
- `.opencode/opencode.json`: scoped permission rules with correct last-match ordering; portable paths (no machine-specific `/home/ubuntu`).
- CI push trigger added for this branch.

---

## Next task

**B02 — Investigate linux-forensic CI build failure.**

Approach:
1. Fetch the full build log from GitHub Actions (requires GitHub token or browser access).
2. Identify the exact cmake/compiler error.
3. Check if the failure is in a file changed between `8d06d862` and `e561322` (our base).
4. If in our scope: fix, test locally if possible, commit and push.
5. If pre-existing and out of scope: document in this file and proceed to B10 (OEM service catalog expansion).

Draft PR creation is authorized through the connected Nexvary GitHub account. No GitHub API token needs to be stored on Lightsail.

---

## .opencode/opencode.json security note

The tracked configuration has local infrastructure repairs that are intentionally excluded from the documentation commit. Runtime permissions are command-dispatch controls, not a security boundary. The service retains NoNewPrivileges, with ordinary checks dispatched through a root-owned broker into systemd sandboxes that cannot read AWS/SSH credentials. Do not restore broad interpreter access or build.steps/maxSteps: a synthetic final assistant message at that limit is incompatible with Sonnet 4.6 Bedrock.

Existing sessions, original source commits and the prefill backup at /home/ubuntu/.local/state/opencode-prefill-repair.UJnJD5 must be preserved.


## Git/CI recovery — 2026-11-17
- Documentation published in 555bb2b. CI repair in fe3c0c9.
- Compiler indentation and legacy launch gating were inherited from e561322. Graph warnings were introduced by f897373. The pending-result test left shared fixture state behind and is now reset in init, with a dedicated blocking/unblocking safety regression.
- Card sizing uses its real vertical offset instead of an outdated 80-pixel header estimate.
- Seven synthetic Python billing tests and whitespace validation passed locally. Lightsail has no CMake/Qt toolchain, so hosted Windows/Linux GUI runs remain the acceptance gate.
- Draft PR: https://github.com/nexvary/NEXVARY-MR/pull/25 . No merge is authorized.
- Secure Git broker supports scoped development publishing. Read docs/OPENCODE_GIT_BROKER.md before changing its policy.
- OpenCode retains 1.19.5, Sonnet 4.6/Bedrock Frankfurt, NoNewPrivileges, session history and original prefill repair backup.
- Next task: inspect the new hosted CI results, repair any remaining demonstrated defects, then reproduce B01 audit-store count discrepancy. Do not treat 135/139 as fixed.


## UI improvement + device catalog — 2026-11-17

### Phase 1: UI improvements (Commit b0361f4)

**SupportedDevicesPage.qml:**
- Added result-count badge (green pill) in the heading row — shows `N refs` when catalog has results
- Added `EmptyState` below `ListView` for zero-result cases with context-aware messages:
  - Search active → "No results for this search / Try different text"
  - No filter → "Catalog is empty / Update catalog to add references"

### Phase 2: Device catalog expansion (same commit b0361f4)

11 new entries in `assets/data/device-service-catalog.json` (revision 2026-11-17):
- Samsung Galaxy S25 / S25+ / S25 Ultra — Snapdragon 8 Elite, Android 15, GSMArena source
- Xiaomi 14 / 14 Ultra — Snapdragon 8 Gen 3, Android 14, GSMArena source
- Redmi Note 13 Pro — Snapdragon 7s Gen 2, Android 13, GSMArena source
- OPPO Find X8 — Dimensity 9400, Android 15, GSMArena source
- Realme GT 6 — Snapdragon 8s Gen 3, Android 14, GSMArena source
- Google Pixel 9 / 9 Pro — Tensor G4, Android 14, official Google source
- OnePlus 13 — Snapdragon 8 Elite, Android 15, GSMArena source

All entries: `evidence: vendor-documentation`, `nexvaryValidated: false`, explicit disclaimer that no operation is enabled.

**Billing tests:** 7/7 pass. Isolation: OK.
**Pushed:** `0d61dd2..b0361f4` on `feat/intelligence-ux-polish-20261117`.

### Next task

Inspect CI results for `b0361f4`. If clean, proceed to:
- B10: OEM service catalog expansion (Samsung chipset service ops in `device-service-catalog.json`)
- Or investigate `mr_gui_smoke` layout-cycle warnings in `ForensicEvidencePage.qml:29` / `Panel.qml:31`
  if the maintainer authorizes touching those files.

Audit-store 135/139 remains CRITICAL/OPEN — do not close without evidence.



### Fix: tst_navigation.qml:1006 investigator launcher assertion

**Commit:** `20a3deb`
**File:** `qml/OsintWorkspace.qml`

**Root cause:** `f897373` changed the `enabled` binding on `intelligenceStartPaidSearch` from
`intelligenceOperator && !administrator` to `intelligenceOperator && !!intelligenceConnection && lookupConnected`.
This disabled the button for investigators who have no active connection — but the button's action is to
open the setup panel, not execute a paid search. No connection is needed to open setup.

**Fix:** Restored exact original gating: `intelligenceOperator && !administrator`.
- Line 982 (admin logged in): `!enabled` ✓ — admin opens setup directly via advanced settings
- Line 1006 (investigator logged in): `enabled` ✓ — investigator can open unified search setup

**Local tests:** 7/7 billing unit tests pass. QML brace balance confirmed.
**Pushed:** `94749df..20a3deb` on `feat/intelligence-ux-polish-20261117`.

### Next task

Inspect new CI results for `20a3deb` (run will trigger automatically via push trigger).
Expected: `test_osintDefaultShowsServiceCards` now passes. Verify no new regressions.
Then investigate `mr_gui_smoke` failure: `ForensicEvidencePage.qml:29` and `Panel.qml:31`
layout-cycle warnings — those files are unchanged by this repair; the cause is pre-existing.
Do not start that investigation in this session without a new task instruction.
Audit-store 135/139 remains CRITICAL/OPEN — separate blocker, do not touch.



Verified recovery:
- Four strategic documents: 555bb2ba.
- Initial C++/QML correction: fe3c0c92.
- Real OpenCode native edit, seven billing tests, Git add/commit/push via restricted broker: 86644eeb. Ten sequential model turns, no permission request, no invalid Bash call, terminal stop with no automatic continuation. Additional estimated model cost USD 0.17127726.
- Ten broker security tests and compact geometry correction: 94749dfa.
- All 359 original-session message IDs from the prefill backup remain present. No sessions or keys were deleted.
- Local .opencode/opencode.json remains intentionally uncommitted. The working runtime has no build.steps/maxSteps cap; do not restore it.

Existing Actions run: https://github.com/nexvary/NEXVARY-MR/actions/runs/44152637439
- C++ core Windows and Ubuntu: SUCCESS. Tracking server: SUCCESS.
- Linux laboratory and production: compile succeeded, but 59/61 CTest groups passed and 2 failed.
- Original unified-search and owner-card overflow assertions passed in the new Linux logs; graph undefined-property warnings disappeared.
- Remaining mr_ui_tests failure: WorkstationNavigation::test_osintDefaultShowsServiceCards at qml/tests/tst_navigation.qml:1006. The first administrator assertion now passes, but the later investigator launcher assertion expects enabled before provider connection. Inspect backend identity capabilities and the actual launch action before correcting gating. Do not weaken the test.
- Remaining mr_gui_smoke failure: Qt polish-loop warnings in ForensicEvidencePage.qml:29 and Panel.qml:31. These files are unchanged by this repair. Preserve the warning-fails-smoke gate and diagnose the layout cycle.
- Windows Qt laboratory/production were still running CTest at the last inspection. Read their existing final results before starting another build.

First Claude task: finish the role/capability-gating diagnosis at navigation line 1006, then investigate the inherited forensic Panel layout cycle. Do not develop new features, merge PR #25, or claim full CI success. Audit-store 135/139 cause, physical-device acceptance and signing remain separate blockers.

Local checkpoint and rollback: /home/ubuntu/.local/state/nexvary-git-ci-repair.sAsMRd
Original prefill backup: /home/ubuntu/.local/state/opencode-prefill-repair.UJnJD5
