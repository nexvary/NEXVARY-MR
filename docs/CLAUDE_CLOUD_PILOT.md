# NEXVARY-MR Cloud Development Pilot

**Status:** Isolated evaluation environment — not production-ready

---

## Purpose

This pilot evaluates Claude Code running in a cloud-hosted CI environment against the NEXVARY-MR repository. It is an infrastructure and tooling experiment only.

## Scope and Limitations

- All work performed here runs in an **ephemeral, isolated runner** with no persistent state.
- Results do not constitute validated, release-ready changes to any branch.
- No commits, pushes, or pull requests generated during this pilot should be treated as production deliverables without independent human review.
- This environment is disconnected from the production build pipeline and Windows installer smoke tests.

## What This Pilot Does Not Prove

- Correctness or completeness of any generated code against the full NEXVARY-MR test suite.
- Compatibility with the native C++20/Qt 6.8 build on Windows 2022.
- Readiness to merge into `main` or any release branch.
- Resolution of any open release blockers (including the audit-store discrepancy noted in `AGENTS.md`).

## Governance

Any output from this pilot that is considered for integration must pass:

1. Full CI pipeline on `feat/intelligence-ux-polish-20261117` or successor branch.
2. Human code review against all ten product pillars defined in `AGENTS.md`.
3. QML or Python test coverage for every new feature.
4. Update to `docs/DEVELOPMENT_HANDOFF.md` with branch, SHA, CI result and next task.

---

*Generated: 2026-11-19 — cloud pilot branch `integration/claude-bedrock-cloud-20261119`*
