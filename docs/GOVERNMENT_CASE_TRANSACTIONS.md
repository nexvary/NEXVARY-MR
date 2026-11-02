# Crash reconciliation of case-bound local analysis

This increment integrates existing **classic-PCAP / JPEG-EXIF case analyses**
with the existing signed government policy journal. The registered raw-image
partition adapter below now uses that same case store and journal. Scanner/update
and other import/export adapters do not inherit this recovery protocol.

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
| At least expected revision + 1, original analysis audit position/actor, one matching operation/analysis UUID, source/parser/report digest and exact stored report bytes | `completed`: that local case mutation is present |
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
identify. A later changed case with no matching retained result is conservatively unknown
if its terminal receipt was never written; a matching retained result can survive
later legitimate revisions under the bidirectional proof gate below. No automatic unknown-resolution override is introduced.

Windows production DPAPI is still required; Linux/native crash fixtures are
synthetic. Institutional identity, external key/time custody, independent review,
power-loss testing, clean Windows 10/11 and physical-device acceptance remain.
## Registered raw-image partition transactions

The Government Analysis page accepts **an evidence UUID from the active case**,
a trusted separately installed native `mmls`, and explicit consent. It reads and
hash-verifies the protected evidence payload; a text case reference or arbitrary
image path cannot establish membership. The original reviewed `external-analysis`
exception must reference this exact case UUID and belong to the active requester.
The production governed RecoveryBridge path refuses unbound partition dispatch;
professional use without government policy remains separate. Historical synthetic
RecoveryBridge negative/cancellation/descendant tests are retained, not silently
counted as tests of the new adapter.

`nexvary-case-operation/v2` binds the original signed operation-start/request,
case/evidence/revision/pre-case/source, reserved analysis UUID, requester/role/UTC,
and preflight executable digest. **Prepared intent is acknowledged before any
external tool launch.** The initial report digest is empty because no result exists.
The existing TSK engine runs asynchronously on a private copy, verifies source,
working copy and executable again, and returns a bounded report. The signed
`candidate` transition sets its report digest exactly once, before the SQLite
case write. The candidate changes no other binding. The original approval and
local role/session/case grant/revision are rechecked after each journal boundary.
Revocation and a newly approved exception cannot authorize an old worker.

The result, analysis/operation IDs and existing case audit commit through the
same SQLite transaction as the local parsers. Case reconciliation then writes an
immutable terminal observation. A completed external receipt must match the
completed case observation's source/report digests. Success is withheld if the
receipt cannot be accepted; there is no best-effort restoration of an old case.
The report digest covers compact canonical JSON stored in `reportJson`, not a
separately exported PDF or the older RecoveryBridge `tsk-partitions.json` format.

| Durable crash observation | Recovery decision |
|---|---|
| Prepared, without a signed report candidate | `unknown`: external execution/outcome is not established; block for review |
| Candidate, original case revision/digest and verified evidence unchanged | `failed` local case write; no external replay |
| Candidate and exactly matching committed case analysis | `completed` local case mutation; never invent completed external execution on restart |
| Changed case/revision, corrupt evidence/report/journal or ambiguous linkage | `unknown`, fail closed |

Explicit worker errors/cancellation observed in the live adapter record a failed
local mutation and the applicable failed/cancelled/revoked/expired worker receipt.
Restart interruption receipts remain interrupted/unknown external outcome even
when a local case mutation is reconciled as present. Repeated recovery adds no
analysis, report or event. No tool dispatch occurs in recovery.

Bounds: registered raw evidence 512 bytes–16 MiB; report 64 KiB; captured output
8,192 bytes per tool invocation; executable hashing 256 MiB; 60 seconds per version
or analysis invocation plus bounded existing startup/kill waits. These are input
and timeout limits, not measured throughput or memory guarantees. Temporary copies
are plaintext with owner permissions; crash traces/secure erasure remain external
storage concerns. This **does not fix process-tree containment**: the existing
TSK parent-only cancellation limitations and retained detached-child counterexamples
remain open. Scanner/update transactions, product descendant cleanup, intermittent
audit-store diagnosis, GUI soak and measured performance remain next work.


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

## Bidirectional retained proof gate — 2026-11-02

Startup now reconciles unfinished operations and then walks **both** the signed
terminal intent inventory and the protected case inventory. The native readiness
gate repeats that check before exposing readiness or permitting case actions.
Each operation-linked analysis requires a completed signed intent, unique operation
and allocated analysis identifiers, the same actual case/evidence/parser/requester,
verified protected source bytes and SHA-256, and the exact bounded stored report
bytes/digest. Partition reports also retain the signed executable digest. New
results retain the immutable intent bindings inside protected case state; the
signed intent remains the authority, never caller labels or a later grant.

A completed intent must have exactly one matching retained result. Duplicate,
missing, rebound or changed results, unreadable cases and lower/conflicting
revisions fail closed. Later legitimate metadata/custody/retention revisions are
allowed: reconciliation checks the original analysis audit position and actor,
the retained analysis and current source/report integrity; it no longer requires
the *current* revision to equal the prepared revision plus one. Unfinished intents
without a committed result retain conservative pre-state/unknown decisions.
Unlinked legacy analyses remain unlinked and are **not** retroactively verified.

A mismatch with an otherwise usable signed journal records one strict
`case-reconciliation` system observation, schema
`nexvary-case-reconciliation/v1`, containing only the aggregate stores SHA-256 and
`outcome: unknown`. It authenticates a mismatch observation, not the orphan's
successful execution or the completeness of the stores. Native and separate
Python verifiers reject extra fields, false success, bad digests, human attribution
and duplicate reconciliation observations. The workspace stays blocked while an
unknown observation exists. Repeated startup adds no new result/event and never
replays a tool, changes a previous terminal, deletes a result, restores either
store or invents an intent. There is no automatic unknown-resolution override.
If the journal itself cannot be verified/written, readiness remains blocked;
a signed decision cannot be promised when the signing store is unavailable.

Retention currently records holds/archive/disposal review **without physical
source or analysis deletion**. Approval is not a tombstone: a missing completed
result is therefore unknown even after an approved disposal review. A future
physical deletion protocol must introduce separately verified, signed retention
proof before relaxing this rule. Case-only restores with missing corresponding
journal history must be reviewed, not silently accepted.

The inventory reads at most 1000 cases, one bounded record at a time (4 MiB input,
existing 3 MiB writer limit, 100 analyses/case). Oversized SQL payloads are refused
before loading them. Source reads keep the existing 8 MiB local / 16 MiB partition
limits and 64 KiB report bound. No aggregate case/evidence corpus is materialized.
Full readiness scans have a CPU/I/O cost that is not yet benchmarked for large
corpora; these limits are not measured throughput/latency claims.

The reproduced pre-fix counterexample is an old **correctly signed** journal
snapshot that omits a committed case analysis's intent. Its baseline failure is
preserved under `evidence/cross-store-2026-11-02/`. Process interruptions around
preparation, case/candidate/terminal writes and the new mismatch-decision write
remain tested. These are process exits, not power-loss/storage-controller tests.
Rolling **both** stores back to an older mutually consistent state still needs an
independently retained trusted anchor to detect; the aggregate observation digest
is not that external anchor. This change does not resolve the historical Audit
Store lag, detached descendants, GUI root cause, NTFS case integration or any
institutional acceptance gate.
