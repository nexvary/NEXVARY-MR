# Signed Government operation-result receipts

The [case-bound transaction adapters](GOVERNMENT_CASE_TRANSACTIONS.md) use the same signed journal for local PCAP/EXIF and government registered-evidence partition analysis. The older RecoveryBridge limitation below remains historical/synthetic; scanner/update and other adapters retain their separate persistence limitations.

Development-evaluation extension of the existing signed policy journal and reviewed exceptions. It neither replaces that journal nor grants additional authority. Exact tested source, results and downloads are in DEVELOPMENT_STATE.md.

## Implemented coverage

The shipped SecurityBridge records native `security-scan` and `signature-update` worker dispatch/results. This covers the existing authorized local ClamAV-file scan and FreshClam update, including explicit cancellation, permission revocation, session closure and expiry. LawCaseBridge records the government `partition-analysis` workflow described below; historical synthetic RecoveryBridge fixtures retain earlier behavior. Other operations do **not** yet have these result receipts; their existing gate/result semantics remain separate.

## Government registered-evidence partition analysis

The current production government adapter is LawCaseBridge through Government Analysis: actual case/evidence UUID membership, original reviewed request/start binding, prepared intent before dispatch, signed candidate before SQLite case mutation and idempotent conservative restart reconciliation. See GOVERNMENT_CASE_TRANSACTIONS.md for exact protocol, bounds and unknown states. Its completed receipt output digest covers exact stored canonical `reportJson`. The governed production RecoveryBridge path refuses unbound execution. Scanner/update receipts still have their earlier persistence boundaries.

## Historical / synthetic RecoveryBridge partition analysis

RecoveryBridge records `partition-analysis` for explicitly authorized local raw images and a separately installed `mmls` executable. It uses the `external-analysis` category and the same reviewed approval, without adding permission or device acquisition. Source and output digests are mandatory for a completed receipt. The output digest covers the exact embedded `tsk-partitions.json` evidence payload, including existing provenance. Explicit evidence export writes those same bytes, so its SHA-256 can be compared with the separately retained signed receipt. General case JSON/PDF reports are different documents and are not assigned this digest.

For this audited path, the background worker produces a candidate and does not persist it. The GUI callback rechecks permission and cancellation before case persistence and terminal acceptance. A finished worker with a queued callback can still be revoked without publishing its candidate. Source, verified working copy, tool digest/version and bounded report checks remain unchanged; a version string/hash does not certify executable or dependency authenticity.

Case persistence and journal persistence are **two separate stores, not one atomic transaction**. A completed result is exposed only after both writes succeed. If terminal acceptance fails after a case write, the adapter attempts to restore previous case contents and records failure. A failed restore is reported; the old in-memory case is retained, but the disk case may already have changed and must be secured/reviewed. A crash between writes can leave a case candidate with an unfinished journal entry; restart classifies the operation as interrupted with unknown external outcome. No cross-store crash recovery or guaranteed rollback is claimed. The optional policy case reference does not prove image membership or equality with the recovery-case identifier.

An approved exception is not a started operation. An operation UUID is allocated only when an activated exception is checked and the signed start can be committed. `started` means entry into the native worker-dispatch path; it does not attest an OS process launch. Preflight validation can subsequently fail before an external executable runs. `completed` means the bounded native worker's checks succeeded; it does not independently certify the external executable, threat coverage, internet service or a phone acquisition.

## Record and lifecycle

`operation-start` and `operation-result` are hash-linked/signed events in the existing `nexvary-policy-audit/v1` export. Their exact-field `nexvary-operation-result/v1` records contain operation/request UUIDs, requester ID/role, the actual approval event sequence/hash, category, registered operation kind, declared case reference, locally observed start/completion UTC, status/failure classification and optional SHA-256 source/output observations. The start also has the requesting account's credential-unlocked attestation. A completion after logout is an explicitly attributed journal-signed system observation, never a fabricated human signature.

| State | Meaning |
|---|---|
| Approved | Existing exception decision; no execution or operation UUID inferred. |
| Started | Durable start committed before native worker dispatch. |
| Completed | Native worker succeeded and the terminal receipt committed. |
| Failed | Worker reported an error; `engine-failure`. After restart, an unfinished worker becomes `process-interrupted`: its actual external outcome is unknown. |
| Cancelled | Operator cancellation or ended session; `operator-cancelled` / `session-ended`. |
| Revoked | Original authorization was revoked; `permission-revoked`. |
| Expired | Original authorization expired; `permission-expired`. |

Revocation/expiry/session closure first stops permission and requests worker cancellation. The terminal receipt is written when the worker returns, rather than asserting immediate OS termination. A later grant cannot authorize an older operation. A completion observed after closure is classified by the original authorization and its verdict is refused. Restart never reruns or restores an operation; an unfinished record is failed with unknown external outcome. Already transmitted bytes, replaced databases, descendants and external side effects cannot be undone by these receipts.

No source path, filename, scanner output, credential, URL or evidence content is accepted in receipt fields. The optional case UUID is copied from the approved exception and is a declared reference; this increment does not enforce file membership in a case or confer case access. Source digests are reported only when the existing worker measured them. A completed scan requires the verified source digest; an update has no single evidence-source digest. Failed/cancelled operations may have no digest.

For a completed scan, output SHA-256 covers the exact UTF-8 compact JSON native report **without** `operationReceipt`. The existing explicit-consent JSON export writes those bytes, so ordinary SHA-256 tools can compare the exported report with the separately retained receipt. Private paths/output remain in that separately authorized report; only its digest enters the general journal. Updates hash their native result snapshot; no new general update-report export is added. Failed/cancelled output hashes, when available, describe the native callback snapshot, not a successful/exportable report.

## Independent verification and limits

Both the native verifier and separate Python/cryptography verifier validate decision sequence/hash correlation, requester/role/case/category/kind, strict receipt fields/digest syntax, unique start, one terminal result, immutable bindings and closure-compatible outcomes. Unknown extra content fields, terminal-without-start, duplicate completion, forged decision references and success after revocation are rejected even when test histories have valid signatures. An export ending at `started` is an incomplete history, not completion; the Python summary reports the observed operation states. Use the existing independently retained key/prefix anchor protocol in GOVERNMENT_POLICY_AUDIT.md. Old histories remain readable; older verifier versions that do not recognize these event kinds fail closed on new histories.

This is first-party semantic and cryptographic verification, not an independent laboratory certification of physical events. Local accounts/time and current-account/process trust limits remain unchanged. Wall-clock rollback is not treated as trusted duration; event order is established by sequence/hash links. Original approval controls, case grants and compiled tool restrictions remain independent.

Capacity remains 8,192 events / 16 MiB; at most 64 unfinished operations are tracked. Government Administration shows up to 32 recent visible-to-owner/reviewer receipts and clears them on logout. Policy/write/capacity failures prevent a new dispatch or make its result unavailable; stopping never depends on successfully writing its final event. A broken/full store can prevent its own terminal receipt. Scalable rotation, complete recovery, all-engine completion coverage, OS containment and institutional key/time/identity custody remain open gates.
