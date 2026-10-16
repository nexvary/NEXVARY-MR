# Changelog

## 0.3.0 — 2026-10-16 (development)

- Replace the five-page foundation shell with a native bilingual eight-section engineering workspace inspired by the supplied visual reference; no fabricated live device/voltage readings.
- Add repair-shop SQLite persistence, versioned migration, transactional stock consumption, inventory restocking with preserved historic prices, invoice PDF, spreadsheet-safe CSV, backup and validated restore.
- Add original licensed JSON component/net viewer, fictional training data, manual measurements and text/photo annotation sessions.
- Add cancellable exact-identity and SHA256 firmware preflight without flashing.
- Harden read-only external tool processes: selection/authorization, output bounds, cancellation, timeout lifecycle, no-device and failed-start paths.
- Add native executable selection and atomic saved tool paths without requiring PATH edits.
- Add bounded, rule-based log indicators with evidence excerpts; no confirmed-fault claims.
- Fix Windows QuickTest setup order to initialize Qt-dependent backends after QGuiApplication.
- Record minimal read-only diagnostic outcomes locally without automatically archiving raw device logs.
- Pin Windows2022/MSVC2022 and fix Unicode CSV portability.
- Normalize optional empty/null form strings before SQLite binding; regression covers blank optional fields.
- Wait for rendered board controls and opened dialogs in UI interaction tests; preserve actual button clicks.
- Remove unused Qt PrintSupport dependency; invoice PDF rendering uses QtGui.
- Make the device/quick-action sidebar scrollable at minimum window height, verified after Windows runtime screenshot review.
- Correct legacy SQLite connection closure on Windows; keep Python baseline and tests.
- Add Release-effective C++ tests, backend/UI tests and isolated Windows portable/installer validation in CI.
- Record verified upstream revisions and reject incompatible/unclear third-party licenses. Only the existing external scrcpy and Android platform-tools adapters are integrated; no third-party application source is vendored.

## 0.4.0 development

- Preserve the latest source-assessment UI, keyboard navigation and JPEG/EXIF research changes; restore the required os import after Windows research CI exposed a NameError.
- Add native bounded PNG/JPEG raw-image carving, source verification, conditional assessment, DPAPI case containers, explicit consent, evidence provenance/audit and plaintext-confirmed exports.
- Add an optional Kotlin/Shizuku metadata-only Android companion, offline protocol and separate Android CI gates. No privileged device or factory-reset recovery success is claimed.
- Add primary-source recovery-tool/license research and an unexecuted hardware laboratory matrix. Retain the Python baseline and research utilities; the desktop engines remain C++20/Qt.
