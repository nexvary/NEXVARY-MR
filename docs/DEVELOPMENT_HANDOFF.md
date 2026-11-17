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
