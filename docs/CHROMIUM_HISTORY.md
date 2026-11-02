# Protected Chromium recorded visits

`chromium-history` is a fifth allowlisted native adapter inside the existing
Government case workflow. The operator imports authorized offline main/WAL/SHM
as protected evidence, selects main and explicitly binds optional sidecars.
The bridge rechecks case/session/grant/revision and every member before signed
acceptance through the existing prepared journal. There is no second evidence
store, SQL connection, filesystem profile access, updater or borrowed authority.

The pinned Browser source `d7754cd8f051d285406403465b6d7daa3bc4b060`,
`nbf/parsers.py:ChromiumParser.history`, is the basis for the selected
visits.url=urls.id join and raw count/transition/from-visit/time fields.
The exact scoped permission is recorded in FORENSIC_SOURCE_PERMISSIONS.md.
The original urls-only fallback, derived URL carving, profile enumeration,
CaseManager store and `_connect_ro` are not imported.

The production adapter wraps the existing `ChromiumHistoryReader` from concurrent
commit5129a15. There is one schema/table/join implementation. The internal reader
still exposes separately labelled urls-only metadata and unmatched-visit counts;
the case adapter requires both ordinary urls/visits tables and rejects orphan
visits, so metadata cannot silently become a visit finding. Its constrained
ASCII bare-identifier declaration grammar supports INTEGER/BOOLEAN/TEXT/LONGVARCHAR,
INTEGER PRIMARY KEY with optional AUTOINCREMENT, NOT NULL and literal DEFAULT.
Explicit known column allowlists include the reviewed sync/referrer columns;
unknown columns, quoted identifiers, expressions, comments, table constraints,
virtual/WITHOUT ROWID and generated tables reject. Required URLs fields include
hidden; schema/table encoding must be UTF-8. Float/reserved serials, invalid text,
unsupported schema and aliased pages reject. Schema shape is retained without
inferring browser version/profile/user identity. Storage classes are validated;
nullable from_visit stays NULL, and nullable title displays empty with original
bytes retained. Null URL/count/last-visit or null transition rejects the case
adapter rather than inventing source values. Valid unsupported databases can be
refused; this is not universal Chrome support.

Existing bounds remain: family/image8MiB, WAL4096frames, SHM128KiB; reader4096rows
per table,64KiB record,256fields,32levels,8MiB decoded payload, five-second
cooperative deadline. Declaration8192ASCII characters/32columns;
visit findings128, URL/title2048units each; report64KiB. A limit rejects the report,
with no silent truncation or partial findings. Large real profiles can be refused.

Each visit retains its exact signed decimal rowid, URL id, raw Chromium microsecond
timestamp, counts, transition and from-visit; URL/title are data, never navigated.
UTC display floors to milliseconds; raw microseconds remain exact. Nonpositive or
out-of-display-range timestamps remain unresolved rather than being invented.
Results are source-reported metadata, not proof of human viewing, authentic
acquisition, external trusted time or device/user identity. Missing WAL may mean
incomplete acquisition; SHM is opaque hash provenance and never replay authority.

Deterministic rowid order, family/image hashes and schema declaration hashes bind
extraction parameters. sqlite_schema, URL and visit records include image extents
and corresponding ORIGINAL protected main/WAL member id/hash/byte extents, including
complete leaf cell extents containing rowids/local payload/overflow pointer and
record payload/overflow data segments. These are
source byte references, not filesystem paths. Internal B-tree navigation and later overflow
chain pointer bytes are not separately enumerated; no database-wide integrity or deleted
record recovery claim. Referenced pages are disjoint across schema/urls/visits.

Retained report validation checks case/member provenance, family structure, schema,
counts, deterministic row order, canonical raw integers, timestamp normalization,
URL consistency and bounded extent correspondence before Findings/Timeline/Reports
projection. The existing signed receipt protects original acceptance; structural
projection checks alone are not independent re-extraction or authenticity proof.
Timeline uses recorded-visit meaning and preserves raw microseconds and source
extents. No Downloads, Sessions, cookie/password decryption, live acquisition,
disk writes or filesystem recovery is enabled.

Format reference: https://www.sqlite.org/fileformat2.html . Independent runtime
fixture: samples/chromium-history/generate_reference.py, read-only Python oracle
in tests/test_chromium_history_reference.py. Existing SQLite-table runtime tests
cover1200rows across512/1024/4096/65536page sizes including overflow/Arabic/BLOBs.
Native tests cover WAL title change, reordered columns, join fields, exact original
byte slices, forged retained timestamps/references, schema/types/orphans/limits,
case isolation, auxiliary substitution, cancellation/revocation/logout/audit failure,
signed retention/restart, and Arabic/English UI at default/125/150percent scales.
Acceptance is tied to exact source in evidence/chromium-history-2026-10-28.
