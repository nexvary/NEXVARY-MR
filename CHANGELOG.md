# Changelog

## 0.3.0 — 2026-10-16 (development)

- Replace the five-page foundation shell with a native bilingual eight-section engineering workspace inspired by the supplied visual reference; no fabricated live device/voltage readings.
- Add repair-shop SQLite persistence, versioned migration, transactional stock consumption, inventory restocking with preserved historic prices, invoice PDF, spreadsheet-safe CSV, backup and validated restore.
- Add original licensed JSON component/net viewer, fictional training data, manual measurements and text/photo annotation sessions.
- Add cancellable exact-identity and SHA256 firmware preflight without flashing.
- Harden read-only external tool processes: selection/authorization, output bounds, cancellation, timeout lifecycle, no-device and failed-start paths.
- Record minimal read-only diagnostic outcomes locally without automatically archiving raw device logs.
- Pin Windows2022/MSVC2022 and fix Unicode CSV portability.
- Correct legacy SQLite connection closure on Windows; keep Python baseline and tests.
- Add Release-effective C++ tests, backend/UI tests and isolated Windows portable/installer validation in CI.
- Record verified upstream revisions and reject incompatible/unclear third-party licenses. Only the existing external scrcpy and Android platform-tools adapters are integrated; no third-party application source is vendored.
