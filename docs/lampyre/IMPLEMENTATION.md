# Lampyre integration evaluation — 2026-11-11

## Repository audit and branch choice

`main` is 7446b17 and does not contain the native Qt application. Native foundation is
`dev/cpp-qt-foundation` (0bfe9b1); government operations and forensic integration remain
separate unmerged development streams. The evidence branch (6e3301f) records an earlier
phone-provider evaluation; it is not a replacement integration base.

This branch is based on `feature/osint-commercial-providers` at d615428 because that
branch already contains the government Qt application, PhoneInfoga integration,
commercial contract, source registry, process policy and offline parser tests. PR #18
is a draft targeting government operations. PR #17 preserves the forensic stream;
PR #15 preserves government operations. No merges or unrelated repair changes were made.
Later government ADB inventory work is not present on this base and is not claimed here.

Inspected relevant branch tips, main, native CMake/QML registration, provider headers,
CaseVault, existing case OSINT panel, workflows, recent commits and open PR metadata.
Upstream d615428 PR CI: native 42336286468, commercial 42336286289 and phone provider
42336286312 failed; Python 42336286338 and history 42336286299 passed. Commercial jobs
configured and compiled on Windows/Linux but failed their tests. Local reproduction
found a whitespace escape defect, a wrong-provider fixture and a C++ line-splicing
fixture that did not actually contain the newline it intended to test. Those are fixed.
Other upstream native/phone CI failures are not diagnosed or claimed resolved.

## Architecture and implementation

Existing case authorization, identity, audit, evidence/export and Windows DPAPI remain
in their current modules. Existing PhoneInfoga execution is unchanged. The new reusable
`IOSINTProvider`, `ProviderRegistry`, live-disabled `LampyreProvider`, synthetic polling
adapter and integer `CreditBudgetManager` live in `LampyreEvaluation.hpp` and reuse the
commercial query/finding types.

The in-memory budget reserves before submission, rejects unknown/negative costs,
missing approval, duplicate IDs and overspending, retains uncertain charges and refuses
settlements above the reservation. Units are millionths of a Photon, not provider prices.
Tests use arbitrary synthetic amounts. No 200-Photon balance is installed or assumed.
This ledger is not yet persistent, synchronized or linked to external query execution.
It must not be used as a production spending authorization service.

`LampyreEvaluationBridge` connects the native Qt panel to a fixed synthetic adapter.
A timer permits cancellation before fixture completion. Only a fixed reserved-domain
selector is accepted; no arbitrary real subject, credential or endpoint is exposed.
The panel labels trial pending, balance/cost unknown and synthetic output explicitly,
and uses the existing shield asset/theme and AR/EN layout. It does not change research
case evidence or present fixture output as a live finding.

## Official capability matrix

Sources checked 2026-11-11:
- https://lampyre.io/ — advertises MCP and account/API-key connection.
- https://lampyre.io/terms — describes Photons and request prices in API documentation.
- https://lampyre.io/python-api-doc/lighthouse_intro/what_is_lighthouse_and_request.html
  — describes local Python Tasks; this is not a verified remote API protocol.

| Capability | Documented publicly | Implemented here | Tested |
|---|---|---|---|
| MCP offering | Yes, homepage | No transport | No live test |
| Account/API-key MCP authentication | Advertised only | No credential flow | No |
| API/MCP endpoints, schemas, tool names, scopes | Account docs required | None invented | Blocked |
| Pricing/balance endpoints, idempotency, cancellation | Not verified | No live implementation | Blocked |
| Provider registry and live refusal | Internal design | Yes | Native C++ |
| Budget reservations and duplicate prevention | Internal design | In-memory evaluation only | Native C++ |
| Synthetic errors/cancellation | Internal fixtures | Yes | Native C++ |
| Qt AR/EN panel and timer bridge | Internal design | Yes | Pending Qt CI/visual QA |
| Secure credential persistence | Existing DPAPI component | No Lampyre credential integration | Blocked |
| JSON/CSV/PDF and SHA-256 evidence exports | Existing case workflow | No Lampyre result linkage | Blocked |
| Hardware/production acceptance | No | No | Not verified |

## Security review

No HTTP client, endpoint, MCP discovery, shell execution, credentials, environment-secret
read or paid lookup is introduced. Provider output is rendered as plain text. No tool
response can invoke application actions. Synthetic output is bounded and fixed. No new
personal-data database is created. Current-account DPAPI is the existing protected-store
candidate; do not add a plaintext fallback or portable API-key file.

Before live enablement: verify official account docs and redistribution permission;
implement narrow tool/category allowlists, schema and output bounds, TLS and redirect
policy, durable identity-bound case/purpose/cost approvals, credential redaction, durable
reservation recovery, provider-authoritative cost reconciliation, access revocation,
retention/deletion and evidence/audit linkage. Avoid retries for chargeable submissions
unless the provider guarantees idempotency. A timeout or cancellation is not proof that
no charge occurred. No synthetic approval is permission to execute a live query.

## Trial plan and blockers

Trial activation, 14 days and 200 Photons are user-supplied offer details, not verified
account state. Do not activate as part of this change. After confirmation, days 1–3:
docs and access; 4–6: individually approved authorized sample queries; 7–9: provenance
and export; 10–12: UI/accounting/stability; 13–14: evaluation and commercial assessment.
Agree a reserve before the first paid test. Never put credentials in an issue or PR.

Required account documentation: authentication, request catalog/schema, price units,
balance and cost estimates, async/poll/cancel behavior, error/rate-limit contracts,
idempotency, MCP transport/tool list and retention/licensing restrictions. Upload a
redacted documentation export when available. No password or API key is needed for
continued offline development.

This is an offline evaluation increment, not the completed live integration. Live
adapter, MCP adapter, credential workflow, durable query/audit store, exports, full
Windows packaging and screenshots remain acceptance gates. Existing repair and case
storage schemas are unchanged. Keep the PR draft; do not merge main.

## Executed validation

Local GNU C++ 14.2 / CMake 4.4.4, GUI OFF, warnings as errors: configured,
built all core targets and passed **6/6 CTest suites**. Raw JUnit and configure
output are in `evidence/lampyre-2026-11-11/`. New suite covers registry duplicates,
live refusal, missing/negative cost, approval, exact budget, overflow, duplicate IDs,
uncertain holds, settlement, cancellation, synthetic data restrictions and errors.
Qt SDK is absent locally: no Qt build, RTL screenshot, packaging, physical Windows
or production claim is made. CI results must be inspected separately after publication.

CI code commit c94ea07344d50ad71497cfcfe745fb8d47c5ed84: commercial offline contract run 43021289721 passed on both Windows 2022 and Ubuntu 24.04. Full native Qt run 43021289665 and phone-provider run 43021289626 were still running when checked. This documentation-only checkpoint does not claim those results or a Windows artifact.

## OSINT workspace redesign

Replaced the stacked phone/Lampyre forms with a dedicated OSINT view inside the
analysis workspace, with a return action to existing evidence analysis. Overview
shows three provider cards, case context and explicitly unknown account balance.
Separate tabs contain provider selection/query controls, results/source status and
usage/history availability. PhoneInfoga keeps its existing case/permission path.
Lampyre connection, costs and synthetic laboratory are separate cards; raw JSON
is opt-in. No decorative graph, fake account balance or fake connection is shown.
Existing colored SVG assets, theme controls, AR/EN and responsive grids are reused.

Added bilingual four-tab Qt screenshot/navigation coverage and native synthetic
run/cancellation assertions to the existing UI suite. Local source-QML component
rendering used PySide6/Qt 6.12 with a synthetic case bridge; it is a component
preview, not a full Windows binary acceptance. Eight AR/EN tab screenshots were
rendered. Core regression CTest remains 6/6 passing. Windows/UI CI must be checked
for the redesign commit separately; earlier screenshots are not its evidence.


## Client-guided analysis workspace

See CLIENT_INSPECTION.md for the actual uploaded 1.10.4 package inspection,
extraction limits and independent design. Results now have request context,
sort/filter table, selected-item provenance and a zoomable/selectable synthetic
relationship view. No proprietary resources or decompiled implementation imported.
Local source-QML table/graph rendering and selection/filter assertions passed in
AR/EN. Core CTest 6/6 passed. Updated native Qt UI assertions and screenshots await
CI for this commit; no Windows runtime acceptance is inferred from local previews.
