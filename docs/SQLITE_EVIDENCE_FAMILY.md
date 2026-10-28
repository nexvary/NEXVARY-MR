# Protected SQLite family page-image observations

`browser-sqlite-family` operates inside the existing Government forensic engine
workflow. Select an already imported main database and optionally bind WAL and
SHM evidence from the same authorized case. No source filename is resolved and
no SQLite connection, browser, process, filesystem write or second evidence
store is introduced. It extends the original Browser Lab family-provenance
concept, not its parallel CaseManager or heuristic WAL URL carving.

The result is a structural finding and a SHA-256 of a derived committed page
image, not a History/Downloads/session analysis, SQL/B-tree integrity test,
recovery guarantee or proof of authentic/common acquisition. WAL checksums do
not cryptographically bind a WAL to a particular main database; operator bindings
remain explicit declarations. No WAL supplied is reported with an acquisition
completeness warning. SHM is hashed and retained as opaque provenance, never used
for replay or trusted to decide transaction visibility. Rollback journals,
live-profile acquisition and encrypted databases are outside this increment.

Input contract: canonical evidence IDs and verified hashes; at most one WAL and
one SHM; no aliasing of evidence IDs; aggregate main+WAL+SHM <=8MiB, SHM<=128KiB,
page sizes512..65536 (power of two), <=4096 WAL frames, derived image<=8MiB,
report<=64KiB and existing cooperative5-second worker deadline. No hard OS memory
or thread sandbox is claimed. Empty protected evidence is unsupported by the
existing store. Malformed/truncated main headers or WAL, unknown roles, page
bounds, header/frame checksum failures and salt mismatches fail closed. The
strict policy also rejects stale/invalid trailing WAL bytes rather than claiming
a recovered prefix. Valid uncommitted complete frames are verified and excluded
from the image. No permissive partial-recovery claim is made.

The parser uses the published SQLite file format, including checksum byte order
and last-commit page selection:
https://www.sqlite.org/fileformat2.html#wal_format
A shrink removes obsolete pages; later growth must be covered by the current
committed transaction, preventing stale-page resurrection. Neither main-file
headers nor source bytes are normalized. A raw reference identifies the relevant
member, digest, byte offset and length; frame numbers are1-based with0 denoting
no committed WAL frame. Main/auxiliary originals are re-read before existing
signed case retention; actor, grant, case revision and cancellation guards remain
in force. Projection checks retained member bindings against that case's current
evidence inventory, parameters, summary and raw reference. Snapshot bytes are
an ephemeral native value for a future isolated schema parser; this increment
retains the report/hash, not a new persisted evidence copy.

Upstream reviewed again: Browser Lab
`d7754cd8f051d285406403465b6d7daa3bc4b060`, workflow-selected v4.2.0 XZ SHA-256
`62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226`,
`nbf/core.py:CaseManager.snapshot_sqlite_family`. Original code preserves sibling
names and hashes sequential copies; it does not prove atomic live acquisition.
MR uses explicitly bound protected evidence and independently implements bounded
WAL validation. Existing owner permission is recorded in
`FORENSIC_SOURCE_PERMISSIONS.md`; no new third-party runtime is linked.

Independent oracle: `samples/sqlite-family/generate_reference.py` uses Python's
SQLite runtime and `serialize()` with a real open WAL writer, two committed
transactions, and known synthetic urls/visits. The pinned fixture includes actual
main/WAL/SHM bytes, opposite-endian checksum and uncommitted-tail vectors, and
missing-growth/shrink-grow rejection vectors. Native results must match SQLite's
serialized bytes exactly, not merely an MR-generated expected hash. Independent
Python tests open the snapshot immutably and check SQL rows/quick_check without
modifying it. Product execution does not run that test SQL or enable History.

All broad integration stages remain in-progress. The next parser must consume
this bounded snapshot through the existing isolated import execution, retain
family/member/snapshot provenance, reject corrupt SQL structures and preserve
raw Chromium timestamps/row references. Original Audit Store divergence,
process descendants and Qt crash-root/soak gates remain open.
