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

- [scrcpy](https://github.com/Genymobile/scrcpy): Apache-2.0; detected and started as a separate user-installed application. Its dependency notices apply to its own package.
- [Android SDK platform-tools](https://developer.android.com/tools/releases/platform-tools): Google SDK terms; obtain directly from Google. No binary redistribution is performed here.
- OpenBoardView and TRACE Boardviewer are researched, separately licensed products. No source, executable, icons or file parsers from them are bundled.

See [INTEGRATIONS.md](INTEGRATIONS.md) for pinned upstream revisions and the rejected/ambiguous candidates. Reviewing a repository does not mean it has been integrated.

## Installer and CI

Inno Setup is a build-time tool; the installer contains only the application's deployed runtime. CI builds unsigned portable ZIP and setup EXE artifacts. A SHA256 file checks file integrity; it does not establish publisher identity. Code signing, antivirus reputation, independent Windows acceptance and complete dependency-license review remain release gates.

## Recovery increment

The C++ image scanner and case workflow are original project code; no PhotoRec, TSK, ALEAPP, MVT, libewf or other recovery application code/binary is bundled. The native Windows case vault calls the operating system's CryptProtectData/CryptUnprotectData APIs (Crypt32), with current-user scope and integrity protection. No Windows system DLL is redistributed. [Microsoft DPAPI documentation](https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata) describes account/machine binding and its limits.

The optional Android companion uses the official MIT Shizuku API/provider13.1.5, downloaded through pinned Gradle dependencies. Its APK assets include the complete upstream MIT notice, copyright2021 RikkaW. Shizuku manager is not bundled. The Gradle8.9 wrapper is an Apache2.0 build-time binary with pinned SHA256; matching source/notices are linked in the companion README. No runtime Python foundation has been added to the desktop. Recovery tool research and its independent PhotoRec/Linux fixture experiment do not imply integration or Windows acceptance of those external applications.

## Fluent navigation SVGs

Ten unmodified Microsoft Fluent System Icons color SVGs are bundled. MIT copyright/permission notice: `licenses/FLUENT-ICONS-MIT.txt`. Exact upstream revision, paths, Git blob IDs and SHA256 digests are recorded in `assets/icons/PROVENANCE.json` in the source repository. SVG decoding uses Qt SVG (matching Qt 6.8.3), deployed dynamically as Qt6Svg.dll and qsvg.dll and covered by the Qt source/license links above. No emoji or font glyph is used for navigation.
