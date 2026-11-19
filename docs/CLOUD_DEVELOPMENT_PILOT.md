# Cloud development pilot — verified OIDC and minimal Bedrock CLI
## Preserved baseline
Branch: integration/claude-bedrock-cloud-20261119; Draft PR #26 remains open.
Baseline: feat/intelligence-ux-polish-20261117. Local redesign work absent remotely must not be assumed included.
No application source changes or main merge in this integration.
Existing Windows CI remains .github/workflows/cpp-qt.yml (Qt/CTest/Inno Setup); no new Windows build or installer validated in this task.

## Verified AWS identity
Account 716997672148.
Role ARN: arn:aws:iam::716997672148:role/nexvary-github-bedrock-development
Repository variable NEXVARY_BEDROCK_ROLE_ARN configured; no permanent AWS credentials stored.
OIDC audience: sts.amazonaws.com. Trust permits only Nexvary-owned repositories on integration/claude-bedrock-cloud-* branches.
Both legacy and immutable subject forms are supported. Actual subject includes nexvary@325503741/NEXVARY-MR@1421842070.
Root cause of first failed authentication: trust expected the legacy name-only subject. Corrected to recognize immutable GitHub IDs.
900-second temporary credentials, 15-minute job timeout, serial execution retained.
Trust identifies the owner and branch prefix, not a specific workflow file. Only trusted administrators should edit these branches.

## Actual validation
- STS passed: https://github.com/nexvary/NEXVARY-MR/actions/runs/44477057278 (attempt 2).
- Claude Code CLI 2.1.328, eu-central-1, eu.anthropic.claude-sonnet-4-6 passed:
  https://github.com/nexvary/NEXVARY-MR/actions/runs/44477689443
- Exact result: NEXVARY_BEDROCK_OK; provider reported bedrock.
- Input 362 tokens; output 13 tokens; one turn; CLI list-price estimate USD 0.001281.
  This is not a reconciled AWS invoice amount.
- Smoke runs in an empty temporary directory, with no tools, restricted/safe mode, no session persistence and a tiny custom system prompt. No repository edits or paid external services.
- No monetary task ceiling per owner's instruction; timeouts/concurrency/explicit activation remain. AWS Budgets is not a hard spending cap.

## Trigger and GitHub App limitations
Official Claude GitHub App installed on NEXVARY-MR; read/write code, issues, PRs, actions/checks/workflows were visible. Installation permissions were not expanded.
The repository's default GITHUB_TOKEN is read-only; workflow-specific permissions are explicit. Automatic PR creation setting is off; not changed.
Normal Run workflow UI requires the workflow on the default branch. No main modification or merge authorized.
Branch-only push bootstrap successfully validates authentication.
The pinned official Claude action rejects push events before model invocation. Minimal branch smoke therefore uses the official pinned CLI directly in Actions; the official action remains only under workflow_dispatch/develop.
Automatic workflow dispatch requiring actions:write was not applied after security review rejected that persistent access expansion.
A commit explicitly containing [bedrock-smoke] and changing this workflow can request a single branch smoke, only attempt 1; ordinary pushes are auth-only.
Manual workflow_dispatch/development, official App token execution, autonomous edits/commit/PR and downstream Windows CI are not verified. Do not claim full migration complete.
GITHUB_TOKEN pushes do not automatically trigger CI.

## Rollback
Revert integration commits or close Draft PR #26; existing app and workflows remain unchanged.
Remove only the new repository variable / new roles and OIDC provider after checking other users if abandoning all integrations.
Original trust used repo:nexvary/*:ref:refs/heads/integration/claude-bedrock-cloud-*; corrected policy additionally recognizes repo:nexvary@325503741/* with the same branch restriction.
