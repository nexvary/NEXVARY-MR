# Changelog

## Unreleased

- Add the Nexvary Intelligence server billing module with a default €14.00 EUR price, administrator price updates, expiring quotes, prepaid balance reservation, idempotency, refunds and private cost auditing. Production gateway and Windows checkout wiring are pending.
- Label direct-provider Intelligence tools as separately billed and remove supplier billing units from visible Intelligence labels.

- Add a bilingual Intelligence tab beside Security Dashboard, with a dedicated route accessible before account sign-in and case selection.
- Move the existing provider setup and research interface to the Intelligence route. Retain provider, account, case and evidence controls for requests and attachments.
- Clear local demonstration data when leaving Intelligence. Cover navigation, right-to-left order and signed-out access in Qt Quick tests.
- Present tools discovered from the approved MCP connection as selectable cards. A tool change clears prior input and charge consent.
- Clear unsubmitted provider credentials and consents on account changes. Route provider setup to Accounts and permissions from either workspace.
- Improve account field contrast and focus, identify the missing case permission, and point case-grant errors to an administrator.
- Report the count for the scanned Android transport and reject unreadable ADB property responses without retaining stale identity data.
- Use full-window network artwork in the dark theme and show the Intelligence welcome on each entry. Compact the device-services header so the action list and results remain accessible in smaller windows.

## 0.4.1 — 2026-11-11 (evaluation)

- Add guided first-administrator sign-in and investigator account creation without granting administrators implicit case access.
- Show required case fields and actionable errors; retain drafts on failed saves and clear them on account changes. Cover save/edit/reopen in Qt UI tests.
- Start OSINT with empty connection settings; move synthetic data behind an explicit demo switch and clear it on exit.
- Add user-configured MCP Streamable HTTP connection, protected per-user Windows credentials, tool discovery, explicit paid-request consent and bounded JSON/SSE results. Reviewed external-analysis permission covers connection testing and queries; no OAuth or vendor REST endpoint is guessed.
- Export explicitly selected results as new JSON files, marked unverified and not automatically attached as case evidence. No automatic balance or Photon-price estimate.
- Separate confirmed official help links from operational grants, simplify Arabic account/policy/research messages, and hide optional technical details.
- Preserve the approved backgrounds and sculpted icon set; compact service guidance at minimum window sizes.
- Live Lampyre authentication, account entitlements, prices and real provider results remain unverified without account-specific MCP credentials.

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
