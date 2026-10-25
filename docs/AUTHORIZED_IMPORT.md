# Authorized case evidence import — bounded native adapters v1

Within the existing Government workspace, register a lawfully available local file
as protected case evidence, open Case evidence analysis, and choose Authorized
evidence import. Select the source and adapter, record its purpose, review the
plaintext-copy notice and confirm authority. Import does not collect a phone or
establish legal authorization. Existing classic-PCAP/JPEG-EXIF summaries are kept.

| Adapter | Implemented observations | Deliberate limits |
| --- | --- | --- |
| UTF-8 document | Numbered raw text lines, including Arabic | Plain UTF-8 only; no PDF/DOCX/OCR or executable/document rendering |
| JSON | Sorted object keys, array indexes and typed scalar values | Object/array roots, depth 32, duplicate decoded keys rejected |
| XML | Qualified element/attribute names, namespace URI, text and line references | UTF-8, depth 32; DTD/entity declarations rejected, no external resource resolution |
| CSV | Numbered records/cells, quoted commas/newlines/escaped quotes | Strict quotes, LF or CRLF, 128 columns and bounded records; no formula execution |
| SQLite | Sorted table inventory and first 32 rowid-ordered rows/table, first 8 columns | Standalone rollback-mode database, 32 tables; views/virtual/shadow/WITHOUT ROWID tables skipped; BLOB SHA-256 only |
| Android exported log | Numbered raw threadtime lines and section markers | No inferred year, timezone, event meaning or timestamp normalization |

Sources are capped at 4 MiB. Reports retain up to 128 findings, references up to
512 and values up to 256 UTF-16 code units, with explicit truncation flags; the
parser report is capped at 60 KiB and protected record at the existing 64-KiB
analysis limit. Large/unsupported input remains registered and unchanged. These
are bounded observations, not a complete device/file acquisition. Encrypted
backups, vendor backup containers, recursive directories/archives, WAL sidecar
collection, arbitrary log formats and PDF/DOCX extraction are not implemented.
Supply a lawfully prepared checkpointed standalone SQLite copy; the application
never checkpoints, unlocks or modifies the original database.

## Processing and trust boundary

The native bridge enforces current role and case ownership/grants, explicit
consent and a bounded purpose. It reads the existing protected evidence only after
SHA-256 verification. A private temporary working copy is created with restrictive
owner permissions, rehashed, made read-only and sent to the fixed sibling
`mr_evidence_import_worker` executable. The worker independently verifies its input
hash and size before parsing, without a shell, device tool, rendering engine or
remote resource loader. Qt/SQLite parsers are original integrations using the
project's existing Qt runtime; no commercial forensic parser is copied.

The child has a five-second wall deadline, bounded output and owned termination.
Cancellation, child failure, invalid report and changed working-copy bytes fail
closed. The bridge rechecks protected evidence, identity/session/grant, active
case, actor and optimistic case revision before retaining output. Changing cases
or signing out cancels and discards pending output. Concurrent edits reject stale
results. Parser success does not authorize another device operation.

This is process error isolation, **not an OS sandbox or a guarantee of no network
access**. Offline network enforcement is a separate pending Government task.
Host Qt libraries, plugins, application directory and the OS remain trusted.
Owner permission calls are not an independent Windows ACL/privacy certification.
Temporary plaintext is removed on normal completion/cancellation, not securely
erased; a host/system crash can leave traces in the OS temporary directory or
paging/dump storage. Agency deployment must review encrypted workspace and OS
controls. The exact source evidence is never passed to the parser for in-place use.

## Provenance, audit and persistence

Each finding retains its source reference and declared value/type, with invisible
bidirectional controls escaped for display. Parser output contains source SHA-256,
source size, adapter/schema/version, deterministic ordering, limits and truncation
flags. It excludes host paths, wall time and random identifiers. The same source,
adapter, purpose and protected provenance on the same build produce the same
report digest. Audit envelopes separately retain the authenticated actor and UTC
time; file content or mutable metadata is not proof of source authenticity.

Successful imports append `evidence-imported`; parser rejection/cancellation
appends `evidence-import-rejected` with a bounded generic code and no source-text
error echo, when the case/session/revision still permit a write. Preflight denials
and results discarded after sign-out/case switch or a conflicting revision are
not appended under a new actor/case. Existing `analyses` records hold the reports;
there is no duplicate evidence or parallel database. Closed/archived cases permit
derived results as existing summaries do; suspended cases reject retention.

Existing protected schema-v1 records load without SQL table rewrites. New reports
are additive, retain the existing evidence/device references, survive protected
backup/restore and appear in case PDF reports. The UI lists findings, permits
reopening retained imports and exposes full JSON for review. No timestamp or
relationship inference is added by this increment.

Windows portable/installer packages include the worker and this guide. Native
synthetic tests cover formats, malformed inputs, duplicate JSON keys, XML external
entities, limits, isolated timeout/output/failure, cancellation, original integrity,
reproducibility, native permissions, sign-out, stale revisions and protected
backup/restore. Deployed smoke checks launch the actual worker from portable and
installed locations without SDK/PATH dependencies. Physical device acquisition,
independent clean Windows 10/11 and institutional acceptance remain external gates.
