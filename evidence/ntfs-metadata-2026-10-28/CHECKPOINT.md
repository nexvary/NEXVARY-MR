# NTFS metadata prerequisite — 2026-10-28

Branch integration/forensic-engines-audit; parent Downloads acceptance d7629c056dcfbd3c46535e76aa14202803872942.
Resolve source with git rev-parse HEAD; PR17 remains draft. This is NEW internal
code, not accepted by prior Downloads/History CI/packages and NOT enabled in desktop.

Pins: Workbench8b35940b1c879e0ce5f5c13e0d6c89aeab023757,
Browserd7754cd8f051d285406403465b6d7daa3bc4b060,
DCd8e2baf61b43d044b33a1dd76a444d093c80023c.
Actual src/core/FilesystemRecovery.cpp helpers fixup/attributes/ntStream/ntfs inspected;
read review research/dc-filesystem-pinned-source-review.txt and scoped permission.
Image(QFile), Output::save and destructive/controller/writer paths excluded.

Changed: NtfsMetadata.cpp/.hpp; ntfs_metadata_tests.cpp; CMake option/test target;
hosted sanitizer target/filter includes it without disabling existing groups;
synthetic/independent fixtures and Python oracle; source/permission/scope/state/docs.
Existing6adapters/case/worker/receipt/UI/timeline remain unchanged. No new runtime,
evidence database, signing/updater authority or registered NTFS case operation.

Selected byte-only kernel verifies input SHA, NTFS3/512-sector geometry, bootstrap
MFT mapping/USA/version/bitmap, bounded attributes/runlists and original record
ranges; reports named user MFT metadata, raw times/deletion flags, reconstructed
resident hashes and recorded sparse/allocation observations. No saved file or
complete/partial recovery. Full bounds/unsupported features and prerequisites for
case enablement: docs/NTFS_METADATA_PREREQUISITE.md. No NTFS4Kn/full-filesystem or
MFT-mirror/content authenticity proof. Errors expose no partial report.

Local exact-input checks:
- GCC13.3/Qt6.4.2 C++20 -Wall/-Wextra/-Wpedantic/-Werror built.
- NTFS28/28 Qt entries (26 substantive including21 malformed data rows); no skips.
  Synthetic fragmented MFT, negative relative LCN/sparse runs, resident data crossing
  USA trailer, actual raw record slices, deleted/bitmap-unavailable/allocated/sparse
  observations, geometry/USA/attributes/text/version/run bounds,128record and output
  caps, hash/cancel/deadline and256deterministic mutations. Mutations outside
  interpreted bytes may succeed; no full tamper/authenticity claim.
- Independent unmodified ntfs-3g2022.10.3 tools created/read a temporary regular-file
  NTFS3.1 volume. No block device/mount. ntfsinfo named user records24/25/26/64 and
  ntfscat `abc` verified; actual parser matches names/IDs/content SHA. Bootloader
  areas zeroed in committed fixture and independent ntfsinfo/ntfscat repeated.
  No GPL executable/library/bootloader code linked/copied/shipped. Package hashes
  in fixture metadata; fixture is not a general real-image compatibility claim.
- Standalone CTest5/5; root Python60/60 including independent decompression/hash/
  bootloader-exclusion/synthetic regeneration byte equality. Codec bitstreams may
  differ by zlib version while underlying images must be identical.
- ASan/UBSan28/28 exit0 with halt_on_error=1/detect_leaks=0. Original detect_leaks=1
  finished tests then exited1 because /proc task inspection is prohibited. Original
  diagnostics retained. Local leak acceptance INCOMPLETE, hosted normal gate intact.

First failures preserved: strict indentation; unpaired UTF16 accepted by decoder
without hasError, fixed with explicit surrogate pairing and original assertion;
independent inventory fixture initially assumed one file instead of system records,
corrected from independent ntfsls output with scope>=16/sorted index. No test disabled.
Environment lacked SDK; dependencies extracted into isolated scratch prefix after
package manager privilege/cache/lock limitations, without touching unrelated checkout.
No new physical Windows test: connected device offline.

Exact NEW-source hosted CI/MSVC/Windows/Linux/full Qt/normal sanitizer/package
verification PENDING until a source-specific report records it. No new Windows
artifact acceptance claimed yet. Prior Downloads accepted-source artifacts are
linked only in evidence/chromium-downloads-2026-10-28/FINAL_CHECKPOINT.md.

All9 stages remain in-progress. Next: case-bound NTFS adapter/retained validator,
existing signed receipt/Findings/Timeline/Reports and adversarial authority/restart/
crash tests before enablement. exFAT/FAT32/recovery, Audit Store historical root,
process descendants and original Qt SIGSEGV root/soak/hardware remain open. No
Government certification or competitor-superiority claim. No main merge.
