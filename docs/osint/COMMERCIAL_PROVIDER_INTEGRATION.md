# Commercial OSINT providers — implementation checkpoint

Branch: `feature/osint-commercial-providers` (private repository).
Status: contract scaffold only. No live API adapter, credential storage, Qt UI, paid lookup, or vendor approval is claimed.

## Architecture
- Keep existing PhoneInfoga isolated; do not change its lifecycle or case/audit behavior.
- Add independent provider adapters behind a common contract, not vendor-specific logic in QML.
- Vendor adapters must require a case ID, named operator, authorization reference, permission for external disclosure, and explicit cost confirmation.
- Fail closed on missing API credentials, missing OEM/API permission, unknown prices, rate-limit responses, or offline state.
- Never silently retry billable calls. Distinguish request submission from provider completion; retain vendor request IDs for deduplication.
- Preserve source attribution, UTC collection time, SHA-256 of the original response, provenance warnings, and independently verified status in the existing evidence/audit engine.
- Store credentials in OS-protected storage, never QML, logs, exports, source control, or portable configuration.
- Minimize personal data and respect legal retention/deletion controls.

## OSINT Industries
Official POST endpoint: `https://api.osint.industries/v2/request`.
Header: `api-key`. Body: `type`, `query`, `timeout`, `exact_match`, `premium`, `premium_modules_only`.
Supported documented selectors: email, phone, username, name, wallet.
Documentation: https://docs.osint.industries/reference/search-1
Terms: https://www.osint.industries/terms-conditions
IMPORTANT: a paid API subscription is not by itself an OEM/resale grant. Confirm contract/order and price per module before enabling. Never send real case data during development.

## Lampyre
No production endpoint or authentication format is assumed here. Obtain official developer documentation and written commercial integration rights first. Implement a mock provider before enabling the network adapter.

## Acceptance gates
1. Unit tests for all denied states and authorization/cost transitions.
2. Provider contract mocks with zero network requests.
3. Redacted HTTP fixture tests for authentication, timeout, cancellation, 429, 402, 5xx, and malformed responses.
4. Consent and audit tests verifying no remote transmission before approval.
5. Qt bilingual AR/EN UI and credit estimate with separate approval action.
6. Authorized sandbox integration tests with vendor keys and contract.
7. Windows CI, packaged build and clean machine acceptance.

Do not label either commercial provider integrated until these gates pass.
