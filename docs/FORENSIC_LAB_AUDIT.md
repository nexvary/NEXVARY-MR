# Forensic Lab — initial repository audit and delivery status

Audit date: 2026-10-17. Branch reviewed: `dev/cpp-qt-foundation`; new development branch: `feature/forensic-lab-foundation`.

## Architecture decision

The native production-oriented desktop is **C++20 / Qt 6 Quick**, with QML UI, CMake/CTest, Windows MSVC CI and QtTest. The older PySide6 prototype on `main` / `feature/oem-service-catalog` is not the native release runtime. The branches are not interchangeable: no blind merge of Python OEM work into C++.

The existing `RecoveryBridge`, `RecoveryImage` and `CaseVault` already implement encrypted local case handling on Windows, bounded authorized file import, source SHA-256, limited PNG/JPEG raw-image carving, audit entries, export and case reopening. These capabilities are reused rather than reimplemented. Their existing tests are the initial integrity regression gate. A case audit list is **not** a complete signed chain-of-custody system.

## First native increment

The new Forensic Lab page is a separate QML module using the existing, working recovery case engine through `recoveryBridge`. Its capability grid distinguishes working bounded features from disabled/unimplemented engines. No external forensic binaries are bundled or executed. The forensic lab page is registered in CMake and navigation.

## External tool candidates (not integrated)

Official upstream projects to evaluate, with licensing and redistribution review **pending**, are:
- Autopsy — https://github.com/sleuthkit/autopsy
- The Sleuth Kit — https://github.com/sleuthkit/sleuthkit
- ALEAPP — https://github.com/abrignoni/ALEAPP
- iLEAPP — https://github.com/abrignoni/iLEAPP
- PhotoRec / TestDisk — https://www.cgsecurity.org/
- Volatility 3 — https://github.com/volatilityfoundation/volatility3
- MVT — https://github.com/mvt-project/mvt
- Plaso — https://github.com/log2timeline/plaso
- bulk_extractor — https://github.com/simsong/bulk_extractor

No assertion of license compatibility, current upstream maintenance, or redistribution permission is made until the relevant source revisions and notices have been inspected.

## Release blockers

1. GitHub Windows Qt build and QML navigation smoke checks for this branch must pass.
2. Separate forensic-case metadata, operator identification and documented custody transfers need schema and tests.
3. An independently verified evidence acquisition/verification workflow is needed beyond the current bounded import.
4. External tool adapters need official source/license checks, sandboxing, cancellation, provenance and integration tests.
5. Android ADB and optional Shizuku permissions must be capability-detected per device; neither bypasses encryption.
6. iOS backup parsing, malware indicators, cross-source timeline, full disk carving and commercial redistribution remain unimplemented.
7. Physical-device tests, clean Windows VM tests, privacy policy and unsigned installer acceptance remain pending.

Do not describe this initial increment as a complete forensic suite or court-accredited product.
