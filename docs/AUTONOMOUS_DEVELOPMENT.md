# NEXVARY-MR — Autonomous Claude Code Cloud Development

## What this system does

The owner submits a natural-language development task. Claude Code:

1. Reads `AGENTS.md` and `docs/DEVELOPMENT_HANDOFF.md`
2. Inspects the relevant repository code
3. Implements the requested changes (Python and QML only by default)
4. Runs the Python test suite
5. Applies up to N bounded repair passes if tests fail
6. Commits the changes to an isolated `claude/auto-{run_id}` branch
7. Opens a Draft PR on `main` with a structured implementation report

The owner's laptop may be off during execution.

---

## Prerequisites

- GitHub repository variable `NEXVARY_BEDROCK_ROLE_ARN` must be set  
  (already configured — see `docs/CLOUD_DEVELOPMENT_PILOT.md`)
- AWS IAM role `nexvary-github-bedrock-development` must be active  
  (already configured — trust permits `integration/claude-bedrock-cloud-*` branches)
- `integration/claude-bedrock-cloud-20261119-autonomous` branch must exist on GitHub  
  (this branch)

---

## How to trigger a task

### Option A — GitHub web UI

1. Navigate to **Actions → Claude Autonomous Development**
2. Click **Run workflow**
3. Select branch: `integration/claude-bedrock-cloud-20261119-autonomous`
4. Enter your task description (10–2000 chars)
5. Choose max repair attempts (default 2)
6. Click **Run workflow**

> Note: The workflow must be on the selected branch for OIDC authentication to succeed.
> If the UI does not show this workflow, use Option B.

### Option B — GitHub CLI

```bash
gh workflow run claude-autonomous-development.yml \
  --ref integration/claude-bedrock-cloud-20261119-autonomous \
  --repo nexvary/NEXVARY-MR \
  -f task="Fix the Python billing test helper to handle edge-case refunds" \
  -f max_repair_attempts=2
```

### Option C — GitHub API (curl)

```bash
curl -X POST \
  -H "Authorization: Bearer $GITHUB_TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/nexvary/NEXVARY-MR/actions/workflows/claude-autonomous-development.yml/dispatches \
  -d '{
    "ref": "integration/claude-bedrock-cloud-20261119-autonomous",
    "inputs": {
      "task": "Fix the Python billing test helper to handle edge-case refunds",
      "max_repair_attempts": "2"
    }
  }'
```

---

## What Claude is authorized to do

| Allowed | Prohibited |
|---------|-----------|
| Read any repository file | Modify `.github/` workflows |
| Edit Python and QML files | Modify `AGENTS.md` |
| Write new Python and QML files | Modify credentials or IAM files |
| Run Python syntax checks (implicit) | Invoke paid external services |
| Use Glob/Grep for code search | Merge branches or push to `main` |
| Create a Draft PR | Run arbitrary shell commands |

---

## What you get back

- A `claude/auto-{run_id}` branch with the changes
- A Draft PR on `main` containing:
  - Task description
  - Changed files
  - Implementation summary (Claude's report)
  - Python test results (pass/fail)
  - Repair attempts if any
  - Estimated Bedrock model cost
  - Known limitations

---

## Verification status levels

| Level | Verified by this system |
|-------|------------------------|
| Python syntax | ✅ Yes |
| Python unit tests | ✅ Yes (with repair loop) |
| QML/CTest on Windows | ❌ No — trigger `cpp-qt.yml` manually |
| Windows installer | ❌ No |
| Physical device | ❌ No |

---

## Security model

- Only the owner (`nexvary`) can trigger execution (enforced at job level)
- Task text is treated as **untrusted input** — prompt injection patterns are rejected
- AWS credentials are temporary (900-second OIDC tokens, no stored keys)
- OIDC trust is scoped to `integration/claude-bedrock-cloud-*` branches
- No automatic merge; no force-push; no main modification
- Claude cannot modify `.github/`, `AGENTS.md`, or credentials
- One execution at a time (concurrency group)
- 30-minute job timeout
- Max 15 Claude turns for implementation, 8 for each repair

---

## Cost estimates

| Phase | Approx. cost (Sonnet 4.6) |
|-------|---------------------------|
| Implementation (15 turns) | $0.05–$0.50 |
| Each repair attempt (8 turns) | $0.02–$0.20 |
| Total per run (2 repairs) | $0.09–$0.90 |

Estimates only; actual AWS Bedrock billing may differ.
No monetary ceiling is configured. Use GitHub Actions billing limits and AWS Budgets for cost control.

---

## Rollback

Delete the `claude/auto-{run_id}` branch and close the Draft PR. No other state is modified.

---

## Verified infrastructure

- AWS OIDC → Bedrock: verified run `44477057278` (STS) and `44477689443` (Claude CLI)
- Pilot branch: `claude/pilot-44545876225`
- See `docs/CLOUD_DEVELOPMENT_PILOT.md` for full evidence

---

## Confirming a completed run

After triggering the workflow, open **Actions → Claude Autonomous Development** in the GitHub repository UI; the run appears as the topmost entry and its URL follows the pattern `https://github.com/nexvary/NEXVARY-MR/actions/runs/<run_id>`. Once the job finishes the final step ("Open Draft PR") logs the PR URL; navigate there and verify that the PR header shows the grey **Draft** label — if it shows the green **Open** label instead the PR was not created as a draft and should be converted before review.
