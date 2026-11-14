# Unified Intelligence transport

Phone, email and username cards now default to the documented Data Lookup API. The existing MCP adapter remains available through the explicit specialized-service toggle. Both transports reuse the existing credential protection, identity checks, case authorization, operation receipts, invoice store and network manager.

Contract: https://account.lampyre.io/api/1.6/data-lookup-doc/api.yaml
- Read-only authentication: `GET /find/history/by-token`; accept HTTP 200 JSON history or HTTP 204 empty history, discard the private history body.
- One paid dispatch: `GET /find/by-token` with the documented `token` and exactly one of `phone`, `email`, `username`.
- Collect the same request: `GET /find/{request_uuid}/by-token`. Status 1 remains pending, 2 completes, 3 fails; HTTP 204 means empty results. No MCP fallback and no automatic paid resubmission.
- Documented response sources are retained separately, including nested `data`. A `payment_required` source stays locked and never triggers extra payment.

The owner reviewed the provider web display on 2026-11-14: phone 35, email 30, username 15 Photons. Provenance: `https://account.lampyre.io/chunk--J6cvFuO.js`, SHA-256 `b0ce2468302a2892a5f759120f3fe6b2e610fd1a567ec39e4392b50fd13565c7`. These are internal quoted supplier costs, not a server billing receipt or a guarantee that future provider rates will stay unchanged. Customer invoices remain EUR 14 due/unpaid, separate from Photons. Actual supplier debit is left unconfirmed without a receipt.

Persist the invoice intent before dispatch and protect the returned request UUID in the same encrypted owner ledger before collecting. A lost/malformed dispatch response is uncertain and is marked for review; do not resend it automatically. A known request can be collected after reopening without a new paid dispatch. Collection polls are bounded; cancellation stops local waiting, not the remote job. The provider publishes a side-effecting GET without an idempotency key: this client cannot promise server-level exactly-once billing after an ambiguous network failure.

Secrets in the required query parameter are percent-encoded; URLs are never exposed to QML or logged. Redirects, response size, TLS failures and timeouts use the existing fail-closed network handling. The phone's plus sign is explicitly encoded as `%2B` to preserve the international identifier.

Live read-only history authentication returned HTTP 204 with the owner's supplied key. No paid phone/email/username Data Lookup request was executed to validate this increment. Windows fixture results are not live intelligence, and do not establish physical installer acceptance. The owner approved creation of one production Windows Setup EXE on 2026-11-14 after interface review; no Portable or Laboratory package is requested.
