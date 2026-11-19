# Cloud development pilot — not activated
Baseline: feat/intelligence-ux-polish-20261117. Local redesign branch is absent remotely; do not overwrite or assume its changes are included.
Existing Windows CI: .github/workflows/cpp-qt.yml; Windows 2022, Qt, full CTest, runtime deployment, Inno Setup and installed-runtime/lifecycle tests. Reuse this, not a new installer stack.

## AWS prerequisite
Inspect existing IAM OIDC providers/roles before creating anything. No access keys.
Proposed trust (replace account only after verifying active AWS identity):
```json
{"Version":"2012-10-17","Statement":[{"Effect":"Allow","Principal":{"Federated":"arn:aws:iam::716997672148:oidc-provider/token.actions.githubusercontent.com"},"Action":"sts:AssumeRoleWithWebIdentity","Condition":{"StringEquals":{"token.actions.githubusercontent.com:aud":"sts.amazonaws.com","token.actions.githubusercontent.com:sub":"repo:nexvary/NEXVARY-MR:ref:refs/heads/integration/claude-bedrock-cloud-20261119"}}}]}
```
Restrict InvokeModel and InvokeModelWithResponseStream to the exact EU Sonnet 4.6 inference-profile ARN plus its underlying foundation-model ARNs, obtained from GetInferenceProfile in Frankfurt. Do not invent a destination region list or grant blanket Bedrock/Marketplace permissions. Existing account access/quotas remain unverified for GitHub identity.
Store role ARN (not a credential) in repository variable NEXVARY_BEDROCK_ROLE_ARN after IAM review.
Trust subject limits branch, not workflow identity by itself. Protect this branch against unreviewed workflow edits; scope a customized subject to job_workflow_ref if repository administration supports it.

## Activation and test
Workflow deliberately supports only owner-initiated workflow_dispatch, exact integration branch, serial tasks, 15-minute timeout, 900-second credentials, no automatic retries. $0.25 CLI budget is a software stop, not AWS hard billing cap.
The workflow must exist on the default branch before GitHub exposes workflow_dispatch normally. Do not merge it without owner authorization.
First run a tiny read-only task, verify STS account, Bedrock response/usage, then a disposable documentation change on a new branch.
GITHUB_TOKEN pushes do not trigger downstream CI. Confirm installed official Claude GitHub App and supported action authentication, or explicitly dispatch Windows CI through an authorized trusted path. No automatic issue/comment trigger is enabled until this is resolved and tested.
Run existing Windows workflow against exact resulting SHA. Never claim an installer from a failed test run. Do not publish releases or claim physical-device validation.

## Verification status
Implemented: reviewable workflow and deployment gates only.
Not verified: installed Claude App permissions, IAM/OIDC resources, role assumption, Bedrock quotas/model invocation, cloud edits/push/PR, Windows build/installer for this pilot.
No paid inference was issued during initial preparation.
Rollback: close this draft PR and leave existing workflow/branches untouched.
