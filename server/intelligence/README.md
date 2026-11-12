# Nexvary Intelligence billing

The initial price is 1,400 EUR cents for one confirmed search. One search maps
to one provider request. Configured prices apply to future quotes, and completed
receipts retain their original prices. An offer expires after five minutes. A
price change invalidates an unused quote at confirmation.

## Integration

Mount `IntelligenceAPI(Billing(...))` in a private WSGI service behind Nexvary's
authenticated HTTPS gateway. The gateway must authenticate callers, set
`REMOTE_USER` in the WSGI environment, and apply rate limits. The application
ignores HTTP identity headers. Inject organization-specific `may_search` and
`is_admin` checks backed by the trusted service authorization and case policy.
Keep the SQLite ledger and its backups private to the service account. The
checkout contains no deployment address, account entitlement or provider keys.

The production adapter must supply the licensed, verified capability catalog
and an account-specific EUR estimate including operating costs. Availability
must be a fast local check. `execute` must enforce the supplied provider ceiling
and use the operation ID as the provider request reference. When a binding
provider ceiling cannot be enforced, return no cost estimate for that capability.
Provider credentials belong in the backend secret store. Reconciliation requires
provider evidence and must use the original operation ID. A lost reply leaves
the reservation pending for operator review.

| Method and route | Use |
| --- | --- |
| GET /v1/catalog | Localized capabilities, availability and the current EUR price for available cards |
| GET /v1/price | Current EUR price in integer cents |
| POST /v1/quote | Category and arguments, checked against authorization, availability and cost |
| POST /v1/confirm | Same category and arguments, quote ID, idempotency key and explicit confirmation |
| GET /v1/receipts/{id} | Read the caller's durable receipt |
| PUT /v1/admin/price | Set the selling price and cost ceiling |
| POST /v1/admin/credit | Apply a verified prepaid credit using a unique payment reference |
| POST /v1/admin/reconcile | Resolve a pending operation from provider evidence |

The catalog and price responses use `Cache-Control: no-store`. The client
should request a fresh quote immediately before showing confirmation. It must
reuse the same idempotency key for transport retries and poll the receipt if
the operation is pending. Provider costs and margins stay in the private audit
table. A definitively failed operation restores the customer's prepaid credit
once. Overruns are recorded in the private audit trail for review.

## Deployment status

The new server module is tested with synthetic providers. The existing Windows
MCP path still uses a customer's direct provider account with its existing
authorization and consent. The Windows app has no configured Nexvary billing
gateway or production catalog in this checkout. Enable commercial cards only
after connecting the application to that gateway and validating a production
adapter, entitlement and result delivery.
