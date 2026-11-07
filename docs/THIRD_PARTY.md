# Runtime dependencies and distribution notices

This is a development build, not a signed commercial release. No external Android tool, manufacturer firmware or licensed schematic is bundled.

## Qt 6.8.3

The Windows C++ executable dynamically links Qt 6.8.3. Core, Gui, Qml, Quick, Quick Controls, Quick Layouts, Concurrent and Sql, together with their deployed runtime dependencies, are distributed as replaceable DLLs/QML modules and plug-ins. The runtime Qt SQLite driver is included. The packaging scripts do not statically link Qt or prevent replacing a compatible Qt build. Reverse engineering required for debugging modifications to LGPL-covered components is not prohibited by this project.

Qt is copyright The Qt Company Ltd. and its contributors. Qt modules have individual licenses, including commercial, LGPLv3 and GPL alternatives. This build uses the LGPL-permitted versions of the modules listed above; no Qt Charts, Qt Virtual Keyboard or other GPL-only module is intentionally selected. A final release distributor must audit the exact deployed DLL/module manifest and all embedded third-party notices.

Full LGPLv3 and GPLv3 license texts are included in the `licenses` directory. The latter is incorporated by reference in LGPLv3; its presence does not assert that the application source itself is GPL-licensed. If Qt's installed SDK contains a `licenses` directory, that directory is also copied into the package. The executable folder includes a SHA256 manifest of the actual distributed files.

Corresponding unmodified Qt 6.8.3 source and module notices:

- [Qt 6.8.3 official source archive](https://download.qt.io/archive/qt/6.8/6.8.3/single/qt-everywhere-src-6.8.3.tar.xz)
- [Qt Base source and third-party metadata](https://github.com/qt/qtbase/tree/v6.8.3)
- [Qt Declarative source and third-party metadata](https://github.com/qt/qtdeclarative/tree/v6.8.3)
- [Qt SVG source](https://github.com/qt/qtsvg/tree/v6.8.3)
- [Qt Image Formats source](https://github.com/qt/qtimageformats/tree/v6.8.3)
- [Qt Shader Tools source](https://github.com/qt/qtshadertools/tree/v6.8.3)
- [Qt licensing and open-source obligations](https://doc.qt.io/qt-6/licensing.html)

The selected sources and license obligations must stay available alongside any public binary distribution, including an appropriate source-access mechanism under LGPLv3. These upstream URLs identify matching sources; they are not a substitute for a final distributor's compliance review or a written source offer where required. Modified Qt builds, if ever used, require publishing their corresponding source modifications. No such modifications are currently made.

## SQLite

SQLite is provided inside Qt's QSQLITE plug-in. SQLite's own code is dedicated to the public domain; see [SQLite copyright](https://www.sqlite.org/copyright.html). Qt's driver code remains subject to Qt's licensing. SQLite customer databases and backups are ordinary unencrypted local files: protect the Windows account and storage (for example, with Windows device encryption/BitLocker), limit access, and keep backups secured.

## MSVC runtime

The Windows package includes app-local **release x64** Microsoft Visual C++ CRT DLLs from the official Visual Studio 2022 `VC/Redist/MSVC/<version>/x64/Microsoft.VC143.CRT` directory. The build locates VS2022 with `vswhere`, selects the newest installed versioned release redistributable directory, copies that directory's DLL set unchanged, and refuses an incomplete package when `msvcp140.dll`, `vcruntime140.dll` or `vcruntime140_1.dll` is absent. It does not copy Windows system DLLs, debug runtimes or arbitrary compiler binaries. `CRT_DEPLOYMENT.json` records the source directory, DLL versions and SHA256 hashes; `SHA256SUMS.txt` covers the resulting package files.

Microsoft owns these binaries. They remain subject to the Visual Studio license terms and redistributable-code list; they are not licensed as open-source project code. The bundled notice identifies the applicable official sources:

- [Visual Studio 2022 redistribution list](https://learn.microsoft.com/en-us/visualstudio/releases/2022/redistribution)
- [Microsoft Visual C++ redistribution requirements](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files?view=msvc-170)
- [Microsoft app-local deployment method](https://learn.microsoft.com/en-us/cpp/windows/choosing-a-deployment-method?view=msvc-170)

App-local deployment preserves portable and per-user installation without requiring a machine-wide redistributable installer or administrator rights. The Universal CRT is supplied by supported Windows 10/11; it is not copied from the build machine. Bundled app-local CRT security fixes require updating the application's package. A GitHub-hosted runner already has developer runtimes installed, so even an isolated-path smoke test does not prove the absence of all system-level prerequisites. Validate the downloaded portable and installer on a clean supported Windows 10/11 x64 VM before public release.

## Optional external tools

- [scrcpy](https://github.com/Genymobile/scrcpy): Apache-2.0; the unchanged official win64 5.0.1 distribution is bundled for explicitly consented screen viewing in both profiles. The pinned archive and complete file inventory are retained in SOURCE_PROVENANCE.json; upstream LICENSE.txt stays unchanged. Its FFmpeg runtime identifies LGPL-2.1-or-later (text included), with additional SDL, libusb and dav1d dependencies. Matching source/build recipes, provenance, limitations and pending license review are listed in [SCREEN_VIEW.md](SCREEN_VIEW.md). Generic service writes remain Laboratory-only. Physical display/cleanup and clean Windows acceptance remain pending.
- [Android SDK platform-tools](https://developer.android.com/tools/releases/platform-tools): ADB and its Windows DLLs are retained as included in that official scrcpy archive. Their upstream terms/notices and redistribution obligations require independent review before final distribution; this development package is not a completed legal acceptance gate.
- OpenBoardView and TRACE Boardviewer are researched, separately licensed products. No source, executable, icons or file parsers from them are bundled.

See [INTEGRATIONS.md](INTEGRATIONS.md) for pinned upstream revisions and the rejected/ambiguous candidates. Reviewing a repository does not mean it has been integrated.

## Installer and CI

Inno Setup is a build-time tool; the installer contains only the application's deployed runtime. CI builds unsigned portable ZIP and setup EXE artifacts. A SHA256 file checks file integrity; it does not establish publisher identity. Code signing, antivirus reputation, independent Windows acceptance and complete dependency-license review remain release gates.

## Recovery increment

The C++ image scanner and case workflow are original project code; no PhotoRec, TSK, ALEAPP, MVT, libewf or other recovery application code/binary is bundled. The optional TSK adapter invokes a separately installed and explicitly selected mmls executable on a temporary verified image copy; it does not link to or redistribute libtsk. The native Windows case vault calls the operating system's CryptProtectData/CryptUnprotectData APIs (Crypt32), with current-user scope and integrity protection. No Windows system DLL is redistributed. [Microsoft DPAPI documentation](https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata) describes account/machine binding and its limits.

The optional Android companion uses the official MIT Shizuku API/provider13.1.5, downloaded through pinned Gradle dependencies. Its APK assets include the complete upstream MIT notice, copyright2021 RikkaW. Shizuku manager is not bundled. The Gradle8.9 wrapper is an Apache2.0 build-time binary with pinned SHA256; matching source/notices are linked in the companion README. No runtime Python foundation has been added to the desktop. Recovery tool research and its independent PhotoRec/Linux fixture experiment do not imply integration or Windows acceptance of those external applications.

## Fluent navigation SVGs

Twenty-four unmodified Microsoft Fluent System Icons color SVGs are bundled. MIT copyright/permission notice: `licenses/FLUENT-ICONS-MIT.txt`. Exact upstream revision, paths, Git blob IDs and SHA256 digests are recorded in `assets/icons/PROVENANCE.json` in the source repository. SVG decoding uses Qt SVG (matching Qt 6.8.3), deployed dynamically as Qt6Svg.dll and qsvg.dll and covered by the Qt source/license links above. No emoji or font glyph is used for navigation.


## Workshop Code 39 labels

The encoding table in `cpp/src/Code39Label.hpp` derives from ZXing-C++ **v2.3.0** `core/src/oned/ODCode39Writer.cpp` (Apache-2.0; Copyright 2016 Huy Cuong Nguyen and ZXing authors). The table is used with an original Qt renderer restricted to `NMR-` plus 16 uppercase hex characters. Source: https://github.com/zxing-cpp/zxing-cpp/blob/v2.3.0/core/src/oned/ODCode39Writer.cpp . The Apache license and attribution are in `packaging/licenses/ZXING-CODE39-*`, automatically copied by the Windows packager. No ZXing decoder/runtime is linked. There is no optional Mod-43 check character in these standard Code 39 labels; a successful lookup still requires operator review of the displayed order/device. The barcode is not an authentication or public tracking credential.

Independent test decoding uses separately installed ZBar and Poppler on synthetic images/PDFs. These test-only tools and their libraries are not bundled. Scanner/printer hardware, Windows rendering and final distribution compliance remain acceptance gates.

Windows CI offscreen checks use copies of six fonts already installed on the hosted Windows runner (Segoe UI, Tahoma and Consolas regular/bold). These stay in RUNNER_TEMP, are excluded from artifacts and packages, and are not repository assets or redistributed dependencies. Native deployment validation clears this test-only font override and uses the operating system's font facilities. The application adds no font download or telemetry.

`assets/icons/check_mark.svg` is an original NEXVARY control asset. Checked indicators use this image rather than a font-dependent Unicode symbol; deployment smoke requires its SVG decoder and QML verifies its loaded/visible state.

## Internal signed-update verification tests

The separate `mr_update_verifier` C++ library uses [libsodium 1.0.22](https://download.libsodium.org/libsodium/releases/) under the ISC license (Copyright 2013–2026 Frank Denis). Its complete upstream notice is included as `licenses/LIBSODIUM-ISC.txt`. The official point-release source and MSVC archives have reviewed minisign signatures and pinned SHA256 hashes in `samples/update/dependency-provenance.json`; preparation verifies those hashes before extraction. Linux builds a static PIC library and runs upstream self-tests. Windows tests use the official x64 Release v143 dynamic DLL/import library. The desktop now also links this pinned dependency for local Argon2id credentials, secretbox-protected per-user signing keys and Ed25519 research/audit attestations. Windows portable/installer packages include the official Release DLL and its ISC notice; Linux uses the prepared static library. The SHA-pinned dependency is a build/test input, not a production trust anchor. No private update signing key is stored. See [UPDATE_VERIFIER.md](UPDATE_VERIFIER.md) for protocol and outstanding installation gates.

The internal update journal uses the same pinned libsodium dependency for HMAC-SHA512-256 and local MAC-key generation. Its Windows key-store factory reuses the project's existing current-user DPAPI provider; `CaseVault.cpp` is now compiled once into a shared static project target. No new encryption library or Windows system binary is redistributed. The protected local MAC key is distinct from a publisher update signing key. Linux key persistence refuses without a plaintext fallback. Existing case formats are unchanged; OS-account recovery, ACL enforcement and protected anti-replay anchors remain acceptance gates.
