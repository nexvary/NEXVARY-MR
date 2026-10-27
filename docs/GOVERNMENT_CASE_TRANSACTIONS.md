# Crash reconciliation of case-bound local analysis

This increment integrates existing **classic-PCAP / JPEG-EXIF case analyses**
with the existing signed government policy journal. It is a bounded first
transaction adapter, not a claim that the partition/scanner/update adapters or
all imports/exports have cross-store recovery. Those older paths remain unchanged.

## Protocol and authorization

The native LawCaseBridge checks the authenticated local role and current case
ownership/grant, reads protected case evidence and verifies its SHA-256, then
uses the existing parser. Before changing a case, LawCaseStore builds an exact
`nexvary-case-operation/v1` intent and the existing PolicyAudit durably replaces
its protected signed snapshot. No case write occurs if preparation fails.

The intent binds operation UUID, newly allocated local authorized invocation
reference (`requestId`), actual case/evidence UUIDs, expected revision, source
SHA-256, exact pre-write case digest, report digest, allocated analysis UUID,
registered parser, requester/role, prepared UTC and recovery state. This invocation
reference is **not** an external court order or a reviewed external-tool exception.
Local analysis does not gain external-tool/network permission. No paths, report
content, evidence bytes or free-text names enter the general journal.

The adapter rechecks the session and current grant after preparation. SQLite
then changes the protected case snapshot and its existing audit in one transaction
with `synchronous=FULL`, matching revision, original case digest and source bytes.
The analysis binds the allocated UUID and operation UUID. Finally, a journal-signed
system observation records the result. Human preparation carries the existing
credential-unlocked actor attestation; recovery never impersonates that actor.

## Restart decisions

Before the case workspace is used, the application verifies signed history,
examines unresolved intents and reads/verifies the actual case audit/evidence.

| Observation | Recovery state |
|---|---|
| Exact original revision and complete pre-write case digest, verified source | `failed`: the local case mutation was not committed |
| Exactly expected revision + 1, one matching operation/analysis UUID, source/parser/report digest and exact stored report bytes | `completed`: that local case mutation is present |
| Missing/corrupt case, evidence mismatch, different revision, missing/duplicate analysis or other ambiguity | `unknown`; case workspace blocked for review |

No recovery reruns a parser, copies evidence, regenerates a report, restores an
older case or launches an external process. Completed recovery observations are
immutable; identical API retries add no event. A second startup sees the terminal
observation and adds nothing. The signed lifecycle verifiers reject missing
preparation, duplicate terminals, changed bindings and private extra fields.
An observation authenticates what this implementation concluded; independent
case inspection is still required to verify the physical facts.

## Verification and limits

The native regression suite abruptly exits subprocesses before preparation,
during its QSaveFile replacement, after preparation, inside the uncommitted
SQLite case write, after the case commit, during terminal journal replacement,
and after terminal commit. It checks actual reopened case revisions/counts,
journal decisions and repeated reconciliation. This tests **process interruption**,
not sudden power failure, controller caches or storage firmware guarantees.
Synthetic-only native hooks are ignored in production profiles and unavailable
to QML. Original journal contention/symlink and QML crash tests remain enabled.
The independent Python-authored signed corpus tests both native and Python
verifiers, including invalid histories with otherwise valid signatures.

Existing limits remain: source 8 MiB for these parsers, report 64 KiB, 100 analyses
per case, 10,000 case audit entries, at most 64 unfinished case intents, policy journal 8,192 events / 16 MiB. Journal
capacity/read/write failure refuses new preparation and acceptance. A terminal
write failure blocks the workspace until restart/review. A cryptographically
valid whole-store rollback still requires independently retained anchors to
identify. A later changed case is conservatively unknown if its terminal receipt
was never written. No automatic unknown-resolution override is introduced.

Windows production DPAPI is still required; Linux/native crash fixtures are
synthetic. Institutional identity, external key/time custody, independent review,
power-loss testing, clean Windows 10/11 and physical-device acceptance remain.
Remaining first-priority migration: the separate RecoveryBridge partition-result
store needs a real case/evidence association and this transaction protocol; its
optional text reference remains insufficient. Product descendant containment,
audit-store intermittent diagnosis, GUI soak and measured performance follow.

## Audit-store post-replacement acceptance

The original intermittent synthetic export failure was reproduced locally as
`audit-digest`: the signed on-disk prefix held 18 events while diagnostics reported
22 in-memory events. Original private fixtures were preserved; the sanitized
observation and first failures are under `evidence/case-transactions-2026-10-27/`.
The cause of the snapshot lag remains unresolved. Atomic replacement success
now additionally requires exact bytes to read back through the existing bounded
no-follow input path before an append is acknowledged. A deterministic test
restores a valid older snapshot in the synthetic-only post-replace hook and
proves fail-closed refusal, no retry/overwrite and no false acknowledged event.
This gate detects immediate post-replace divergence; it cannot prevent or repair
a later external rollback or prove power-loss durability.
