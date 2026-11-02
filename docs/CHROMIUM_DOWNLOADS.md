# Protected Chromium download metadata

`chromium-downloads` is a sixth explicitly allowlisted native adapter in the
existing Government case workflow. It interprets selected offline Chromium
`downloads` and optional `downloads_url_chains` rowid tables from protected
main/WAL/opaque-SHM evidence. The existing family verifier derives only its
validated last-committed image. SQLite SQL, live profiles and recorded target
paths are never opened or executed by the adapter.

The existing bridge rechecks every protected member and the case/session/grant/
revision before the existing prepared-operation journal and signed retention.
Results feed the existing Findings, Timeline and Reports services. No second
store, authority, signing key, update mechanism or acquisition path is added.
Parsing uses the existing QtConcurrent native path with cooperative bounds;
this increment does not claim new OS process containment.

## Interpretation

- Download IDs, received/total byte counters, raw state/danger/interrupt codes,
  timestamps and optional recorded fields retain exact source values. Signed
  integers are decimal strings, including values beyond JSON double precision.
  A negative total such as -1 remains raw metadata; no completeness inference.
- Current/target paths are strings only. The displayed finding value is the
  recorded target path, never a fallback URL or an opened file. Mixed-direction
  technical values receive display-only bidi isolation; stored values do not.
- Each redirect member retains its index, URL, rowid and original main/WAL byte
  extents. Indices must be unique, nonnegative, contiguous and start at zero.
  Chains with missing download IDs, duplicate/gapped indices or malformed values
  reject the whole report, without swallowed exceptions or partial findings.
- An absent chain table, an empty chain for a known download and a contiguous
  recorded chain have distinct labels. Context tab/site/referrer fields remain
  separately labelled fields, without being substituted as a download URL.
- Start/end/last-access values retain raw microseconds since 1601 and exact Unix
  microsecond strings where supported. Zero stays unset, NULL stays NULL and
  negative values stay raw/unsupported. UTC display floors to milliseconds and
  only start time is projected as a source-recorded download-start event.
- A stored BLOB hash becomes `declaredHashHex`, retaining at most 64 raw bytes.
  It is not a content verification. Recorded state/counters/hash do not establish
  file presence, completed transfer, integrity, safety, a user or a causal event.

## Schema and byte provenance

History and Downloads share one constrained declaration grammar, storage-class
checks, time conversion, source-extent mapper and retained record validator in
`SqliteReaderSupport`. Both reuse the same bounded `SqliteTableReader` and
`SqliteEvidenceFamily`. History's existing column/type profile remains unchanged.
The shared retained validator also refuses boolean/string byte-offset impostors.

The Downloads profile supports the explicitly listed modern fields and known
legacy subsets with id/current_path/target_path/start_time/end_time/received_bytes/
total_bytes/state. INTEGER PRIMARY KEY id maps to the actual download rowid.
The chain profile requires the exact declared composite PRIMARY KEY(id,chain_index)
on an ordinary rowid table; its id field is NOT treated as the SQLite rowid.
Columns may be reordered. VARCHAR and VARCHAR(255), BLOB hash, and the existing
simple ASCII grammar are supported only by their explicit column policies.
Unsupported fields, expressions, quoted identifiers, other constraints, encodings,
WITHOUT ROWID/index pages, storage classes, NULL required fields, inconsistent
field counts, or aliased schema/data/overflow pages reject explicitly. ALTER TABLE
rows missing trailing fields are conservatively refused. Schema shape is not a
browser-version or whole-database-integrity attestation.

Every schema/download/chain record retains image extents and mapped original
member evidence ID, SHA-256, byte offset and length, including rowid cell prefixes
and overflow payloads. Retained validation checks family/inventory bindings,
known schema columns, roots, record extents, counts, canonical integers, timestamp
conversion, chain status/order/unique record IDs and displayed path provenance.
This is structural projection validation over an existing signed case result;
it does not independently reread source bytes or prove acquisition authenticity.

Bounds: existing 8 MiB family/image, 4096 WAL frames, 4096 rows/table, 64 KiB record,
256 record fields and 32 B-tree levels; additionally 8192 schema characters,
32 columns, 128 downloads, 512 chain rows, 64 links/download, 2048 characters/text
field, 64 raw declared-hash bytes and 64 KiB compact report. The request carries
its existing 5-second cooperative deadline/cancellation. Bounds fail, not truncate.

## Review and dependency scope

Pinned original Browser Lab `d7754cd8f051d285406403465b6d7daa3bc4b060`, reviewed
`nbf/parsers.py:ChromiumParser.downloads`, is the selected metadata/chain source.
The previously verified workflow payload SHA-256 is
`62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226`.
The scoped owner permission is recorded in `FORENSIC_SOURCE_PERMISSIONS.md`.
No upstream `_connect_ro`, CaseManager, live enumeration, SQL query executor,
URL fallback, broad exception suppression, decryption or updater is copied.

Current schema/path interpretation was cross-checked on 2026-10-28 against
https://chromium.googlesource.com/chromium/src/+/refs/heads/main/components/history/core/browser/download_database.cc
observed blob `54a19e9b04bfefe4623dff36541078c6b6bce304`.
SQLite format basis: https://www.sqlite.org/fileformat2.html.
This is an MR bounded byte implementation, without a new production runtime
library. Qt Sql/SQLite and Python stdlib SQL are used only as independent fixture
oracles. Existing Qt distribution/notices and external permission/release gates
remain unchanged.

The runtime oracle has modern 28-column downloads, a composite-key rowid chain,
physical insertion order opposite chain order, Arabic path updated in WAL, a
large row ID/counter, unknown total size, raw BLOB hash, two commits and independent
SQL expectations. Synthetic main/WAL/SHM are captured while the writer is open;
this controlled fixture does not establish coherent acquisition of arbitrary
operator-supplied evidence.

Sessions, live acquisition, filesystem recovery, original Audit Store divergence,
descendant containment, historical Qt root/soak/hardware and signing/institutional
acceptance remain separate open work.
