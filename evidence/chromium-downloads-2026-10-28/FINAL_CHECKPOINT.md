# Protected Chromium Downloads — verified-source checkpoint, 2026-10-28

## 1. Branch / HEAD / accepted source

integration/forensic-engines-audit. Tested source `9ce344941158c0ce02d2dd9e87000dfa000f0d88`, tree `624e5eb5a7d7395026c531ab4229a7b03ceb7895`;
PR checkout `0fcd821774d45c9f75a060211728f96dc72f317f` has identical tree. Government unchanged
eb2322140956a55a50ee7494570f35573a1ce0c0. Resolve evidence checkpoint HEAD with
Git; it changes only unshipped evidence/state/research, not executable/test/workflow/
packaging inputs. Its full-tree CI is separate, not inherited. No main/Guardian/
Government/upstream write or merge.

## 2. Original source commits

Workbench8b35940b1c879e0ce5f5c13e0d6c89aeab023757;
Browserd7754cd8f051d285406403465b6d7daa3bc4b060;
DCd8e2baf61b43d044b33a1dd76a444d093c80023c.
Owner-scoped module permission and dependency review: docs/FORENSIC_SOURCE_PERMISSIONS.md.
No new production runtime/store/updater/signature authority or broad license claim.

## 3. Delivered functionality

Sixth allowlisted chromium-downloads reads protected offline downloads and optional
ordered rowid redirect chains, using the same SQLite family/table/schema/provenance
primitives as History. Exact raw values, source member/schema/data/overflow offsets,
case/session/grant/revision/member revalidation, existing prepared signed acceptance,
Findings/Timeline/Reports and actual AR/EN controls. Recorded paths are strings only;
no target opening, file existence/content/complete-download or recovery assertion.
Malformed/unsupported schemas/chains/types/aliasing/limits reject without partial
findings. Existing5adapters preserved; docs/CHROMIUM_DOWNLOADS.md defines limits.

## 4. Modified files

Implementation9ce3449 changed ChromiumDownloads/ChromiumDownloadsReader, shared
SqliteReaderSupport, History reuse, ForensicAdapters, LawCaseBridge/forensic bridge,
CaseTimeline/CaseOperationIntent, public policy verifier/interop, GovernmentAnalysisPage,
CMake and native/Qt/SQLite/Python tests/fixtures, permission/research/docs/state/evidence.
Exact inventory: git show --stat9ce3449; original input hashes and failure logs in
CHECKPOINT.md and source-sha256.json. This checkpoint adds only acceptance evidence.

## 5. Exact-source tests

[C++40693581027](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693581027):7/7 success.
[Python40693580930](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693580930) and
[public history40693581229](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693581229)
success for the same source.

| Scope | Result |
|---|---|
|Windows Production/Laboratory|52/52 CTest each; MSVC /WX|
|Linux Production/Laboratory|52/52 CTest each|
|Native forensic adapters, four profiles|30/30 each|
|Qt Quick, four profiles|59/59 each|
|Independent signed interoperability, four profiles|76/76 each|
|Combined SQLite readers|51/51 on Linux; Windows CTest target passes, numeric stdout not inferred|
|Production ASan/UBSan|21/21 under normal unchanged leak gate|
|DPI|default100/125/150 groups and repeated GUI passed|
|Root Python|58/58 preceding local result; exact hosted workflow succeeded|

Raw per-profile receipts/LastTest/XML and subcase-skips.txt retained. No suppressed
platform/tool/profile exclusions or disabled tests; zero skipped CTest groups is
not a claim of zero subcase skips. Text copies normalize CRLF to LF; original ZIP
provider digests identify original bytes. Earlier local LSan inspection was unavailable;
current hosted sanitizer acceptance is separate. No earlier History result reused.

## 6. Failures / causes

Current exact C++/Python/history workflows succeeded. Earlier local failures remain
in original implementation CHECKPOINT.md: mixed QVariant auto declaration; output
fixture not large enough to cross actual cap; exact-kind prepared intent refusing
new adapter until consistently allowlisted; initial warning fallthrough. Assertions,
cap, /WX, signatures, authorization and inventory checks remain. No historical
Audit Store/Qt root cause or common causal relationship is claimed. No new physical
Windows test could run because the connected device is offline.

## 7–8. Commits / PR

Implementation `9ce344941158c0ce02d2dd9e87000dfa000f0d88`; subsequent acceptance checkpoint resolve HEAD through
Git. [PR17](https://github.com/nexvary/NEXVARY-MR/pull/17) remains draft, Government
base. [Issue16](https://github.com/nexvary/NEXVARY-MR/issues/16) remains open. No merge.

## 9. Windows artifacts / actual bytes

- [Portable unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693581027/artifacts/12819871099), providerSHA25649ec416d5b0f202c9e1847f7127579e64e3ade67217825eb55ba224fd6bae488.
- [Installer unsigned](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693581027/artifacts/12819073181), providerSHA256db0f2b6ae2bee30f6f11323099365e91a352f37a62ee84db0da4d1966af21e87.
- [Windows Production receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693581027/artifacts/12818354653).
- [Windows Laboratory receipts](https://github.com/nexvary/NEXVARY-MR/actions/runs/40693581027/artifacts/12820270362).

All1419 SHA entries/exact file set,1418 SPDX entries and actual AMD64 main/worker
verified. Portable inner ZIPd671894d64846703f6dec422fee862765d33e3dadf079ed101a3f4a42e70c7a4; setup EXE8b219eac2937efbc749f795ff424091d61b0ada0387faa36fd8751802fe95d9b.
Main EXE5c90815a15d7903b15065aa13dda271174e83c1f7d0e6e0e8ef601dd5da0e1d0; worker9fc3840d56591046f4699e57d1bfef6e8a1d16130faf7307879de41a6b6c5f47.
BUILD_PROFILE binds Qt6.8.3 unsigned production to identical PR source tree.
Actual hosted portable/installed main/worker hashes match downloaded bytes; launch0,
install/uninstall0, executable/per-user registration removal, independent23-event
audits each checked. Installer PE/checksum checked, not just archive creation.
No new physical/clean Windows11/hardware/phone/product-signing acceptance.
Provider sizes/digests/expiry in provider-artifacts.json.

## 10. Open issues / broad stages

All9stages stay in-progress. Audit Store historical signed disk/memory divergence
root, process grandchildren containment, original Qt SIGSEGV root/DPI soak/hardware,
agency identity/trusted time/product signing/clean-host/independent examiner gates
remain open. No government certification/full recovery/competitor superiority claim.

## 11. Next increment

Read-only NTFS metadata prerequisite from the pinned DC source, separating USA/
attributes/runlists from Image(QFile)/Output::save. No formatter/repair/erase/rebuild/
destination writer imports. Internal prerequisite tests are a separate new source
and do not inherit this Downloads acceptance. Case integration, exFAT/FAT32,
Sessions/profile metadata and recovery remain separately bounded future work.
