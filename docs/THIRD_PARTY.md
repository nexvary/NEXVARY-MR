# Runtime dependencies and distribution notices

This is a development build, not a signed commercial release. No external Android tool, manufacturer firmware or licensed schematic is bundled.

## Qt 6.8.3

The Windows C++ executable dynamically links Qt 6.8.3. Core, Gui, Qml, Quick, Quick Controls, Quick Layouts, Concurrent, PrintSupport and Sql, together with their deployed runtime dependencies, are distributed as replaceable DLLs/QML modules and plug-ins. The runtime Qt SQLite driver is included. The packaging scripts do not statically link Qt or prevent replacing a compatible Qt build. Reverse engineering required for debugging modifications to LGPL-covered components is not prohibited by this project.

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

`windeployqt` handles the Microsoft Visual C++ runtime dependencies according to the selected Qt toolchain. Microsoft components remain subject to their redistribution terms. Validate the deployed package on a clean supported Windows 10/11 x64 machine before public release; a GitHub-hosted runner is not a clean end-user VM.

## Optional external tools

- [scrcpy](https://github.com/Genymobile/scrcpy): Apache-2.0; detected and started as a separate user-installed application. Its dependency notices apply to its own package.
- [Android SDK platform-tools](https://developer.android.com/tools/releases/platform-tools): Google SDK terms; obtain directly from Google. No binary redistribution is performed here.
- OpenBoardView and TRACE Boardviewer are researched, separately licensed products. No source, executable, icons or file parsers from them are bundled.

See [INTEGRATIONS.md](INTEGRATIONS.md) for pinned upstream revisions and the rejected/ambiguous candidates. Reviewing a repository does not mean it has been integrated.

## Installer and CI

Inno Setup is a build-time tool; the installer contains only the application's deployed runtime. CI builds unsigned portable ZIP and setup EXE artifacts. A SHA256 file checks file integrity; it does not establish publisher identity. Code signing, antivirus reputation, independent Windows acceptance and complete dependency-license review remain release gates.
