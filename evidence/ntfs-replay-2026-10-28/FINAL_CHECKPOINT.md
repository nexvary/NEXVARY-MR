# Internal NTFS v2 replay — verified source, 2026-10-28

## 1. Branch and exact tested source

integration/forensic-engines-audit; executable/test source `1a4471d89f98d4bf91c03a202b32786cb8e264fa`, tree `13590730417670cb8b95365dda60b1eb104dda92`.
PR checkout `5f709e3830608da9388755c4640a5b1fe23dc0b6` has identical tree. Government remains
eb2322140956a55a50ee7494570f35573a1ce0c0. Resolve subsequent metadata-only HEAD
with Git. Its full tree has separate CI; no exact-tree result inherited from this
source. checkpoint-scope.json identifies unchanged native/app/test/build/workflow/
packaging/shipped-guide inputs. No main merge/Guardian/upstream/Government write.

## 2. Pinned sources

- Workbench:8b35940b1c879e0ce5f5c13e0d6c89aeab023757.
- Browser:d7754cd8f051d285406403465b6d7daa3bc4b060.
- DC:d8e2baf61b43d044b33a1dd76a444d093c80023c.

Selected DC source review: research/dc-filesystem-pinned-source-review.txt; scoped
owner permission: docs/FORENSIC_SOURCE_PERMISSIONS.md. This increment adds MR
provenance/replay checks to the existing selected internal reader; no additional
upstream module/new dependency/store/update/signature authority or third-party
implementation imported. Not a general upstream distribution/license grant.

## 3. Actually implemented capability

**Internal/test-target-only, NOT an enabled NTFS case adapter.** Report v2 binds
case/evidence UUID labels, protected source/working-copy hash and explicit bounded
parameters; retains MFT logical/image mapping, system/user original ranges and
raw/reconstructed record hashes, full original USA arrays and exact trailer
replacement bytes, and bitmap content hash. verifyNtfsMetadataReport checks a
bounded report digest and complete canonical byte equality after fresh protected-
source replay using the same cancel/deadline. Rehashed false in-range references,
USA/times/flags/confidence/parameters/extra fields, cross-bindings and duplicate-key/
noncanonical JSON refuse with no partial report and no untrusted JSON parsing.

UUID labels and a report hash are NOT authorization, original acquisition proof
or a signed audit receipt. No actual grant/session/case-persistence boundaries are
implemented by this API. Caller must obtain fresh bytes/current authority from
existing services at later case integration. No v1 acceptance fallback.
512-sector/offset0 NTFS3,8MiB input/128MFTrecords/128runs/64attributes/64KiB report
limits remain. Extra provenance can reject earlier; no silent truncation. No
full-filesystem/mirror/attribute-list/compression/encryption/named-stream/large-image
support, source/destination/device writes, recovery or overwritten-content truth.
Record/resident/bitmap hashes and recorded runs are distinct observations, not a
recovered external file. Raw FILETIME remains exact with no deletion-event, actor,
preferred alias or hierarchy inference. Existing six case adapters unchanged.

## 4. Modified files

implementation-files.txt lists17 source-checkpoint paths. Principal code:
cpp/src/NtfsMetadata.cpp/.hpp and cpp/tests/ntfs_metadata_tests.cpp; scope docs,
state/research and evidence. source-sha256.json records exact native/test/build/
fixture inputs and is checked again at acceptance. Production registry/desktop/
case/QML/kind allowlists/workflows were not modified by this increment.

## 5. Exact-source acceptance

[C++40747698401](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698401) **7/7 success**;
[Python40747698812](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698812)
and [public-history40747698461](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698461)
success for this exact source.

| Scope | Result |
|---|---|
|Windows Production/Laboratory, MSVC /WX|53/53 CTest each|
|Linux Production/Laboratory|53/53 CTest each|
|Internal NTFS target, all4profiles|57/57 each, explicit result files|
|Existing forensic case adapters, all4|30/30 each|
|Qt Quick, all4|59/59 each|
|Independent signed interoperability, all4|76/76 each|
|Production ASan/UBSan, normal leak gate|22/22|
|DPI/repetition|default100/125/150groups and3repeat GUI each pass|
|Local strict NTFS/standalone/root Python|57/57;5/5;60/60|

57 includes init/cleanup and55substantive entries. Previous28 entries retained;
new29 add independently fixed source offsets and full USA reconstruction, replay/
cross-binding/source-tamper checks,26rehashed report variants and bounded/cancel/
deadline checks. Independent ntfs-3g-generated regular-file image/abc answer also
passes replay; no formatter/library/bootloader code is linked/shipped. Existing
fragmentation/deleted/sparse/bitmap/malformed/output/mutation tests remain. Current
UUID-binding tests do NOT substitute for real grant/session/revocation/crash tests.
Per-profile raw text/XML/full logs and subcase-skips.txt retained; zero skipped
CTest groups does not erase platform/tool/profile subcase exclusions. CRLF text
copies normalize to LF; provider archive size/digests identify original bytes.

## 6. Failures and limits

No exact-source hosted CI failures. Earlier normal LOCAL LSan body passed57 then
exit1 due fatal ptrace/runtime inspection restriction; original diagnostic retained.
Final local ASan/UBSan57/57 uses detect_leaks=0, not local leak acceptance. Hosted
normal22/22 is independent acceptance; no normal gate/test/warning/signature or
permission check was weakened. A transient connector polling response was retried;
it was not a CI test result. No historical Qt/audit/descendant root cause inferred.

## 7–8. Commit and PR

Implementation/accepted source `1a4471d89f98d4bf91c03a202b32786cb8e264fa`. Later evidence checkpoint HEAD resolves
through Git and has separate CI. [PR17](https://github.com/nexvary/NEXVARY-MR/pull/17)
remains draft; [issue16](https://github.com/nexvary/NEXVARY-MR/issues/16) open. No merge.

## 9. Windows artifacts and actual executable verification

- [Portable unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698401/artifacts/12843801568); providerSHA25639938a48f98af4f0ab6fabd023c025f495f041529964a2badcfe445d5b4ac3ab.
- [Installer unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698401/artifacts/12842389979); providerSHA2568e6ec4e1fcf8f5876d0a72b65f47539aa56854c013aff3cbe39a6174e071863a.
- [Windows Production receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698401/artifacts/12844120931).
- [Windows Laboratory receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40747698401/artifacts/12843442109).

All1419 manifest entries/exact file set,1418 SPDX entries and actual AMD64 main/
worker verified. Unsigned Qt6.8.3 production BUILD_PROFILE binds identical checkout.
Inner Portable ZIP10cc2865e3ba6605aa093120d6aa63d8cf5cf9746569d774c9d1d74933e9609f; actual setup PEbebda71746a502cc0d5cd7196bc4c1c55af6a67b992a02c7ae888278b393760f;
maind0cc234f5a87412213335c9e9274dc36b8d1930ba7786f6afb12413d49810537; worker1b51354c3212d3f9771f6b1aba90f2781bbee136ce74c19d9a1ddef3e69c8ef6.
Actual hosted Portable/installed main+worker hashes equal downloaded bytes;
launch0/install0/uninstall0, executable/per-user registration removal and independent
23-event public-policy audit each verified. No mere archive-creation acceptance.
Reproduce static inspection with existing verify_windows_bundle.py and retained
verify_windows_deployment.py; provider-artifacts.json and downloaded-artifacts-
verification.json identify original six checked archives. Packages deploy existing
desktop, NOT NTFS functionality. Physical Windows device rechecked offline: no new
physical/clean Windows11/hardware-renderer/phone/product-signing acceptance.

## 10. Open issues and broad stages

All9 broad stages in-progress. NTFS authorized case/signed retention/Findings/
Timeline/Reports and real UI enablement pending. exFAT/FAT32/recovery and Sessions/
profile work pending. Historical signed Audit Store disk/memory divergence root,
process grandchildren containment and original Qt SIGSEGV root/DPI soak/hardware
remain OPEN. Read-back fail-closed barrier is not an established original root-
cause repair. No certification, full recovered file or comparative superiority.

## 11. Next

NEXT_CASE_INTEGRATION.md records concrete existing-store/authority/replay-on-
persistence/strict-retained-projection/exact-kind/native-public-verifier and actual
cancel/revoke/expiry/replacement/restart/crash/AR-EN-scaled acceptance gates.
No second store, caller-controlled authority, path/device/destination writer or
premature enabled NTFS case claim.
