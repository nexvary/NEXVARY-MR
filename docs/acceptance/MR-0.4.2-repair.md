# NEXVARY-MR 0.4.2 repair — acceptance record

This is a repair candidate, not a certification of live Lampyre connectivity or institutional/legal acceptance. No paid provider request, customer credentials or production case data were used.

## Audited baseline

- PR 18: `feature/osint-commercial-providers`, d615428983906b9e16f9c491092c4acf83466718; draft, unmerged, conflicting with its base; provider contracts, not a completed integration. No inline review comments at inspection.
- PR 19: `feature/lampyre-api-mcp-integration`, 5211847f924fb58800c6583092c8817ec0cdffe0; draft, unmerged, 14 commits. No inline review comments at inspection. This repair preserves its assets and working services.
- Latest prior Windows candidate verified by CI: run 43103508867, production job 130800902222 and laboratory job 130800902178 succeeded. Overall run failed because both Linux integration jobs failed. Do not label that run universally successful.
- All 21 remote branch heads inventoried before editing. Relevant case, identity, government-policy and provider source paths traced on the latest provider branch. Other branches are not blindly merged.

## Defects and repairs

| Root cause | Repair | Verification |
|---|---|---|
| `LawCaseBridge` cleared active case on every identity `changed` notification, including registry grant updates that did not change the session or remove access. | Preserve the active case for the same principal with access; still clear on logout, account switch, disabled/versioned accounts and access revocation. | New native registry-notification regression plus existing grant/revocation tests. |
| Role eligibility, case grant and temporary network approval were separate but scattered across screens. | Current account/role header; actual native permission state; direct navigation; six current-state setup checks. | Native/UI suites and compiled smoke. |
| Administrators could enable/disable accounts and grant case access but not restrict individual operations. | Eight per-account permission restrictions, bounded by the existing role ceiling. Native checks enforce case operations, MCP connection and paid searches. Changes increment the account version and require sign-in again. No administrator investigation promotion. | New role-ceiling, permission restriction and active-session revocation regression. |
| Saved key and memory-only key had the same visible status. | Separate `credentialSaved` state; protected per-application-user file remains within Windows current-user DPAPI. | Reopen/persistence fixtures; Windows protected-file checks. |
| Missing local approval produced a short technical code. | Show requester username, role, missing category, independent approval/activation instructions and direct Administration navigation. | QML and policy fixture suites. |
| A single checkbox accepted charges of unknown size. | Default deny; an independently reviewed administrator-only unknown-cost grant is additionally required, plus per-request consent and an explicit submission-count limit. No automatic retries. | Native policy fixtures and request-count checks. |
| MCP tool dispatch did not require an active accessible case. | Production composition binds dispatch/result acceptance to native case readiness, current role and reopening/access validation. | Native gate plus Windows smoke; real institutional authorization remains unverified. |
| HTTP 403, HTTP 5xx and certificate failures were grouped with authentication/network failures. | Separate provider authorization, provider service and TLS errors; no raw provider error body or credential is surfaced. | Synthetic HTTP failure fixtures; certificate behavior requires native TLS acceptance evidence. |
| A pane filled its parent height while deriving its own implicit height from that layout. | Make pane content determine vertical size; constrain only horizontal edges/top. | Repeated compiled GUI smoke and layout warning checks. |
| PE resource version was 0.4.0 while prior installer was 0.4.1. | Align candidate project/installer/PE/client metadata to 0.4.2. | Windows package metadata checks. |

## Storage and migration

No SQLite table migration, case rewrite, credential format replacement, path move or deletion is performed. The protected identity registry reads legacy schema 1. The first explicit per-account permission change writes schema 2 with a `deniedPermissions` list; earlier executables reject that schema instead of ignoring restrictions. Existing accounts without that list retain their existing role ceiling. Restrictions do not add roles or privileges. New restriction changes bump the account version and end affected sessions at the next native session check (also polled each second). Keep a backup before configuring restrictions; do not edit/decrypt the registry to bypass the schema guard.

Existing credential files remain readable. Keys remain isolated by application identity UUID and the Windows OS account. Logout clears memory; signing in again reloads saved protected credentials. Sessions are intentionally not silently restored after process restart; password-derived signing keys are not persisted in plaintext. The fixed 15-minute session expiry now explains why reauthentication is needed.

## Official provider evidence reviewed (2026-11-12)

- https://api.lighthouse.lampyre.io/api/1.0/doc/ loads `/api/1.0/openapi.json`; the official REST specification documents `TokenAuth` as a `token` query parameter, `/balance` and task/method endpoints. This is REST documentation, not proof of identical MCP authentication or pricing metadata.
- https://account.lampyre.io/api/1.6/data-lookup-doc/ was retrieved; its Swagger page references its own `api.yaml`. No undocumented REST endpoint or schema is implemented here.
- A credential-free `initialize` sent to the user-confirmed `https://account.lampyre.io/api/v2/mcp` returned HTTP 401 with `WWW-Authenticate: Bearer` and the official protected-resource metadata URL. This confirms a Bearer challenge, not acceptance of the user's API key or successful MCP negotiation. No tool was called.
- https://lampyre.io/faq and https://lampyre.io/terms describe variable Photon costs. No balance, 30/200-Photon entitlement, price or tool name is fabricated.

The client retains Streamable HTTP JSON-RPC initialize/initialized/tools-list behavior and bounded JSON/SSE fixture coverage. Live supported protocol versions, account capabilities, API-key compatibility and actual authentication remain unverified. A 401 cannot reliably distinguish an invalid key from an expired key without a documented provider signal; the UI asks the user to check the key/entitlements. HTTP 402 means provider payment refusal, not a verified numeric balance.

## Configure Lampyre

1. Set up the administrator, then create an investigator with a separate identity. Keep the administrator for permissions/review.
2. Review the eight account permissions in Administration. Select case users by the UUID supplied by the case owner; grants are independent of role eligibility.
3. Sign in as the investigator. Save the prefilled official MCP URL and API key. Confirm that the protected saved-key indicator is shown. Never paste the key into issue reports, screenshots or CI.
4. In Administration, request `external-analysis` with a reason (and the case reference where appropriate). Sign in as another authorized administrator/reviewer, review and approve. Sign back in as the requesting investigator and activate the approved request. It is temporary and ends on logout/identity change.
5. Test connection with the explicit connection checkbox. Only initialize, initialized and tools/list are sent. This never calls crawlers/tools.
6. For a potentially billable tool, request `paid-unknown-cost`. A separate administrator must review/approve it. Return as the investigator, activate both required categories, open the accessible case, and set a submission count. Zero blocks execution. Confirm each selected tool/argument submission separately.
7. Balance is unavailable in this MCP client. Check the official account. The submission count is not a Photon or money ceiling; approved unknown-cost requests can cost an unbounded amount per operation. If that is unacceptable, do not approve the unknown-cost category.
8. To replace credentials, paste the replacement and save. To remove local credentials, use Forget connection. Revoke the key with Lampyre separately. Cancellation stops waiting locally; it cannot promise cancellation or refund at the provider.

## Security and remaining acceptance

- No bypass of independent review, case grants, role ceilings, certificate validation or cost consent. Tests use local synthetic fixtures only.
- Account restrictions use schema 2 to fail closed under older executables. Registry backups remain sensitive and must be protected.
- API credentials are never included in CI or diagnostic/evidence exports. Existing response redaction is retained.
- Provider result copies are explicitly marked unverified. A separate user-confirmed attachment selects an existing case device and registers a protected evidence copy through the existing native case audit/custody path. The envelope retains the local operation ID and original case ID; switching cases prevents attachment. It does not certify provider claims or legal authority.
- Exact Photon spending caps, verified balance/cost retrieval and provider-confirmed consumption/request IDs are not implemented in this generic MCP path. A bounded submission count cannot guarantee a monetary ceiling.
- A separate server-side API-key revocation workflow, live invalid/expired-key differentiation and agency/SSO identity federation remain outside this candidate.
- Windows 10 physical installation, real Lampyre authentication, real investigator acceptance and live Arabic/English usability must be distinguished from Windows Server 2022 CI and synthetic smoke.

## Evidence status

Local Python regression suite: 42 tests passed. Native C++/Qt, Windows installation/uninstallation, screenshots, checksums and package links must be recorded only after the final exact commit finishes CI. Do not substitute the prior 0.4.1 binaries for this repair.
