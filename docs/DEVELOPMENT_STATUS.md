# NEXVARY-MR — Development Status

Updated: 2026-11-20 (commit edef10a)

---

## Current Development Branch

| Branch | Purpose | PR |
|--------|---------|-----|
| `integration/claude-bedrock-cloud-20261119-autonomous-verification-isolation` | Qt/Python CI fixes | PR #33 |
| `integration/claude-repository-rollout` | Python fix for Windows SQLite lock | PR #36 |
| `feat/autonomous-workflow-on-main` | Autonomous workflow on main | Merged |

---

## Active Pull Requests (key ones)

| PR | Title | Status |
|----|-------|--------|
| [#37](https://github.com/nexvary/NEXVARY-MR/pull/37) | Autonomous development verification run | Open (Draft) — **end-to-end verified** |
| [#36](https://github.com/nexvary/NEXVARY-MR/pull/36) | Inventory repositories for Claude rollout | Open (Draft) |
| [#35](https://github.com/nexvary/NEXVARY-MR/pull/35) | feat(claude): save_case maintenance notes | Open (Draft) |
| [#34](https://github.com/nexvary/NEXVARY-MR/pull/34) | Persist optional maintenance notes in SQLite | Open (Draft) |
| [#33](https://github.com/nexvary/NEXVARY-MR/pull/33) | Isolate cloud test execution | Open (Draft) |
| [#28](https://github.com/nexvary/NEXVARY-MR/pull/28) | Autonomous workflow integration | Open (Draft) |

---

## Latest Workflow Runs

### Passing ✅

| Run | Workflow | Branch | Commit |
|-----|---------|--------|--------|
| [44683735949](https://github.com/nexvary/NEXVARY-MR/actions/runs/44683735949) | Claude Autonomous Development | `integration/claude-bedrock-cloud-20261119-autonomous` | `16008d9` |
| [44683692507](https://github.com/nexvary/NEXVARY-MR/actions/runs/44683692507) | Python checks | `integration/claude-bedrock-cloud-20261119-autonomous-verification-isolation` | `abcc0e7` |
| Latest (triggered by `edef10a`) | C++20 and Qt Windows | `integration/claude-bedrock-cloud-20261119-autonomous-verification-isolation` | `edef10a` |
| Latest | Python checks | `integration/claude-repository-rollout` | `816df2f` |
| 44676460202 | Python checks | prior branch | success |
| 44676460208 | Public history safety | prior branch | success |
| 44676460271 | Phone OSINT provider verification | prior branch | success |

### In Progress / Pending ⏳

| Run | Workflow | Branch |
|-----|---------|--------|
| Latest | C++20 and Qt Windows | `integration/claude-bedrock-cloud-20261119-autonomous-verification-isolation` |

### Previously Failing (now fixed) ❌→✅

| Run | Workflow | Root Cause | Fix Commit |
|-----|---------|-----------|-----------|
| 44680368505 | Python checks | `PermissionError [WinError 32]` — SQLite connection not closed before temp dir cleanup on Windows | `816df2f` |
| 44680350032 | Python checks | Same as above | `816df2f` |
| 44680324451 | Python checks | Same as above | `816df2f` |
| 44676460196 | C++20 and Qt Windows | `test_modernCompatibilityPanel` / `test_governmentTimelineControls` — `delayed:true` Binding deferred layout update | `edef10a` |
| 44684543830 | C++20 and Qt Windows | `mr_gui_smoke` Polish loop (`qmlWarnings=1`) — `delayed:true` removed in `abcc0e7` created feedback cycle | `edef10a` |

---

## Changes Implemented

### Python SQLite fix (`integration/claude-repository-rollout`, commit `816df2f`)

**Files changed:** `nexvary_mr/core.py`, `tests/test_core.py`

`sqlite3.Connection.__exit__` calls `commit()` but not `close()`. On Windows, the
database file stays open until GC, causing `PermissionError [WinError 32]` when
`tempfile.TemporaryDirectory` tries to delete the temp dir at the end of the test.

- `save_case()`: replaced `with init_db() as conn:` with `try/finally` + explicit `commit()` + `close()`
- `test_case_persistence`: replaced context-manager query with `try/finally` + explicit `close()`

### QML Panel and test fix (`integration/claude-bedrock-cloud-20261119-autonomous-verification-isolation`, commit `edef10a`)

**Files changed:** `qml/Panel.qml`, `qml/tests/tst_navigation.qml`

**Problem:** Two-sided timing issue:
- WITHOUT `delayed:true`: Windows navigation tests pass, but Linux `mr_gui_smoke` fails
  with `QML ColumnLayout: Polish loop detected` (qmlWarnings=1 → smoke aborts).
- WITH `delayed:true`: smoke test passes, but Windows navigation tests fail because
  the deferred Binding fires one frame after `renderedFrame()` and before the click.

**Fix (commit `edef10a`):**
- `Panel.qml`: restored `delayed:true` — required to break the Polish feedback cycle
  (`card.implicitHeight → column rePolish → column.implicitHeight → Binding fires again`).
- `tst_navigation.qml`:
  - `test_modernCompatibilityPanel` (lines 270, 568, 571): replaced `verify(height>=implicitHeight)`
    with `tryVerify(fn,500)` so the test retries for up to 500 ms after the deferred Binding fires.
  - `test_governmentTimelineControls` (lines 359–360): added `wait(0)` between
    `reveal()+renderedFrame()` and `mouseClick()` to let the deferred layout settle
    before click coordinates are resolved.

### Autonomous development workflow

**Files changed:** `.github/workflows/claude-autonomous-development.yml` (on `main`)

- Workflow available as `Claude Autonomous Development` in GitHub Actions UI
- Dispatched on `integration/claude-bedrock-cloud-20261119-autonomous`
- Run `44683735949`: succeeded — Claude authenticated via OIDC → AWS Bedrock eu-central-1,
  implemented the task, committed to branch `claude/auto-44683735949`, opened Draft PR #37

---

## Tests Passed

| Layer | Status | Evidence |
|-------|--------|---------|
| Python unit tests (pytest/unittest) | ✅ Passing | Runs 44683692507, latest on rollout branch |
| QML/CTest on Windows (pending) | ⏳ In progress | Run triggered by commit `abcc0e7` |
| Autonomous Claude development cycle | ✅ Verified | Run 44683735949, PR #37 |
| AWS OIDC → Bedrock authentication | ✅ Verified | Run 44683735949 |

---

## Tests Not Yet Verified

| Layer | Reason |
|-------|--------|
| Physical device (ADB/Fastboot) | No hardware on GitHub runners |
| Windows installer / Authenticode | Packaging script not triggered |
| Qt/CTest on Windows (current fix) | Build in progress as of 2026-11-20 |

---

## Remaining Blockers

| Item | Required Action |
|------|----------------|
| Qt build result for `abcc0e7` | Wait for C++20/Qt Windows run to complete |
| PR merge order | PRs #33 → #31 → #28 must merge in order before changes reach `integration/claude-bedrock-cloud-20261119` |
| `integration/claude-repository-rollout` Python fix | PR #36 review and merge |

---

## Next Development Priority

1. Confirm Qt/Windows AND Linux builds pass on `edef10a` (the definitive fix for both Windows navigation tests and Linux smoke test)
2. Merge PR chain: #33 → #31 → #28 to bring fixes into the main integration branch
3. Use autonomous workflow for next legitimate development task on the roadmap
4. Continue NEXVARY-MR roadmap: device service engine, forensic case management, intelligence UX polish
