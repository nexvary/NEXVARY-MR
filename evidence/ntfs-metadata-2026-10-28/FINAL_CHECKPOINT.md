# Internal NTFS metadata prerequisite — verified source, 2026-10-28

## 1. Branch / HEAD / tested tree

Branch integration/forensic-engines-audit. Tested source `302ec290fbb8c65def0a68b1d69916105fa74ebb`;
tree `9786385ae3a5d660b48d0ad527ca87fddba541b7`; PR checkout `4fe3fd07d66ea5507af555a5c7e600b535aff688` has identical tree.
Government remains eb2322140956a55a50ee7494570f35573a1ce0c0.
Resolve subsequent checkpoint HEAD with Git: it changes only unshipped state/
research/evidence. Its full metadata tree has separate CI; scope proof identifies
unchanged executable/test/workflow/packaging/shipped-guide inputs. No inherited
exact-tree claim for later changes, main merge or Guardian/Government/upstream write.

## 2. Pinned original sources / review

Workbench8b35940b1c879e0ce5f5c13e0d6c89aeab023757;
Browserd7754cd8f051d285406403465b6d7daa3bc4b060;
DCd8e2baf61b43d044b33a1dd76a444d093c80023c.
Actual DC src/core/FilesystemRecovery.cpp selected fixup/attributes/ntStream/ntfs
reviewed; Git blob e2fa91596ca5d56059b7cce4e905da170b40f814. Scoped owner permission:
docs/FORENSIC_SOURCE_PERMISSIONS.md; review research/dc-filesystem-pinned-source-review.txt.
No general upstream license grant, copied third-party filesystem implementation,
new production dependency/store/updater/signature authority. Published NTFS3
structure references are in docs/NTFS_METADATA_PREREQUISITE.md.

## 3. Actual delivered capability and limits

**INTERNAL prerequisite only, NOT an enabled NTFS case adapter.**
NtfsMetadata.cpp is compiled into mr_ntfs_metadata_tests only; production allowlist,
desktop/QML, case store/receipt/Findings/Timeline/Reports do not use it. Existing
six integrated adapters preserved. No new application, unsigned case acceptance,
recovery/destination/source/device write or destructive API is introduced.

Bounded protected bytes/SHA, NTFS3.0/3.1 512-sector geometry, full USA/trailer checks,
MFT bootstrap/run mapping, bounded attributes/signed/sparse runs, volume version,
optional bitmap, recorded names/raw FILETIME/deletion/directory flags and original
pre-fixup record byte ranges. Resident hashes describe reconstructed data only.
Scope8MiB/128MFTrecords/128runs/64attributes/64KiB report, cooperative cancel/deadline,
no partial error report; unsupported attribute-list/compression/encryption/multiple
DATA extents refuse. Named streams/extension records excluded in declared scope.
No 4Kn/large-image/MFT-mirror/full-filesystem/acquisition/authenticity/overwrite/
complete-recovery claim. Raw times are not deletion events or trusted external time.
Bitmap free/allocated counters are observations, not recoverability. Detailed
case/retained/provenance/permission/restart gates: NEXT_CASE_INTEGRATION.md.

Independent test image: unmodified external ntfs-3g tools created a temporary
regular file, inserted abc and independently read metadata/content. No block device
or mount. Primary/backup non-interpreted bootloader code areas removed; ntfsinfo/
ntfscat checked again. No external library/executable/bootloader code is linked or
shipped. Package digests, exact image SHA and scope retained in fixture metadata.

## 4. Modified files

Full list in implementation-files.txt; runtime/test input hashes in source-sha256.json.
New NtfsMetadata.cpp/.hpp and ntfs_metadata_tests.cpp; CMake test option/target;
existing sanitizer workflow adds this target/filter without removing groups;
manual/independent fixture and Python checks; source/permission/scope/state/docs.
Downloads exact-source acceptance was recorded first in d7629c056dcfbd3c46535e76aa14202803872942.
No NTFS addition to production registry or worker/case signed kind allowlists.

## 5. Exact-source acceptance

[C++40738028510](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028510): **7/7 success**.
[Python40738028536](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028536) and
[public-history40738028545](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028545)
succeeded for the same source.

| Scope | Result |
|---|---|
|Windows Production/Laboratory|53/53 CTest each, MSVC /WX|
|Linux Production/Laboratory|53/53 CTest each|
|New NTFS target, all4profiles|28/28 each, explicit result files|
|Existing forensic case adapters, all4|30/30 each|
|Qt Quick, all4|59/59 each|
|Independent signed interoperability, all4|76/76 each|
|Production ASan/UBSan|22/22 under normal unchanged leak checking|
|DPI/GUI|default100/125/150groups and3repeat GUI per profile pass|
|Local root Python|60/60; exact hosted workflow succeeded|
|Local NTFS/standalone|28/28 plus CTest5/5|

New28includes init/cleanup and26substantive executions (21malformed data rows).
Fragmented MFT/source ranges, negative LCN delta, sparse/live/deleted/bitmap absence,
resident data crossing USA trailers, independent names/IDs24/25/26/64 and abc SHA,
geometry/USA/attrs/UTF16/version/run bounds,128record/output cap, hash/cancel/deadline
and256mutations. Uninterpreted mutations may succeed; not a universal tamper claim.
Per-profile XML/full logs/result text and subcase-skips.txt retained. No disabled
checks; zero skipped CTest groups does not erase platform/tool/profile subcase
exclusions. Text copies normalize CRLF to LF; provider ZIP digests identify original
bytes. Local LSan inspection failed due /proc restrictions; hosted normal22/22 is
separate acceptance, not a relabelled local leak pass.

## 6. Failures / causes

Current exact-source hosted C++/Python/history workflows succeeded. First local
strict indentation warnings corrected without suppression. First malformed UTF16
surrogate was accepted by Qt decoder without hasError; explicit pair validation
fixes it and retains the assertion. Independent fixture first assumed one file,
then dot/order entries were filtered/sorted to declared named-record>=16 scope using
external ntfsls inventory; exact four entries remain checked. First logs retained.
Local SDK unavailable/package privilege/cache/lock limits led to an isolated
extracted Qt/CMake dependency prefix; no unrelated checkout/user changes touched.
No product root-cause relationship to historical audit/Qt/descendant issues claimed.

## 7–8. Commits / PR

Accepted Downloads source9ce344941158c0ce02d2dd9e87000dfa000f0d88;
its evidence checkpoint d7629c056dcfbd3c46535e76aa14202803872942;
new NTFS prerequisite/tested source `302ec290fbb8c65def0a68b1d69916105fa74ebb`.
Later checkpoint resolve through Git. [PR17](https://github.com/nexvary/NEXVARY-MR/pull/17)
stays draft; [issue16](https://github.com/nexvary/NEXVARY-MR/issues/16) open. No merge.

## 9. Windows actual artifacts / deployment scope

- [Portable unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028510/artifacts/12840395753), providerSHA256bc5c1793356253af8aeaf29a9645fd1c4ec57592f0b3800b23be95a8a7c520f0.
- [Installer unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028510/artifacts/12839253780), providerSHA256675c368bf124bb5ed14e01b4542c21933c75d26ccb2d993cf38d1e557bce88ef.
- [Windows Production receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028510/artifacts/12839009266).
- [Windows Laboratory receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40738028510/artifacts/12838938851).

All1419 manifest entries/exact file set,1418 SPDX entries, actual AMD64 main/worker,
Qt6.8.3 unsigned production profile/source checked. Portable inner ZIP6ba15d28a263b26c90bf849c047d1bd3e6f0c22b985bc9058bdc73a32deeb058;
setup EXEda7758631d1e8bd1f7da2a7b2ae5e3ac87e28571ff691c33025357c737422188; mainbdb64bdf48a409b1f390c0f787e45e7394ec61e808d7f32660e936fc9c9b66c1; worker3109cb3af36918a4313b981ba7e980b9c996025727f4e7f45c0c02e2019a88eb.
Provider sizes/digests/expiry retained. Actual hosted portable/installed binary
hashes match downloads, launch0, install/uninstall0 and executable/per-user
registration removal verified; independent23-event audits each. Actual setup
PE/checksum verified, not just archive creation. These packages deploy existing
six-adapter desktop; NTFS prerequisite is NOT shipped as enabled functionality.
Physical Windows device offline: no new physical/phone/clean Windows11/hardware/
installer/product-signing acceptance claim.

## 10. Open issues / broad stages

All9 broad stages in-progress. NTFS case/authority/receipt/report integration,
exFAT/FAT32/recovery and Sessions/profile metadata pending. Historical Audit Store
signed disk/memory divergence root, process grandchildren containment, original Qt
SIGSEGV root/DPI soak/hardware, agency identity/trusted time/clean-host/independent
examiner/product signing remain open. No government certification, full recovered
file or competitor superiority claim.

## 11. Next

Implement strict retained NTFS schema and MFT/USA/source mapping, bind existing
case/evidence/original-working hashes, exact operation kind and signed receipt,
verify cancel/revoke/expiry/cross-case/restart/crash/report integrity and actual
AR/EN scaled controls before enablement. No second store or writer import. Limits
and acceptance criteria in NEXT_CASE_INTEGRATION.md; broader read-only filesystems
and recovery remain separate scoped increments.
