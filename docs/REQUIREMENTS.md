# NEXVARY MR verification contract

C++20 core independent of Qt; Qt desktop bridges independent of QML; native SQLite local persistence; QML UI. Avionics Lab `CMakeLists.txt` and README were inspected via connected GitHub: adopted C++20/CMake/CTest, optional GUI target, compiler warnings, sanitizer option and explicit bridge/UI separation. No private Avionics source was copied.

## Automated gates

1. Core-only C++ Debug/Release checks. Tests must remain active under NDEBUG.
2. Qt desktop build on Windows MSVC/Qt6.8.3.
3. QtTest: transactions/stock/invoice/backup restoration, malformed input, schema versions, permissions, board sessions, firmware identity/hash/cancel, fixed read-only process arguments, no ADB/unauthorized/disconnect, failed start, timeout and stale timeout regression.
4. Qt Quick: actual navigation clicks, locale toggle, diagnosis action, exit cancel/accept and board interactions.
5. Executable smoke: load compiled resources, navigate all eight pages in Arabic and English, capture a real runtime PNG, clean close, fail on QML warnings.
6. Copy portable to an isolated path with spaces, remove Qt SDK/plugin/import PATHs and run the same smoke command.
7. Build Inno per-user installer, install, smoke installed runtime, uninstall; publish unsigned artifacts and checksums only on passing gates.
8. Retain and pass Python regression suite.

## Human/device acceptance still required

- Clean Windows 10/11 x64 VM; actual display scaling 100/125/150/200%, keyboard focus and screen-reader accessibility.
- Actual authorized Android devices, SDK/OEM drivers, USB disconnect/reconnect, scrcpy external process interaction. Fake-process tests do not prove physical compatibility.
- Real licensed board data converted to native JSON with independently checked component/net definitions. No .brd/PDF proprietary parser is implemented.
- Measurements verified against model-specific authorized source; no electrical-instrument adapter. Local PNG/JPEG component photos are supported, with bounded decoded size and metadata stripping.
- OEM firmware authenticity/signature checks, complete backup and recovery workflows. Current firmware gate only checks identity metadata and checksum, with no flashing.
- Customer privacy/retention/access control acceptance, tested backup recovery, complete dependency/licensing audit, installer publisher signing.
- Legacy database migration/parity acceptance before retiring Python baseline.

No final-release claim, main merge or device-destructive workflow is authorized merely by passing automated tests.
