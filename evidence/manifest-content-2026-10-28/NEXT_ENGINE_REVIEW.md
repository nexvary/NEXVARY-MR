# Pinned-source follow-on review — 2026-10-28

This is source review, not additional migrated functionality. MR source under
acceptance is c83f2d7d741e0368cfb9e31f690eeb4df9273d3c.

Browser Lab d7754cd8f051d285406403465b6d7daa3bc4b060 workflow-selected v4.2.0
XZ was reconstructed again: 84,692 bytes, SHA-256
62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226.
Inspected actual nbf/parsers.py, core.py, session_recon.py, sqlite_recovery.py
and browsers.py rather than the historical ZIP.

- ChromiumParser.history joins visits/urls, with a distinct urls-only fallback.
  Preserve that fallback as a lower-confidence metadata observation, not an
  equivalent visit record. Retain visit and URL row IDs/raw timestamp fields.
- downloads adapts to available columns and optional downloads_url_chains.
  Current queries have no global row bound/SQLite progress handler; MR must
  reuse its bounded isolated import execution instead of copying this verbatim.
- _connect_ro uses SQLite mode=ro on a snapshot. CaseManager.snapshot_sqlite_family
  preserves DB/WAL/SHM/journal filenames and checks each member hash. It copies
  members sequentially; it does not prove atomic acquisition of a live family.
  MR should accept explicitly linked protected offline evidence members and
  report family/acquisition limitations, with interrupted/corrupt-WAL tests.
- Session reconstruction parses SNSS command envelopes and separately carves
  raw URL traces. Preserve command/offset references and medium/low confidence;
  raw strings do not establish complete sessions or visits. Firefox LZ4
  decompression needs an independent output bound before enablement.
- recover_wal_file carves URL strings per frame after checking basic header and
  size. It does not validate frame salts/checksums/commit membership before
  treating these as candidate observations. It cannot substantiate committed
  History records; do not promote it as WAL transaction recovery.
- Profile enumeration must be replaced with explicit evidence bindings rather
  than probing the user's installed/live browsers. No cookie/password decryptor
  or upstream case/signing/update authority is authorized by this migration.

Disk Care d8e2baf61b43d044b33a1dd76a444d093c80023c actual
src/core/FilesystemRecovery.cpp was re-inspected. NTFS, exFAT and FAT32 parsing
is coupled to Image(QFile), ReadCoverage and Output::save destination writes.
No wholesale import: first separate bounded memory/range reads and metadata/
extent results. Keep Output::save and all destructive/device/controller paths
outside the Government adapter allowlist. Preserve candidate/partial/complete
coverage labels and the explicit warning that coverage is not original-content
authenticity. Known-source limitations include compressed/encrypted NTFS and
external MFT attribute-list extensions. Neither filesystem parsing nor recovery
has been enabled in MR by this review.

All broad integration stages remain in progress. Next implementation is protected
SQLite family ingestion with hash-bound members, deterministic family provenance,
resource/authority/cancellation/crash tests, then a schema-specific History slice.
