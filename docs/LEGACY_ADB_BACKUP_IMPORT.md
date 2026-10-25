# Legacy Android ADB backup inventory

Select **ADB v5 backup — inventory without extraction** in Government → Case
evidence analysis → Authorized evidence import. Register a lawfully available
backup as protected evidence first, supply a purpose and confirm the existing
temporary plaintext-copy notice. This adapter never invokes ADB, contacts a
device, restores a backup, obtains keys or changes device security.

## Implemented format and output

Adapter `adb-backup-v5`, parser version 1, accepts only the exact four-line header
`ANDROID BACKUP`, `5`, `0`, `none` (LF-terminated), followed by a bounded USTAR
stream. This is **member inventory**, not semantic application recovery. It records
regular files/directories below `apps/<package>/`, including opaque `_manifest`
and `_meta` members. A manifest's existence/hash does not authenticate its claims
or application signatures. Unknown domain names are literal labels only.

For each retained member: full archive path, package/domain token, type, size,
SHA-256 of exact file content, absolute content offset in the source backup and
raw modification-time epoch seconds. Directory hashes are empty. Raw mtime is
untrusted metadata, not event time, and is not projected into the timestamp
timeline. The report also retains the existing whole-source hash, case/device/
evidence references, purpose and exact report hash in protected analysis history.
Repeated imports on the same source/build/provenance are deterministic; history
survives the existing protected backup/restore path. No new database/schema.

All members are validated before success, including members omitted from the
retained sample. Limits: **4 MiB source, 4,096 members, 128 retained findings,
255 UTF-8 bytes/path, 60 KiB parser report**, existing five-second child deadline.
If any member violates bounds/format, the entire import is rejected; no partial
success report. Findings retain archive order, not lexicographic or time order.
Large long-path reports can hit the report-byte cap before the member-count cap.

Checks include USTAR marker, numeric octal fields, header checksum, payload bounds,
zero padding, two zero end blocks and no nonzero trailing bytes. Invalid UTF-8,
ambiguous NUL suffixes, controls/bidi formatting, absolute/traversal/empty/dot/
backslash/colon paths, duplicate paths and file-as-parent conflicts are rejected.
Nothing is extracted or executed. Source bytes go only to the existing private
read-only working-copy child; pre/post hashes, cancellation, session/grant/actor/
case/revision checks and rejection auditing remain enforced.

## Unsupported and pending

Compressed or encrypted backups, other versions, PAX/GNU extended headers,
links/devices/FIFOs/sparse files, shared-storage layouts and non-USTAR variants
are **Unsupported** by this adapter. A valid Android backup using these features
will be rejected. No decryption, decompression, extraction, application record
decoding, filesystem restoration, iOS/vendor-container support or completeness
claim. A filename extension is never proof of format or lawful authority.

Temporary cleanup is not secure erasure; process separation is not an OS/network
sandbox. Existing account/DPAPI, local-clock and independent legal/personnel
verification boundaries are unchanged. See [authorized imports](AUTHORIZED_IMPORT.md).

## Reproducible verification

`samples/authorized-import/adb-v5-reference.json` is a populated synthetic fixture
generated independently using Python stdlib `tarfile.USTAR_FORMAT`: opaque app
manifest, directory, Arabic UTF-8 note and binary payload. Expected content hashes,
offsets, sizes and paths come from the independent Python reader, not C++.

```sh
python scripts/check_adb_backup_interop.py build/mr_evidence_import_worker
ctest --test-dir build -R 'mr_(import_tests|adb_backup_interop|ui_tests|ui_scale_)' --output-on-failure
```

The interoperability checker covers reproducibility, exact provenance, no member
extraction, positive Unicode/prefix/empty-file/limit cases and explicit rejection
of malformed, dangerous and unsupported inputs, including a bad member after the
128-result cap. Native bridge tests apply consent, source preservation, audit,
protected restore, cancellation, stale revision, logout and revoked grant to both
JSON and ADB sources. Qt tests exercise the actual selector/import and AR/EN views
at 840/1366/1536 widths and 100/125/150% DPI. Deployed Windows checks run the same
populated fixture through portable and installed workers with Qt SDK excluded.
Exact-source results must be recorded in DEVELOPMENT_STATE.md before being called
Tested. Independent lab/physical-phone/institutional validation remains Pending.

## Primary format references

- [AOSP PerformAdbBackupTask, pinned revision 3682f501fb72](https://android.googlesource.com/platform/frameworks/base/+/3682f501fb72/services/backup/java/com/android/server/backup/fullbackup/PerformAdbBackupTask.java): envelope version/compression/encryption fields. No implementation copied.
- [AOSP BackupHelpers, pinned revision 31dbfc1b3e18f24d9a34c4e600c2048e8d244c14](https://android.googlesource.com/platform/frameworks/base/+/31dbfc1b3e18f24d9a34c4e600c2048e8d244c14/libs/androidfw/BackupHelpers.cpp): Android USTAR layout, padding, prefix and extended-header boundaries.
- [AOSP TarBackupReader, pinned revision 7951f5d173cbf3904399ebd042c771a88e894c61](https://android.googlesource.com/platform/frameworks/base/+/7951f5d173cbf3904399ebd042c771a88e894c61/services/backup/java/com/android/server/backup/utils/TarBackupReader.java): package/domain metadata and PAX handling, which is deliberately unsupported here.

Format documentation and internal synthetic tests establish neither vendor
coverage parity nor comparative superiority, field verification or accreditation.
