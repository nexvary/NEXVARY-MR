# Internal protected NTFS metadata prerequisite

`inspectNtfsMetadata` is compiled into `mr_ntfs_metadata_tests` only. It is NOT
registered in forensicAdapters, mr_desktop, QML, signed case retention, Timeline or
Reports. It does not authorize caller-supplied bytes or issue a signed receipt.
Case integration/retained validation is a subsequent acceptance gate; do not treat
its internal observations as accepted case findings or recovered files.

The internal schema is now `nexvary-mr-ntfs-metadata/2`. It binds exact case and
evidence UUID strings, protected source/working-copy SHA256 and explicit analysis
parameters. These are provenance labels, not access authorization, original
acquisition proof or a signed audit receipt. The original acquisition hash must
come from the existing case evidence store at eventual integration.

The report retains the nonresident MFT logical-to-image mapping, original byte
ranges and raw/reconstructed record digests for named records and the bootstrap,
$Volume and available $Bitmap records. Each record includes the original USA
array and exact trailer replacement offsets/bytes. The bitmap content digest is
separate from its source record digest. Reconstructed resident/file-record hashes
must never be described as a recovered external file's original content hash.

`verifyNtfsMetadataReport` is an internal replay gate. Its caller must supply
fresh protected bytes and trusted current case/evidence bindings. It checks the
bounded report digest, re-runs the parser with the same deadline/cancellation
token and compares the entire canonical report byte-for-byte. Rehashing a forged
report, an in-bounds but false MFT reference, altered USA data or a changed case
cannot pass replay. Noncanonical/duplicate-key/unknown-field JSON is refused by
the exact comparison without parsing attacker-controlled report JSON. No v1
compatibility fallback. The expected report digest is only an equality token;
any caller can hash arbitrary bytes, so a digest alone establishes no authority.

This gate is NOT wired into the case store yet. It does not test grants, expired
sessions, signed preparation, crash reconciliation or restart deduplication.
Before enablement, call it on re-read protected bytes before the existing signed
transaction; retain the current authority/member/revision checks and all case
integration acceptance requirements. A stored report alone cannot replace replay
or independently prove source mapping, authenticity, free-space integrity or
recoverability. Bounds remain 8MiB input/64KiB output; added provenance can cause
earlier whole-report rejection, never silent omission or partial success.

Pinned Disk Care source d8e2baf61b43d044b33a1dd76a444d093c80023c,
src/core/FilesystemRecovery.cpp:fixup/attributes/ntStream/ntfs was inspected.
Image(QFile), Output::save and all destination/format/repair/erase/rebuild/controller
paths are excluded. Existing Qt Core byte/hash/text APIs only; no new production
filesystem library, executable, evidence database or authority. Scoped permission:
FORENSIC_SOURCE_PERMISSIONS.md. This code is a bounded derivative of selected
original functions, not wholesale recovery-engine migration.

The byte request's SHA256 must match, size512bytes..8MiB. Selected scope is an
NTFS3.0/3.1 volume-image prefix at offset0,512-byte sectors, power-of-two clusters
up to64KiB, record512bytes..64KiB no larger than cluster, at most128 MFT records.
Declared volume fits image; trailing bytes are hashed but not parsed. Boot MFT
record0 must agree with its own bounded nonresident stream mapping. Full USA array
must finish before the first512-byte trailer; every trailer must match before
in-memory replacement. $Volume version3.0/3.1 and optional $Bitmap are checked.
This is not 4Kn support, a boot/MFT-mirror/freelist/full-filesystem integrity check,
a partition-offset scanner, or a complete acquisition/authenticity claim.

Attributes are bounded/aligned/terminated with unique IDs,64per record. Attribute
lists, split DATA extents, compression/encryption and unsupported stream flags
refuse. Named DATA streams and extension records are counted/excluded. Runs:
128per stream, bounded signed LCN deltas, sparse flag/header checks and nonoverlap;
size/initialized/physical ranges cannot exceed the declared bounded volume.
No metadata reads through sparse/uninitialized regions. FILE_NAME metadata permits
8names,255UTF16units each; malformed UTF16/NUL/namespace refuses. Empty free slots
are counted. Named user records>=16 yield raw names, FILE_NAME timestamp integers,
parent reference, sequence/live/deleted/directory flags, pre-fixup record hash/
original image ranges and recorded resident/runlist metadata. Raw FILETIME is not
converted into Timeline events. No inferred directory hierarchy or actor identity.

Resident SHA256 refers to USA-reconstructed data, NOT a saved/original external
file. Raw record ranges remain pre-fixup source bytes. Bitmap availability/free/
allocated bytes and sparse logical sizes are source-recorded observations, not
recoverability, authenticity, overwrite detection or a complete/partial recovered
file. Uninitialized bytes stay distinct. Input image is never changed, and paths
are never resolved. No filesystem/device/source/destination writes exist in the
parser. Cooperative cancel/deadline checks,64KiB output and no partial error report.

Independent tests include a manually constructed fragmented MFT and signed/sparse
runs, a regular-file NTFS image independently generated/read by ntfs-3g tools,
exact `abc` content hash, and original record byte references. External tools are
unmodified fixture-generation tools only, not linked or shipped. Their temporary
regular file is created explicitly; no block device/mount is used. Non-interpreted
bootloader areas in the checked-in fixture were zeroed and the metadata/content
independently reread afterwards. Package digests and scope are in fixture metadata.
Synthetic fixed fields reproduce exactly; compression streams may differ by zlib
version while decompressed image bytes must match. Independent image timestamps/
serials are tool-generated, not an external trusted-time assertion.

Published NTFS3 references (layout facts only, no third-party implementation copied):
https://learn.microsoft.com/en-us/windows/win32/devnotes/multi-sector-header
https://learn.microsoft.com/en-us/windows/win32/devnotes/file-record-segment-header

All nine broad stages remain in progress. Before enabling a case adapter, add
strict retained schema/reference validation, case/evidence/authority bindings,
signed receipt integration, cancellation/revocation/restart/crash tests and real
Arabic/English scaled controls. exFAT/FAT32, reconstruction/carving/output recovery,
large images, attribute-list extensions and the original audit/descendant/Qt-root
acceptance blockers remain separate work.
