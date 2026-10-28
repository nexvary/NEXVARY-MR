# Scope of the project owner's integration permission

The project owner's 2026-10-28 instruction explicitly authorizes integrating
actual NEXVARY engines from the three named repositories into NEXVARY-MR,
while retaining the original repositories. This is the permission basis for
the selected original NEXVARY functions below. It is recorded as
`LicenseRef-NEXVARY-Owner-Authorized-Integration`, a reference to that limited
instruction, **not an invented MIT/GPL license or an assertion of public-domain
status**. No general license grant, third-party ownership warranty, institutional
approval, or permission to reuse unrelated repositories is inferred.

| Original repository / exact source | Selected original function | Derived MR files |
|---|---|---|
| NEXVARY-Digital-Forensics-Workbench `8b35940b1c879e0ce5f5c13e0d6c89aeab023757`, verified v12.22 payload | `nexvary_lab/pro_tools_v1221.py:export_validate` | `ForensicAdapters.cpp:inspectManifest` |
| NEXVARY-Browser-Forensics-Lab `d7754cd8f051d285406403465b6d7daa3bc4b060`, verified v4.2.0 payload | `nbf/parsers.py:ChromiumParser.bookmarks`, `nbf/core.py:chrome_time` | `ForensicAdapters.cpp:parseBookmarks/chromeTime` |
| NEXVARY-Browser-Forensics-Lab `d7754cd8f051d285406403465b6d7daa3bc4b060`, verified v4.2.0 payload | `nbf/core.py:CaseManager.snapshot_sqlite_family` family provenance concept; MR independently implements bounded byte-only WAL validation | `SqliteEvidenceFamily.cpp` |
| NEXVARY-Browser-Forensics-Lab `d7754cd8f051d285406403465b6d7daa3bc4b060`, verified v4.2.0 payload | `nbf/parsers.py:ChromiumParser.history`, `nbf/core.py:chrome_time`; selected urls/visits join only, MR bounded schema/table reader and original member extent mapping | `ChromiumHistory.cpp`, `SqliteTableReader.cpp` |
| NEXVARY-Browser-Forensics-Lab `d7754cd8f051d285406403465b6d7daa3bc4b060`, verified v4.2.0 payload | `nbf/parsers.py:ChromiumParser.downloads`; selected recorded metadata and ordered redirect-chain semantics, MR bounded byte/provenance implementation | `ChromiumDownloads.cpp`, `ChromiumDownloadsReader.cpp`, shared `SqliteReaderSupport.cpp` |
| NEXVARY-DC `d8e2baf61b43d044b33a1dd76a444d093c80023c` | `src/core/FilesystemRecovery.cpp:fixup/attributes/ntStream/ntfs`, selected metadata/runlist concepts only, internal test prerequisite | `NtfsMetadata.cpp` (not a registered production adapter) |
| NEXVARY-DC `d8e2baf61b43d044b33a1dd76a444d093c80023c` | `src/core/Partitions.cpp:imageVolumes` | `DcImagePartitions.cpp` |

The inspected payloads contain no general LICENSE for original application
code. Browser `INTERNAL_USE.txt` additionally restricts operation to authorized
NEXVARY laboratory/evidence use; it is not an open-source redistribution license.
DC `NOTICE.md` identifies the application's original code and separately lists
third-party distribution notices. The owner's specific integration instruction
authorizes this migration; it does not silently relicense those third parties.
External distribution/license review remains an acceptance gate.

Only these small, traceable functions/concepts are adapted. The SQLite-family increment does not copy the upstream CaseManager store, live-file copying, updater or WAL URL carving. Its frame/salt/checksum/last-commit page-image validation is an MR extension based on the published SQLite file format; no new SQLite/Python/third-party runtime is linked. Python stdlib calls are
replaced with existing Qt Core APIs; CPython, PySide6, lz4, pytsk3, smartctl,
CrystalDiskInfo, OpenSuperClone, GRUB, PowerShell helpers and updater/signing
code are not copied, linked, downloaded at runtime, or bundled by this change.
MR's existing dynamically linked Qt distribution and notices continue to apply.
Any additional module requires its own source/permission/dependency review;
this permission record must not be treated as approval of an entire dependency
tree or all files bearing the NEXVARY name.

History review: original history() snapshots History and joins visits.url to urls.id. MR adapts the selected join/metadata semantics only; it does not copy _connect_ro, CaseManager, derived_web_artifacts, urls-only fallback, arbitrary SQL, profile filesystem enumeration or the upstream store. No new dependency: Qt Core handles byte traversal/time/hash; Qt Sql is used only by independent generated-fixture tests. No CPython/SQLite runtime is newly linked by this adapter. Schema/table/file-format interpretation is an MR extension and remains deliberately bounded.

Downloads review: the actual pinned downloads() selects available recorded fields and optional chains. MR preserves fields and each chain member, explicitly labels absence and refuses malformed chains; it does not copy broad exception suppression, guessed URL/path fallbacks, filesystem access or SQL execution. Recorded paths/hashes never become file-presence/content/completion claims. The existing case authority, retention and signed-operation services remain the owners. No new production dependency or general upstream license is asserted.

NTFS prerequisite review: only bounded byte reads, USA/attribute/runlist interpretation, recorded names/times/bitmap observations are adapted. Image(QFile), Output::save, recovery destination writes, carving, format/repair/erase/rebuild/controller/update paths are excluded. External unmodified ntfs-3g tools generated an independent temporary regular-file test image only; no executable/library/bootloader code is linked or shipped. Non-interpreted primary/backup bootloader regions in that fixture were zeroed, then ntfsinfo/ntfscat verified the metadata and abc content again. Microsoft published NTFS3 structures inform stricter USA validation; no third-party filesystem implementation is copied.
