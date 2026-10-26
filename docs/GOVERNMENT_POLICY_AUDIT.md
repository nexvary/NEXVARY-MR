# Signed Government policy decisions and reviewed exceptions

This is a bounded development-evaluation implementation within NEXVARY-MR, not
institutional accreditation. Exact source, CI and deployed verification are in
DEVELOPMENT_STATE.md. It extends the existing native application operation gates;
it does not create an OS sandbox, a new product, an institutional identity provider
or a device-acquisition capability.

## Operator workflow

1. An existing local administrator provisions a different reviewer or administrator
   account through Administration / local identities. No production credentials
   are automatically created. Different accounts do not prove different people.
2. A signed-in administrator, investigator or repair technician requests **one**
   category, a reason, an optional case UUID reference and 1–600 activation seconds,
   with explicit consent. The request remains blocked. Duplicate open requests for
   the same requester/category are rejected. Auditors, trainers and reviewers do
   not obtain requesting authority merely by viewing this panel.
3. A different administrator/reviewer signs in, starts review with a reason and
   approves or rejects with a decision reason. Only that assigned reviewer may
   decide it; neither self-review nor self-approval is permitted. Approval does not
   start a tool, enable a permission for the reviewer or confer access to a case.
4. The original requester signs in and explicitly activates the unused approval.
   Activation verifies that the approving account/key is still enabled and has its
   recorded role, and that the requester still has the original role. Existing
   device/source/owner consents, compiled command restrictions and case grants
   remain independent requirements. The first activation consumes the approval.
5. Revoke a pending approval through the request controls, or stop an active
   category immediately. Active stop remains possible without an attributed
   authenticated human; it is then a signed **system observation**. Changing
   identity/logout, observed reviewer invalidation, monotonic expiry or process
   restart also closes active permission. Regrant never resumes old operations.

An unactivated request/review/approval expires ten minutes after its locally
recorded creation time. Pending validity uses the local UTC clock, including
across restart; clock rollback can affect that window. Active lifetime uses a
monotonic clock, capped at ten minutes, and never survives process restart.
The event-loop monitor checks at most once per second; each new permit call also
checks. Stalled processes, already published bytes or started device changes
cannot be recalled. The duration override in native tests shortens active lifetime;
it is not a QML/production setting that extends the limit.

**Enforced scope is a category and requesting principal.** It is not an origin,
executable/hash, device, case or destination allowlist. The optional case UUID is a
recorded reference, not case access validation or a restriction of the category to
that case. Do not treat descriptive reasons as executable policy constraints.

## Durable records and signatures

`local-identities.dat.policy-audit` is an independently versioned sidecar; no case
schema or case evidence is migrated. One atomic protected snapshot contains the
bounded history and a randomly generated journal signing key. Windows uses the
existing current-account DPAPI protection; non-Windows production fails closed.
The explicit native synthetic profile supports test data only. The process holds
its journal key in memory; secret buffers are wiped on destruction, but this is
not hardware key custody or a guarantee of complete memory erasure.

Each event signs exact UTF-8 bytes with Ed25519 using the existing pinned
libsodium dependency. The payload binds journal UUID, sequence, previous event
SHA-256, action, request snapshot, result, locally observed UTC time and principal
scope. Its digest covers payload plus signature. Human requests/reviews/decisions,
activations, authorized request revocations and in-app audit verifications also
carry the existing credential-unlocked user's Ed25519 attestation over the exact
decision body. System events (expiry, logout/restart/emergency stop, native gate
outcomes and rejected API observations) use the journal signature and explicitly
identify their principal as `system-observation`; the application never forges a
logged-out user's signature. Local actor proofs bind the declared ID/role/key;
they are not agency identity certificates.

Recorded states are Request → Review → Approve/Reject → Activate → Revoke/Expire,
with independent closure records and denial observations. Request snapshots retain
requester, category/scope, reason, duration, optional case reference and local
creation/deadline. Later events retain reviewer/review/decision reasons and
activation/expiry/closure times. The verifier rejects changed immutable scope,
self-review, wrong reviewer, invalid signatures, changed actor binding,
reordering, interior deletion/duplication and mismatched retained history.

`operation-allowed` and `operation-denied` record the **native gate decision**.
They do not assert external-tool success, the absence of network packets or a
completed forensic acquisition. Browser results additionally record rejected URL,
failed handoff or accepted OS handoff; subsequent browser activity is uncontained.
Existing tool/analysis/case result stores retain their own completion semantics;
this increment does not unify every external operation's completion into this
journal. Independent verification outcomes are themselves durably signed when
run through the application's audit controls. The standalone verifier writes a
separate result and has no signing key.

### Failure and concurrency

Bounded regular-file reads, same-writer file digest checks, QLockFile and atomic
QSaveFile commit protect normal application updates. An outside write/deletion,
stale second writer, corrupt protected file, signature/state failure, write/lock
failure or capacity exhaustion denies exceptions. Active permissions are stopped
when failure is observed; stop is never conditional on successfully writing an
audit event. A broken/full store may therefore prevent persistence of its own
final stop/failure record. There is no silent repair, reset, overwrite-export or
rotation command. Preserve the original file and investigate independently.

Limits: 8,192 events, 16 MiB snapshot/export, 32 KiB/event, 64 open requests,
256 UTF-16 reason characters and 32 visible recent requests. Every permit decision
writes a snapshot; sustained high-throughput services need a reviewed scalable
journal/rotation design. These bounds are not a claim of long-term records-system
capacity. Snapshot rewriting/atomic commit is not guaranteed stable-media flush
across hardware/power failure. There is no automatic policy/identity disaster
recovery in a case-only backup.

## Independent export and retained anchors

An administrator/reviewer/auditor explicitly consents to an **unencrypted** export
to a new local file. It contains reasons, local actor identifiers and case UUID
references, never the signing secret, credentials or raw case evidence. It inherits
the destination's security context; the application rejects direct UNC-style paths
but does not identify OS-mounted network filesystems. Export does not authorize
publication. Preserve the shown `{journalId, publicKey, sequence, hash}` anchor in
an independently controlled location/channel, separate from the journal/export.

The GUI verifies an export against a supplied retained anchor and records the
outcome. For a separate implementation without Qt, registry/database access or
private keys:

```sh
python -m pip install cryptography
python verification/verify_policy_audit.py audit.json --anchor retained-anchor.json
# In a source checkout: python scripts/verify_policy_audit.py ...
```

The Python verifier uses `cryptography` Ed25519 verification, rejects ambiguous
JSON/base64 and validates exact signed event/actor bytes, lifecycle and retained
prefix. Windows CI pins its separate test dependency to 50.0.2; Ubuntu uses its
separately packaged Python cryptography dependency. This is a verifier dependency,
not bundled Python code executed by the GUI or an added runtime network request.

A retained newer sequence detects a truncated/rolled-back valid history; a retained
prefix digest detects a different earlier prefix. An anchor copied from the same
untrusted export establishes no independent trust. Sequence-zero anchors pin only
the journal identity/key, not completeness. Deleting/replacing **the whole store
and key** or replaying an older valid protected snapshot after application restart
cannot be detected without an independently retained key/history anchor. Current
application startup does not enforce a separately provisioned institutional
revision floor. A compromised current Windows account/process can access DPAPI and
keys and may forge a new history. No external trusted time, HSM, multi-machine
consensus, institutional identity, independently controlled reviewer or OS
administrator resistance is claimed.

## Verification evidence

`mr_policy_audit_tests` covers requesting-role/consent/input negatives,
self/wrong-reviewer refusal, one-time requester activation, rejection/revocation,
monotonic expiry/logout, disabled reviewer refusal, export authorization and
no-overwrite, signature/order/truncation changes, outside-file/stale-writer failure,
and a real child process that exits without destructors before restart closure.
`mr_policy_audit_interop` produces native signed synthetic history and checks it in
Python, including independent-key/anchor/order/deletion/signature negatives and
standalone CLI execution. This is an internally authored interoperability corpus,
not external institutional review. Existing offline/cloud/mirror tests now obtain
permissions through the complete real reviewed-exception APIs.

Qt Quick clicks request/review/approve/activate/stop controls in AR/EN and checks
responsive/DPI layouts. Deployed portable/installed Windows smoke must use the
same reviewed protocol, export an actual signed synthetic history and anchor,
verify in native code and independently through Python, then publish receipts and
real screenshots. Exact passing/failing results must be pinned in
DEVELOPMENT_STATE.md before a hosted/deployed verification claim.

Remaining: institutional identity/key/time custody, audited rotation and recovery,
per-tool/destination restrictions, comprehensive completion receipts, OS egress/
DNS/proxy/descendant/mounted-share tests, clean physical Windows 10/11, independent
security/records/privacy/license review, publisher signing and release approval.
No competing licensed product was executed; no comparative superiority is proven.
