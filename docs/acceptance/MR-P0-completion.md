# P0 repair candidate — 2026-11-13

Base: integration/mr-intelligence-final-ux-pricing, 8705543b23ff81a67def02675bc1f91b28304539. Reuses existing identity, policy, protected case storage, MCP transport and Windows workflow. PRs 18/19 were inspected; they are draft predecessors. No main merge or paid request.

| Requirement | Status before repair | Evidence | Action / acceptance |
|---|---|---|---|
| Authentication | Implemented; current candidate needs native verification | IdentityService; identity_tests; previous base Windows CI passed | Preserve account schema, first-admin sign-in and role ceilings; no silent password-derived session restoration |
| Permissions | Implemented; spending ceiling missing | IdentityService; GovernmentOperationPolicy; policy/identity tests | Keep independent approvals; add administrator-only persistent submission ceiling |
| MCP integration | Protocol fixtures implemented; live unverified | LampyreConnection; loopback JSON/SSE tests | Centralize fixed production destination; remove QML URL API; keep non-billable discovery, timeouts, TLS validation and no automatic paid retries |
| Case management | Implemented; current Windows verification pending | LawCaseBridge/Store; case transaction/workspace tests | Preserve storage; require current case access for production result export and attachment |
| Intelligence graph | Synthetic view only; real mapping blocked | OsintAnalysisPane uses opt-in evaluation records only | Do not infer provider relationships without documented live result schema; real graph/save/reopen NOT IMPLEMENTED |
| Branding | Broken | Provider name in setup, source labels, exports and packaged guide | Nexvary display labels; sanitize untrusted provider display strings and exported result; no endpointHost in export |
| Windows packaging | Existing workflow; current verification pending | .github/workflows/cpp-qt.yml | Trigger existing workflow through draft PR; retain production/laboratory suites and installer lifecycle smoke |

## Credit protection

The existing independently reviewed administrator-only `paid-unknown-cost` grant remains mandatory, as do explicit consent, native operational roles, case authorization and the session submission limit. A new per-investigator protected remaining-submission allowance defaults to zero. Administrators set/replenish it from the existing account panel. It is atomically decremented under a file lock BEFORE dispatch; a timeout, cancellation, credential replacement or restart cannot refund/reset that allowance. Failed/uncertain requests remain counted. This is a submission ceiling, NOT a Photon/currency ceiling. Exact costs/balances and provider request/consumption identifiers remain unavailable without a verified provider contract; the previous unsupported EUR price is removed.

No existing identity, case or credential path is moved/deleted. Legacy credential files can be opened, but production always selects the internally fixed official destination. Endpoint overrides remain native-only and production rejects alternate destinations. QML receives credential configuration only. Raw provider result display/export is sanitized; it is an unverified display copy rather than an assertion that the provider returned Nexvary-owned data. No proprietary desktop binary is bundled. Required third-party legal notices are retained.

## Verification gates

- Local: 53 Python tests (49 existing + 4 focused branding/configuration/access checks); core CTest 6/6 passed before the additional native Qt regressions.
- Added native regressions: hidden meta-property/method, fixed endpoint validation, provider/secret display redaction, export redaction, administrator-only persistent allowance, cancellation, unsupported protocol. Extended QML advanced-settings screenshot/regression.
- Native Qt/Windows tests, package checksums and exact-commit artifacts: must be recorded after CI finishes. Local environment has no Qt SDK. Prior base artifacts are not deliverables for this candidate. The official REST OpenAPI was reachable; the account Swagger YAML path returned 404 and an unauthenticated MCP GET returned 405. These are not evidence of accepted MCP credentials or protocol negotiation.
- Live authentication, invalid-vs-expired credential differentiation, real provider schema/relationships, exact spending caps, verified balance/pricing, clean Windows 10/11 functional acceptance and physical device testing remain NOT VERIFIED. No credential was supplied or sent to CI. New operation receipts use `intelligence-mcp`; legacy signed receipt identifiers remain verifiable without rewriting history.

The inherited setup Loader was restricted to administrators, making native account/tool support unreachable by investigators. It now admits authenticated administrator/investigator/reviewer roles; investigator search opens the same native panel. Backend permission, independent approval, active-case and credit checks remain authoritative. Added native QML investigator reachability and denied-unconfigured-search regression.
