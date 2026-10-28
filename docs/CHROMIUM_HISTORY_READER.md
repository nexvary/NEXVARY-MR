# Protected Chromium History extraction prerequisite

The internal `readProtectedChromiumHistory` function accepts only an existing
protected byte request. It verifies the SQLite main/WAL/opaque-SHM family, reads
sqlite_schema from its committed page image, resolves ordinary `urls` and
`visits` tables, validates a deliberately small CREATE grammar and maps columns
by their declared names. It never executes SQL or accepts a filesystem path.

This increment is compiled into the SQLite reader test target only. It is not
registered in the production adapter allowlist, exposed in QML, retained as a case
analysis, or projected into a timeline. That integration needs a separate retained
report validator, current authority/hash revalidation, signed operation and worker
protocol tests. Do not present internal results as accepted case findings.

## Supported observations

- Joined visit/URL records retain exact row IDs, raw timestamps, transition,
  nullable referrer ID, URL/title, visit/typed counts and last-visit metadata.
- A genuinely absent visits table enables a distinctly labelled URLs-only
  metadata mode. An empty visits table produces no visits; an unsupported or
  malformed visits table fails, without a silent fallback.
- Unmatched visit URL IDs are counted and excluded from the join, never invented.
- Integers remain decimal strings, including values outside JSON double accuracy.
  Storage classes are retained by the table reader: BLOB/text impostors cannot
  silently become integers or text. Nullable source URL/title remain NULL.
- Chromium microseconds since 1601 are preserved. Positive signed values additionally
  yield exact decimal Unix microseconds by subtracting 11644473600000000. Zero
  stays explicitly unset, NULL stays NULL, and negative values remain raw with
  an unsupported-negative label. No millisecond rounding or inferred user identity.
- Each observation includes schema-record and data-cell/payload extents mapped
  to original protected main/WAL bytes with evidence ID, SHA-256, offset and length.
  WAL mappings use the last committed replacement for each page, including
  overflow pages. Image offsets remain explicitly distinguished from source offsets.

## Schema and resource limits

Only ASCII CREATE TABLE statements for rowid tables, simple unquoted column
names, INTEGER/BOOLEAN/TEXT/LONGVARCHAR, an exact INTEGER PRIMARY KEY ID (optional
AUTOINCREMENT), simple numeric/boolean defaults and NOT NULL are recognized.
Known URL fields and known visit fields are mapped by name, including current
sync/referrer/app fields and legacy is_indexed. Unknown columns, quoted identifiers,
comments, expressions, virtual tables, WITHOUT ROWID, unsupported encodings/types,
invalid page references and field-count disagreement fail with no partial report.
This is a field-layout acceptance rule, not Chrome-version attestation. Rows missing
trailing fields after ALTER TABLE are conservatively rejected, not filled by guessed
defaults. Index/freelist/global integrity and historical schema coverage are not claimed.

Inherited family/table limits: 8 MiB input/image, 4096 WAL frames, 4096 rows per
table, 64 KiB record, 256 fields, 32 B-tree levels, bounded decoded payload and
cooperative 5-second deadline/cancellation. Additional limits: 8192 schema characters,
32 columns, 128 observations and 64 KiB compact JSON report. Exceeding limits fails
rather than truncating observations. Schema/data parsing is in memory; paths and
live browser profiles are inaccessible. Repeated in-memory family verification is
intentional until a separately reviewed verified-snapshot interface exists.

## Source review and attribution

Original pinned Browser Lab source remains `d7754cd8f051d285406403465b6d7daa3bc4b060`;
reviewed workflow-selected v4.2.0 XZ SHA-256:
`62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226`.
`ChromiumParser.history` in its actual `nbf/parsers.py` was reread: it joins
visits/urls and separately falls back when visits is absent. MR implements its own
bounded byte reader and preserves that distinction; no upstream live-profile
enumeration, SQLite connection, derivation, decryption or authority is copied.
See the earlier `evidence/manifest-content-2026-10-28/NEXT_ENGINE_REVIEW.md`.

Schema cross-check on 2026-10-28 against primary Chromium sources:
- https://chromium.googlesource.com/chromium/src/+/refs/heads/main/components/history/core/browser/url_database.cc
  observed blob `bef58b4e43b8b39e3c2559c010f62455efad4860`, CreateURLTable.
- https://chromium.googlesource.com/chromium/src/+/refs/heads/main/components/history/core/browser/visit_database.cc
  observed blob `a85535de9a90e5dd97f4d8411487201ebfc11134`, InitVisitTable.
- https://www.sqlite.org/fileformat2.html — inherited table record/page format.

Runtime SQL exists only in independent synthetic tests. It creates and queries
fixtures while the writer is open, captures actual WAL/SHM, and compares the
extractor's join with SQLite's SELECT results. No real profile data is used.

## Remaining work

Case/worker/receipt integration and adversarial retained-report validation, then
Downloads and the separate read-only filesystem metadata/extents path remain open.
No recovery, live acquisition, original-content authenticity, institutional
accreditation, release signing or repair of historical Audit Store/Qt/process-tree
issues is established by this prerequisite.
