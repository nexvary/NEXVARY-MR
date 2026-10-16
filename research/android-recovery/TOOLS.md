# NEXVARY-MR recovery tooling — read-only research

Research snapshot: 2026-10-16 (UTC). This register records upstream evidence and integration decisions. Research is distinct from a runtime integration. No external recovery worker is currently integrated with NEXVARY-MR; upstream platform claims are not MR acceptance results.

## Decision summary

The recovery pilot design is **image-first analysis and recovery**: user-authorized, already acquired raw/DD or supported EWF evidence; immutable source; separate output volume/folder; bounded work; SHA256 provenance and explicit partial/error states. Start with a pinned PhotoRec external carving worker and independently compare TSK filesystem exports. Add ALEAPP/iLEAPP as optional artifact-report workers, and evaluate libewf for explicit EWF verification/export. Keep acquisition, spyware triage and filesystem repair separate.

No investigated tool proves universal recovery of deleted files from a modern stock Android phone. A raw `userdata` acquisition is not necessarily plaintext or decryptable. AOSP requires FBE on devices launching Android 10+; CE data is tied to unlock state and protected keys. Unlocking a bootloader calls for a factory reset. Therefore **do not unlock/root a customer device as an automatic recovery preparation step**. These are engineering constraints, not missing button implementations. Sources: [AOSP FBE](https://source.android.com/docs/security/features/encryption/file-based), [AOSP bootloader lock/unlock](https://source.android.com/docs/core/architecture/bootloader/locking_unlocking). The conclusion about raw ciphertext recovery is an inference from these documented protections.

## Capability and integration matrix

All Windows/build support below is upstream evidence, **not tests run by this research**. “Root” means phone superuser access; OS administrator rights to a local disk are a different permission. Existing readable image analysis generally needs neither phone root nor administrator access.

| Tool | Version/activity observed | Actual role and access boundary | Windows evidence | License / commercial integration decision |
|---|---|---|---|---|
| TestDisk / PhotoRec | Stable 7.2 (2024-02-22); 7.3-WIP; HEAD 2026-10-03 | TestDisk repairs partitions/boot sectors and can write media; PhotoRec carves files from DD/raw/E01/media. Offline image analysis needs no phone root. Physical media access needs OS privileges; no access to protected Android storage is supplied. | Official Windows x64/32 downloads; QPhotoRec GUI. | GPL-2.0-or-later confirmed in photorec.h. Prefer PhotoRec external worker with image-only source and separate destination. Do not expose TestDisk repair/write actions as ordinary recovery. [T1] |
| GNU ddrescue | 1.30, 2026-01-01 announcement; source hash verified | Acquisition/rescue of readable blocks with persistent mapfile and resume; it is not a file parser or undelete engine. Needs a readable file/block device; cannot make ordinary ADB expose encrypted phone sectors. | POSIX recommended; official manual warns about Cygwin error handling. No official native Windows portable binary verified. | GPL-2.0-or-later in 1.30 main.cc/COPYING. Optional Linux acquisition station later; not first Windows worker. Never substitute similarly named dd_rescue. [T2] |
| The Sleuth Kit (TSK) | 4.15.0 release 2026-04-15; HEAD 2026-10-14 | Filesystem/volume-aware analysis via library and CLIs; reads pre-acquired images without phone root. Deleted-file reconstruction depends on surviving metadata/content and supported filesystem. | Official release has sleuthkit-4.15.0-win32.zip; source supports Visual Studio 32/64-bit. Package name alone does not prove artifact architecture. | Mixed licenses: historical IPL/CPL, Apache-2.0 Java/case code, and other file-specific licenses. Prefer a pinned external CLI pilot before any C++ library link; audit exact compiled components. [T3] |
| Autopsy | 4.23.1 release 2026-05-07; HEAD 2026-05-04 | Complete evidence-analysis GUI using TSK; useful cross-check/reference workstation. Imports existing evidence; not a general way to acquire locked or deleted mobile storage. | Official 64-bit MSI release; Windows dependencies bundled. | Autopsy code Apache-2.0; dependency license list is mixed. Launch separately or export a case manifest for manual import; do not embed the full Java/NetBeans application into Qt. [T4] |
| libewf | HEAD 2026-10-12; no GitHub latest-release object returned | C library and ewftools for EWF inspection/verification/export. Current README marks project experimental; Ex01 encryption and Ex01 bzip2 explicitly unsupported. Logical L01/Lx01 read-only support listed. | MSVC solution/build scripts and Windows build support exist; no ready official release asset validated in this review. | LGPL-3.0-or-later confirmed in README/header. Good dynamic-library or external ewfinfo/ewfverify/ewfexport candidate after corruption/fuzz/format tests. No universal decrypt claim. [T5] |
| bulk_extractor | 2.2.0 release 2026-08-18; HEAD 2026-10-12 | Scans raw bytes/images/files for features and embedded encodings; no need to mount a filesystem. Supplementary triage, not filesystem-aware reconstruction or phone acquisition. | Official release has bulk_extractor64.exe. HEAD states native Windows builds unsupported; current MinGW cross-build + Windows smoke described. | Post-NPS project code GPL-3.0-or-later; LICENSE.md explicitly supersedes older MIT statement. Original NPS government work only is public-domain. Optional external image worker with complete matching-source notices. [T6] |
| Scalpel | HEAD 2021-03-26; README says no official releases and not maintained | Header/footer carving. Upstream warns integration effort was abandoned after memory leaks. Cannot recover overwritten/encrypted content. | Old README describes Windows MinGW 32-bit and says MSVC unsupported; not modern Windows acceptance. | Current sleuthkit fork Apache-2.0, not automatic GPL assumption from older Scalpel history. Reject as default engine; research/reference only until independent memory-safety modernization. [T7] |
| Foremost | Official tarball 1.5.7; CHANGES/Makefile timestamps 2010-03-02 | Legacy signature/data-structure carver for image files or drives. No modern Android storage acquisition or cryptographic key recovery. | Official archive has Win32/MinGW build paths; no supported modern Windows x64 artifact independently verified. | Official source/manpage give US-government/no-copyright notices; extract.c has separate Nick Mikus copyright, and no separate COPYING/LICENSE file was found in reviewed archive. Do not guess GPL or blanket public-domain permission; commercial redistribution remains unresolved. [T8] |
| ALEAPP | v2026.4.3 release 2026-10-02; HEAD 2026-10-16 | Offline Android artifact parsing/reporting, not live acquisition. HEAD adds raw-image input via ewfprobe/qnxprobe; ordinary extracted files/ZIP/TAR also accepted. Root not needed for already accessible evidence. | Official current Windows x64/ARM64 portable and installer assets. Executables are Python-based packaged workers. | MIT LICENSE; parser/dependency attributions still required. Strong external-report-worker candidate that keeps C++/Qt as commercial core; independently test pinned schemas. [T9] |
| iLEAPP | v2026.4.4 release 2026-10-02; HEAD 2026-10-16 | Offline iOS/iPadOS extraction/backup parser; encrypted iTunes/Finder backup requires supplied password. Not device unlock, raw iPhone acquisition or automatic deleted-storage recovery. | Official Windows x64/ARM64 portable/installer release assets; packaged Python worker. | MIT LICENSE with dependency/asset review. Strong backup/report adapter candidate; pass legitimate secrets through secure non-command-line channel in any future adapter. [T10] |
| AndroidQF (Amnesty fork) | v1.8.3 release 2026-05-12; HEAD 2026-10-12 | Consensual ADB logical/diagnostic evidence collection for compromise triage; requires USB debugging/device authorization. Collector is executed on the phone, so acquisition is not zero-change. Not generic deleted-file recovery. | Official unsigned Windows amd64 EXE in current release; Go build. | Custom MVT License 1.1, including explicit freely informed Data Owner consent and source-distribution conditions. Separate license/use-case review required; defer from ordinary repair/recovery MVP. [T11] |
| MVT | v2026.10.12 release 2026-10-12; HEAD 2026-10-09 | Android/iOS compromise/spyware indicator analysis, not undelete. Works on acquisitions/backups; IOC absence does not prove a clean device. | Official docs do not support native Windows; recommend WSL. Python 3.10+. | Custom MVT License 1.1, not unmodified MPL2/GPL. Specialized optional analysis only after license/privacy workflow review; no claim it is core recovery. [T12] |
| VigneshVijayK/android-forensics | Created 2026-09-12; single observed HEAD 2026-09-12; no latest release verified | README claims root-based userdata imaging, carving and logical extraction. Non-root sees accessible files only. Claims that raw imaging automatically recovers deleted Android files ignore FBE/key/discard conditions; no independent acceptance evidence reviewed. | README explicitly Linux-only; Windows/macOS future work. Python, not C++. | MIT root license but independently bundled/integrated tool licenses need review. Reference only; do not adopt its device-rooting, broad forensic or court-admissibility marketing claims. [T13] |
| RafalB82/forensic | Created 2026-09-30; HEAD 2026-10-01; pyproject version 0.25.0 beta | Offline ext4/F2FS/EROFS partition-image readers and artifacts; no root for direct image readers, optional legacy mount requires elevated access. README refuses encrypted F2FS. Whole suite also includes PIN/credential and external EDL modules outside NEXVARY-MR scope. | Linux/Python >=3.10 documented; no native Windows support proven. | GPL-3.0-or-later in pyproject/README. Narrow offline-parser research only; exclude PIN search, credential disclosure and external EDL acquisition from integration. [T14] |
| Crush Forensics (new 2026) | Created 2026-03-15; v0.22.0 release 2026-10-08; HEAD 2026-10-16 | Offline evidence viewer for images, archives, mobile backups and databases; useful viewer/provenance reference. Keys must be user supplied for supported encrypted databases; no key recovery claimed. | Project advertises Windows packages and Windows forensic test suite; not executed here. Python/Qt-based separate app. | Apache-2.0 LICENSE. Good optional independently installed evidence viewer candidate; audit dependencies and verify no-change tests with own fixtures. [T15] |
| ewfprobe / qnxprobe (new 2026) | Created 2026-09-08 / 2026-08-27; HEAD 2026-10-15 | Read-only pure-Python image-container and filesystem readers used by current LEAPP stack. No mounting/admin needed for readable images; broad format claims remain per-format acceptance work. Supplied-password formats do not imply Android FBE key recovery. | Upstream describes cross-platform Python; no stand-alone signed Windows x64 package acceptance performed. | MIT root licenses. Prototype external-worker alternatives/cross-checks to libewf/TSK, not substitutes for C++ core or mature-validation evidence. [T16] |
| Amele (new 2026; rejected) | Created 2026-03-15; v0.1.1 release 2026-10-08; HEAD 2026-10-10 | Vendor describes acquisition and read-only viewing. No source-level functional integration assessed. | Vendor Windows/Linux claim; no acceptance test here. | Actual current LICENSE is proprietary EULA, limits commercial resale/monetization and reverse engineering. Exclude source/binary embedding or resale absent explicit rights; public GitHub presence is not open-source permission. [T17] |

## Pinned GitHub revisions and primary evidence

Commit dates are commit metadata, not independent proof of product quality. Latest releases are repository `releases/latest` observations; absence means no latest-release object returned, not proof that no distribution exists elsewhere. README functionality can be newer than the named released binary. Always pin a release and test that exact binary before building an adapter.

### T1 — cgsecurity/testdisk

- [Pinned HEAD `0aa4bc2b68378bc2bd2e92be3a465f0957e0f026`](https://github.com/cgsecurity/testdisk/commit/0aa4bc2b68378bc2bd2e92be3a465f0957e0f026), committed 2026-10-03T16:34:10Z; repository created 2015-06-13T15:18:03Z.
- [Scope/platform README](https://github.com/cgsecurity/testdisk/blob/0aa4bc2b68378bc2bd2e92be3a465f0957e0f026/README.md); [license evidence](https://github.com/cgsecurity/testdisk/blob/0aa4bc2b68378bc2bd2e92be3a465f0957e0f026/src/photorec.h).

### T3 — sleuthkit/sleuthkit

- [Pinned HEAD `e08bdd2f9b74b319981144c03ee492e9287b9e0f`](https://github.com/sleuthkit/sleuthkit/commit/e08bdd2f9b74b319981144c03ee492e9287b9e0f), committed 2026-10-14T20:15:07Z; repository created 2011-10-12T14:26:49Z.
- [Scope/platform README](https://github.com/sleuthkit/sleuthkit/blob/e08bdd2f9b74b319981144c03ee492e9287b9e0f/README_win32.txt); [license evidence](https://github.com/sleuthkit/sleuthkit/blob/e08bdd2f9b74b319981144c03ee492e9287b9e0f/licenses/README.md).
- [Latest observed release sleuthkit-4.15.0](https://github.com/sleuthkit/sleuthkit/releases/tag/sleuthkit-4.15.0), published 2026-04-15T18:09:44Z; release assets: `sleuthkit-4.15.0-win32.zip`, `sleuthkit-4.15.0-win32.zip.asc`, `sleuthkit-4.15.0.tar.gz`, `sleuthkit-4.15.0.tar.gz.asc`.

### T4 — sleuthkit/autopsy

- [Pinned HEAD `cb3dacdcad67abe7cf863c74f10dcdb8e25a5c21`](https://github.com/sleuthkit/autopsy/commit/cb3dacdcad67abe7cf863c74f10dcdb8e25a5c21), committed 2026-05-04T17:07:28Z; repository created 2011-10-05T02:11:11Z.
- [Scope/platform README](https://github.com/sleuthkit/autopsy/blob/cb3dacdcad67abe7cf863c74f10dcdb8e25a5c21/README.txt); [license evidence](https://github.com/sleuthkit/autopsy/blob/cb3dacdcad67abe7cf863c74f10dcdb8e25a5c21/LICENSE-2.0.txt).
- [Latest observed release autopsy-4.23.1](https://github.com/sleuthkit/autopsy/releases/tag/autopsy-4.23.1), published 2026-05-07T15:23:27Z; release assets: `autopsy-4.23.1-64bit.msi`, `autopsy-4.23.1-64bit.msi.asc`, `autopsy-4.23.1.zip`, `autopsy-4.23.1.zip.asc`.

### T5 — libyal/libewf

- [Pinned HEAD `33e8dccf18aa0dbb26cee908a3ad122011abbdaf`](https://github.com/libyal/libewf/commit/33e8dccf18aa0dbb26cee908a3ad122011abbdaf), committed 2026-10-12T05:20:02Z; repository created 2014-10-02T21:10:50Z.
- [Scope/platform README](https://github.com/libyal/libewf/blob/33e8dccf18aa0dbb26cee908a3ad122011abbdaf/README); [license evidence](https://github.com/libyal/libewf/blob/33e8dccf18aa0dbb26cee908a3ad122011abbdaf/include/libewf.h.in).

### T6 — simsong/bulk_extractor

- [Pinned HEAD `e86383947ee016c825599887334cacf011736abf`](https://github.com/simsong/bulk_extractor/commit/e86383947ee016c825599887334cacf011736abf), committed 2026-10-12T00:57:38Z; repository created 2012-04-03T04:36:41Z.
- [Scope/platform README](https://github.com/simsong/bulk_extractor/blob/e86383947ee016c825599887334cacf011736abf/README.md); [license evidence](https://github.com/simsong/bulk_extractor/blob/e86383947ee016c825599887334cacf011736abf/LICENSE.md).
- [Latest observed release v2.2.0](https://github.com/simsong/bulk_extractor/releases/tag/v2.2.0), published 2026-08-18T10:20:24Z; release assets: `bulk-extractor_2.2.0_amd64.snap`, `bulk-extractor_2.2.0_arm64.snap`, `bulk_extractor-2.2.0.tar.gz`, `bulk_extractor64.exe`, `SHA256SUMS`.

### T7 — sleuthkit/scalpel

- [Pinned HEAD `35e1367ef2232c0f4883c92ec2839273c821dd39`](https://github.com/sleuthkit/scalpel/commit/35e1367ef2232c0f4883c92ec2839273c821dd39), committed 2021-03-26T13:29:17Z; repository created 2013-06-27T16:59:40Z.
- [Scope/platform README](https://github.com/sleuthkit/scalpel/blob/35e1367ef2232c0f4883c92ec2839273c821dd39/README); [license evidence](https://github.com/sleuthkit/scalpel/blob/35e1367ef2232c0f4883c92ec2839273c821dd39/LICENSE-2.0.txt).

### T9 — abrignoni/ALEAPP

- [Pinned HEAD `e31e3023c56cb9c40338a6ef3ba0d84fd7cdc074`](https://github.com/abrignoni/ALEAPP/commit/e31e3023c56cb9c40338a6ef3ba0d84fd7cdc074), committed 2026-10-16T04:13:38Z; repository created 2020-02-22T19:55:39Z.
- [Scope/platform README](https://github.com/abrignoni/ALEAPP/blob/e31e3023c56cb9c40338a6ef3ba0d84fd7cdc074/README.md); [license evidence](https://github.com/abrignoni/ALEAPP/blob/e31e3023c56cb9c40338a6ef3ba0d84fd7cdc074/LICENSE).
- [Latest observed release v2026.4.3](https://github.com/abrignoni/ALEAPP/releases/tag/v2026.4.3), published 2026-10-02T00:35:36Z; release assets: `ALEAPP-2026.4.3-linux-arm64.AppImage`, `ALEAPP-2026.4.3-linux-x64.AppImage`, `ALEAPP-2026.4.3-macos-arm64.dmg`, `ALEAPP-2026.4.3-macos-x64.dmg`, `ALEAPP-2026.4.3-windows-arm64-portable.zip`, `ALEAPP-2026.4.3-windows-arm64-setup.exe`, `ALEAPP-2026.4.3-windows-x64-portable.zip`, `ALEAPP-2026.4.3-windows-x64-setup.exe`, `SHA256SUMS.txt`.

### T10 — abrignoni/iLEAPP

- [Pinned HEAD `45b6016f84ae8ae55359838879ff9b7c0234be47`](https://github.com/abrignoni/iLEAPP/commit/45b6016f84ae8ae55359838879ff9b7c0234be47), committed 2026-10-16T11:37:09Z; repository created 2019-12-24T00:28:42Z.
- [Scope/platform README](https://github.com/abrignoni/iLEAPP/blob/45b6016f84ae8ae55359838879ff9b7c0234be47/README.md); [license evidence](https://github.com/abrignoni/iLEAPP/blob/45b6016f84ae8ae55359838879ff9b7c0234be47/LICENSE).
- [Latest observed release v2026.4.4](https://github.com/abrignoni/iLEAPP/releases/tag/v2026.4.4), published 2026-10-02T00:35:33Z; release assets: `iLEAPP-2026.4.4-linux-arm64.AppImage`, `iLEAPP-2026.4.4-linux-x64.AppImage`, `iLEAPP-2026.4.4-macos-arm64.dmg`, `iLEAPP-2026.4.4-macos-x64.dmg`, `iLEAPP-2026.4.4-windows-arm64-portable.zip`, `iLEAPP-2026.4.4-windows-arm64-setup.exe`, `iLEAPP-2026.4.4-windows-x64-portable.zip`, `iLEAPP-2026.4.4-windows-x64-setup.exe`, `SHA256SUMS.txt`.

### T11 — mvt-project/androidqf

- [Pinned HEAD `0c2278b3aec385bdc0da7065856406ce225cd38f`](https://github.com/mvt-project/androidqf/commit/0c2278b3aec385bdc0da7065856406ce225cd38f), committed 2026-10-12T10:25:38Z; repository created 2023-03-06T16:27:07Z.
- [Scope/platform README](https://github.com/mvt-project/androidqf/blob/0c2278b3aec385bdc0da7065856406ce225cd38f/README.md); [license evidence](https://github.com/mvt-project/androidqf/blob/0c2278b3aec385bdc0da7065856406ce225cd38f/LICENSE).
- [Latest observed release v1.8.3](https://github.com/mvt-project/androidqf/releases/tag/v1.8.3), published 2026-05-12T15:37:41Z; release assets: `androidqf_linux_amd64_1.8.3`, `androidqf_linux_arm64_1.8.3`, `androidqf_macos_universal_1.8.3_signed`, `androidqf_windows_amd64_1.8.3_unsigned.exe`, `checksums.txt`.

### T12 — mvt-project/mvt

- [Pinned HEAD `b3aa443819972e3918097d5383157f0afdfd5c94`](https://github.com/mvt-project/mvt/commit/b3aa443819972e3918097d5383157f0afdfd5c94), committed 2026-10-09T10:52:44Z; repository created 2021-07-16T05:50:44Z.
- [Scope/platform README](https://github.com/mvt-project/mvt/blob/b3aa443819972e3918097d5383157f0afdfd5c94/README.md); [license evidence](https://github.com/mvt-project/mvt/blob/b3aa443819972e3918097d5383157f0afdfd5c94/LICENSE).
- [Latest observed release v2026.10.12](https://github.com/mvt-project/mvt/releases/tag/v2026.10.12), published 2026-10-12T09:13:55Z; release assets: .

### T13 — VigneshVijayK/android-forensics

- [Pinned HEAD `4bc711df95210f01e1cb0afb8382081b6789b760`](https://github.com/VigneshVijayK/android-forensics/commit/4bc711df95210f01e1cb0afb8382081b6789b760), committed 2026-09-12T17:54:23Z; repository created 2026-09-12T18:10:46Z.
- [Scope/platform README](https://github.com/VigneshVijayK/android-forensics/blob/4bc711df95210f01e1cb0afb8382081b6789b760/README.md); [license evidence](https://github.com/VigneshVijayK/android-forensics/blob/4bc711df95210f01e1cb0afb8382081b6789b760/LICENSE).

### T14 — RafalB82/forensic

- [Pinned HEAD `d3d50be8a475ed9ac6c662264b0675f360ebca7e`](https://github.com/RafalB82/forensic/commit/d3d50be8a475ed9ac6c662264b0675f360ebca7e), committed 2026-10-01T17:56:18Z; repository created 2026-09-30T13:12:12Z.
- [Scope/platform README](https://github.com/RafalB82/forensic/blob/d3d50be8a475ed9ac6c662264b0675f360ebca7e/README.md); [license evidence](https://github.com/RafalB82/forensic/blob/d3d50be8a475ed9ac6c662264b0675f360ebca7e/pyproject.toml).

### T15 — kalink0/crush-forensics

- [Pinned HEAD `f54b2517faae6733bfba84d6facf5c4d99e2459b`](https://github.com/kalink0/crush-forensics/commit/f54b2517faae6733bfba84d6facf5c4d99e2459b), committed 2026-10-16T11:18:09Z; repository created 2026-03-15T13:17:38Z.
- [Scope/platform README](https://github.com/kalink0/crush-forensics/blob/f54b2517faae6733bfba84d6facf5c4d99e2459b/README.md); [license evidence](https://github.com/kalink0/crush-forensics/blob/f54b2517faae6733bfba84d6facf5c4d99e2459b/LICENSE).
- [Latest observed release v0.22.0](https://github.com/kalink0/crush-forensics/releases/tag/v0.22.0), published 2026-10-08T16:24:30Z; release assets: `crush-forensic-audit.html`, `crush-forensic-audit.json`, `crush-linux-v0.22.0.AppImage`, `crush-macos-arm-v0.22.0.zip`, `crush-macos-intel-v0.22.0.zip`, `crush-windows-v0.22.0.zip`.

### T16a — abrignoni/ewfprobe

- [Pinned HEAD `9c77734b3a641e21b5d8536a5de8940078796053`](https://github.com/abrignoni/ewfprobe/commit/9c77734b3a641e21b5d8536a5de8940078796053), committed 2026-10-15T14:45:08Z; repository created 2026-09-08T18:43:32Z.
- [Scope/platform README](https://github.com/abrignoni/ewfprobe/blob/9c77734b3a641e21b5d8536a5de8940078796053/README.md); [license evidence](https://github.com/abrignoni/ewfprobe/blob/9c77734b3a641e21b5d8536a5de8940078796053/LICENSE).
- [Latest observed release v0.13.0](https://github.com/abrignoni/ewfprobe/releases/tag/v0.13.0), published 2026-10-15T15:06:08Z; release assets: `ewfprobe-v0.13.0-linux-arm64.tar.gz`, `ewfprobe-v0.13.0-linux-x64.tar.gz`, `ewfprobe-v0.13.0-macos-arm64.zip`, `ewfprobe-v0.13.0-macos-x64.zip`, `ewfprobe-v0.13.0-windows-arm64.zip`, `ewfprobe-v0.13.0-windows-x64.zip`, `ewfprobe.py`, `ewfprobe_gui.py`, `SHA256SUMS.txt`.

### T16b — abrignoni/qnxprobe

- [Pinned HEAD `08cbd7b53c7d51dab47f2cc5dd8dfde86c064465`](https://github.com/abrignoni/qnxprobe/commit/08cbd7b53c7d51dab47f2cc5dd8dfde86c064465), committed 2026-10-15T19:31:05Z; repository created 2026-08-27T07:27:20Z.
- [Scope/platform README](https://github.com/abrignoni/qnxprobe/blob/08cbd7b53c7d51dab47f2cc5dd8dfde86c064465/README.md); [license evidence](https://github.com/abrignoni/qnxprobe/blob/08cbd7b53c7d51dab47f2cc5dd8dfde86c064465/LICENSE).
- [Latest observed release v1.68](https://github.com/abrignoni/qnxprobe/releases/tag/v1.68), published 2026-10-15T19:38:47Z; release assets: `qnxprobe-v1.68-linux-arm64.tar.gz`, `qnxprobe-v1.68-linux-x64.tar.gz`, `qnxprobe-v1.68-macos-arm64.zip`, `qnxprobe-v1.68-macos-x64.zip`, `qnxprobe-v1.68-windows-arm64.zip`, `qnxprobe-v1.68-windows-x64.zip`, `SHA256SUMS.txt`.

### T17 — noirlang/amele

- [Pinned HEAD `e992a30506d60767015cd17aaf27475b72a17ad6`](https://github.com/noirlang/amele/commit/e992a30506d60767015cd17aaf27475b72a17ad6), committed 2026-10-10T09:43:44Z; repository created 2026-03-15T16:36:23Z.
- [Scope/platform README](https://github.com/noirlang/amele/blob/e992a30506d60767015cd17aaf27475b72a17ad6/README.md); [license evidence](https://github.com/noirlang/amele/blob/e992a30506d60767015cd17aaf27475b72a17ad6/LICENSE).
- [Latest observed release v0.1.1](https://github.com/noirlang/amele/releases/tag/v0.1.1), published 2026-10-08T13:24:02Z; release assets: `amele`, `amele-linux`, `amele-linux-x64.AppImage`, `amele-linux-x64.deb`, `amele-linux-x64.pkg.tar.zst`, `amele-linux-x64.rpm`, `amele-win.exe`, `amele-windows-x64.msi`.

### T2 — GNU ddrescue official source verification

- [GNU project page](https://www.gnu.org/software/ddrescue/ddrescue.html): states GPL version 2 or later.
- [GNU 1.30 release announcement](https://lists.gnu.org/archive/html/info-gnu/2026-01/msg00001.html); [official 1.30 source archive](https://ftp.gnu.org/gnu/ddrescue/ddrescue-1.30.tar.lz).
- Downloaded official source archive read-only; SHA256 `2264622d309d6c87a1cfc19148292b8859a688e9bc02d4702f5cd4f288745542`, matching published announcement. Inspected `COPYING` (GPLv2), `main.cc` header (v2-or-later) and `README` (mapfile/resume/block rescue), without compiling or executing.
- [Official manual](https://www.gnu.org/software/ddrescue/manual/ddrescue_manual.html): 1.30 / 2026-01-01; recommends POSIX and describes misleading Cygwin/Windows error handling. No unofficial Windows port is treated as GNU-supported.

### T8 — Foremost official source verification and unresolved licensing

- [Official project](https://foremost.sourceforge.net/) links [the 1.5.7 tarball](https://foremost.sourceforge.net/pkg/foremost-1.5.7.tar.gz). [Official manual](https://foremost.sourceforge.net/foremost.html) documents image carving and government-work notice.
- Downloaded archive SHA256 `502054ef212e3d90b292e99c7f7ac91f89f024720cd5a7e7680c3d1901ef5f34`; inspected archive members only, no build/execution. `CHANGES` and `Makefile` say 1.5.7; their embedded modification timestamps are 2010-03-02 (timestamps are not an authenticated release announcement).
- `main.c`, `main.h`, `engine.c` and `helpers.c` carry US-government-work notices. `extract.c` carries a separate 2005 Nick Mikus copyright. The archive has no separate COPYING/LICENSE document. This evidence does **not** establish a blanket GPL or unrestricted worldwide public-domain grant for the entire current archive. Commercial inclusion is deferred until clarified.

## Important distinctions and uncertainties

1. **PhotoRec vs TestDisk:** [official PhotoRec documentation](https://www.cgsecurity.org/wiki/PhotoRec) says read-only media access and separate recovery destination; [official downloads](https://www.cgsecurity.org/wiki/TestDisk_Download) distinguish stable 7.2 from 7.3-WIP. Partition repair is a separate write-capable TestDisk workflow and should not be included in a read-only engine adapter.
2. **AndroidQF upstream identity:** `mvt-project/androidqf` identifies itself as the Amnesty-maintained fork of `botherder/androidqf`. The older repository was also inspected: HEAD `2bc2b1b756b3259f57284492cdd9ecb41d4e23e7` (2026-02-04), latest observed release v1.6 (2023-08-04). Prefer the explicitly maintained fork for research; do not confuse their releases.
3. **MVT license restrictions:** Current pinned MVT and AndroidQF LICENSE files are MVT License 1.1. Section 3.0 requires freely informed explicit consent by the Data Owner; executable/source and Larger Work conditions also exist. This is a custom license adapted from MPL2, not unmodified MPL2 and not a standard unrestricted permissive license. An external process alone does not waive its conditions.
4. **MVT purpose/platform:** [official installation documentation](https://docs.mvt.re/en/latest/install/) says native Windows is not officially supported and recommends WSL. MVT analyzes compromise traces and public IOCs; its own README warns that absent matches cannot establish a device is clean.
5. **Recent code:** Vignesh/Rafal repositories were created in September 2026; ewfprobe/qnxprobe in September/August 2026; Crush in March 2026. Newness and frequent commits do not establish mature parser correctness, memory/resource safety, authenticity of upstream claims or hardware acceptance. Pin and test.
6. **Source licenses vs transitive binaries:** MIT/Apache root licenses do not relicense bundled tools/dependencies, fonts, GUI assets, test images or data. GPL/LGPL commercial use is not categorically forbidden, but redistribution and combined-work/source obligations must be evaluated for the exact architecture. Prefer separable workers and preserve corresponding-source access, notices, versions and hashes; do not assume separation alone resolves every obligation.
7. **No bypass scope:** Device rooting/unlocking, PIN search, token/key disclosure, EDL acquisition and lock bypass in broader suites are not approved simply because their offline-parser portions are useful. Keep them out of the proposed MR integration surface.
8. **Unproven claims:** The initial upstream review did not build/execute tools. The standalone Linux PhotoRec synthetic evaluation below is the only subsequent execution covered here; no Android hardware, Windows recovery artifact or real-world recovery rate was validated. No customer, overwritten or encrypted content is claimed recovered. README court-admissibility and universal-recovery language is not independently validated evidence.

## Proposed verification evidence before any recovery feature is enabled

- Start with original synthetic raw images and explicitly licensed public fixtures. Include allocated/deleted files, fragmented/truncated files, all-zero/TRIM-like images, malformed/corrupt EWF segments, unsupported encryption, oversized signatures, cancellation, disk-full output and long Unicode Windows paths.
- Hash source before/after; open the image read-only and prevent source/output path collisions; enforce image-only mode initially. If live-media recovery is added later, use a separate physical destination from the original recovery medium. Record offsets, declared vs validated length, confidence/validation failures and original-source digest for each candidate.
- Distinguish filesystem-restored paths, carved names, visible logical copies and parsed records. Report partial recovery and unrecoverable encrypted/overwritten areas honestly.
- Run each external worker with fixed argument vectors, explicit trusted executable selection, bounded time/output and no automatic downloads, shell strings or privileged access. Isolate untrusted parsers from the Qt UI process.
- Validate actual release binaries and their licenses/dependency manifests on clean Windows; compare selected outputs with an independent tool. The core MR base gates must remain green before this separate work begins.

See README.md for the implemented/optional/researched boundaries, LAB_PROTOCOL.md for the required validation procedure, and device-matrix.json for explicit not_run hardware entries. Any separate PhotoRec synthetic experiment is reported in the PhotoRec validation section below; it does not establish MR adapter or Windows/device integration.


## PhotoRec 7.2 standalone synthetic validation

Status: **passed for the two contiguous original synthetic fixtures below on Linux x86_64 only**. This is not MR adapter integration, Windows validation, Android hardware acquisition or a deleted/encrypted-media acceptance result. No binary/source is vendored into this repository. A ready-made official binary was executed; no PhotoRec build was performed.

### Official provenance

- [Official scripted-run documentation](https://www.cgsecurity.org/testdisk_doc/scripted_run.html) and [upstream scripted-run command reference](https://www.cgsecurity.org/wiki/Scripted_run).
- [Official Linux x86_64 binary archive](https://www.cgsecurity.org/testdisk-7.2.linux26-x86_64.tar.bz2), 3,105,736 bytes, computed SHA256 `19669b6d36314d6e531efdf836c768574e8a556d1e9db3c8f3c4e93a5092cb1c`.
- [Official 7.2 source archive](https://www.cgsecurity.org/testdisk-7.2.tar.bz2), 855,781 bytes, computed SHA256 `f8343be20cb4001c5d91a2e3bcd918398f00ae6d8310894a5a9f2feb813c283f`.
- Executed `photorec_static` SHA256 `2063ed2f649309e296bb880403391b97c9846b39ca7adfcc343c87bf7a5a1d38`; `/version` reported 7.2, February 2024, GCC 4.8. Archives were downloaded from official HTTPS URLs; hashes were computed locally. No upstream signature verification was performed, so these hashes identify the inspected bytes rather than independently authenticating the publisher.
- Inspected the downloaded 7.2 `src/photorec.h`: GPL version 2 or later; `COPYING` carries GPLv2. Optional libraries and redistribution still need their own notice/source review for the exact future Windows package.

### Exact invocation and image-only boundary

The successful argv, with per-run scratch paths represented below, was:

```text
photorec_static /log /logname <scratch>/photorec.log /d "<scratch>/recovered output" /cmd "<scratch>/synthetic source image.dd" partition_none,fileopt,everything,disable,jpg,enable,png,enable,search
```

These are distinct process arguments. Quotes above illustrate paths with spaces; a future Qt worker must use a program path and `QStringList`, without constructing a shell command. Options precede `/cmd`; its source path is followed by one comma-separated PhotoRec command-list argument. The chosen source was a regular 4MiB read-only fixture image, not `/dev/*`, a Windows physical-drive/volume path, phone storage or customer media. The `/cmd` mechanism itself can accept devices: **there is no PhotoRec image-only security switch**. A future image-only adapter must independently validate/restrict the source to an ordinary file and prevent output collision.

`partition_none` selected the whole unpartitioned fixture image. `fileopt,everything,disable,jpg,enable,png,enable,search` restricted the requested classes. PhotoRec created `recovered output.1`; `/d` names a prefix and the engine may increment suffixes. The current-working-directory/session/log side effects must stay inside a case scratch directory. Execution had a 25-second wall limit, 15-second CPU limit, 512MiB address-space limit and 64MiB per-output-file limit for this tiny experiment. No device elevation or mounting command was invoked.

The analogous Windows executable name is `photorec_win.exe`, documented upstream, but the Windows binary/argv, process supervision, cancellation, error/exit mapping and packaging have **not** been validated by this experiment.

### Observed results

| Fixture | Offset in original synthetic image | Bytes | Recovered result | SHA256 (fixture and recovered bytes) |
|---|---:|---:|---|---|
| Original MR PNG icon | 65,536 | 16,077 | `f0000128.png` | `db9efbd28bdd074e5d01b917e7d97807ba1e0238056b4e085ab82670dc775786` |
| Original MR 8×8 JPEG codec fixture | 1,048,576 | 634 | `f0002048.jpg` | `53a4ba51351bf9e0aa05627603e573a421be08713e9032247c0f486b7ed9ffe0` |

Image size: 4,194,304 bytes. Source image SHA256 before and after: `a2deaf45486fc2fe42f89f83d58aa7828efadf407061ba51d13b28a077fdce3a`. Process exit: 0. Elapsed process time observed: 0.008 seconds; this tiny case is not a performance benchmark. Log reported two recovered files, zero invalid files rejected and normal exit. Recovered files matched both original hashes; DFXML reported the correct starting offsets and file sizes.

A parser pitfall was observed in the genuine DFXML: each `<byte_run>` had `len=65536`, while `<filesize>` and actual output lengths were 16077 and 634. A future adapter must not treat the byte-run length as the recovered file size. Validate actual output bytes and preserve the separately reported extent/offset evidence.

Full per-run JSON, argv, binary/source provenance, version output, logs, DFXML and recovered fixture outputs were created in the task's external `photorec-validation` scratch evidence directory. No live/customer media was opened or modified. Before integration: repeat exact Windows-binary verification, cancellation/error/disk-full cases, Unicode paths, corrupted/fragmented images, source preservation, license/source notices and Qt worker tests. This successful pilot supports **engine evaluation readiness**, not a shipped external integration.
