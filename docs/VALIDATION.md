# Validation evidence and release boundaries

Validation date: 2026-10-16. Work is on `dev/cpp-qt-foundation` in [draft PR #1](https://github.com/nexvary/NEXVARY-MR/pull/1); `main` remains unchanged.

## Latest candidate

Use the [C++/Qt workflow](https://github.com/nexvary/NEXVARY-MR/actions/workflows/cpp-qt.yml) and [Python workflow](https://github.com/nexvary/NEXVARY-MR/actions/workflows/test.yml) for the results attached to each exact commit. A workflow configuration in this repository is not evidence that its steps have passed. Test XML, deployment stdout/stderr, PNG screenshots and checksums are uploaded as Actions artifacts. Portable/setup artifacts are published only after the Qt job's gates pass.

## Verified Windows candidate

The subsequent sidebar revision `39175ba2c235ccf2cac6ac61891c8a27a06b05cb` passed [run 38900200998](https://github.com/nexvary/NEXVARY-MR/actions/runs/38900200998): all seven native suites, portable isolated runtime, and installer install/run/uninstall passed. Python checks also passed. Actual Windows screenshots and test files were downloaded and reviewed. [Portable](https://github.com/nexvary/NEXVARY-MR/actions/runs/38900200998/artifacts/12045304923), [installer](https://github.com/nexvary/NEXVARY-MR/actions/runs/38900200998/artifacts/12045479161), [evidence](https://github.com/nexvary/NEXVARY-MR/actions/runs/38900200998/artifacts/12045817545).

## Base completion increment

The next increment expands the directory to 20 brand groups/65 family entries and 22 distinct guided workflows, without treating catalog entries as firmware/driver/board compatibility certification. SQLite schema v2 persists technician findings, manually reviewed evidence and the exact rule-guidance snapshot against an explicitly chosen repair order. JSON reports and invoice PDFs include these entries. Existing schema-v1 databases and backups migrate through tested paths; the unrelated Python `cases` database is retained separately.

Local checks executed after this increment: core/workshop/tool/board/firmware/GUI smoke passed; the first UI launch encountered a zero-byte locally linked executable, which was relinked, and UI then passed including the real save-findings click. Workshop has 14 QtTest results including setup/cleanup; UI has 11 including setup/cleanup. Legacy Python remains 4/4. Windows CI for the increment must pass before claiming Windows validation for it.

Remaining base acceptance gates require external resources: actual authorized Android/OEM-driver/scrcpy interaction, clean Windows10/11 consumer VMs/DPI/accessibility, and independently verified licensed model-specific board data. No physical phone, instrument or licensed phone schematic is present in this execution environment. Native JSON board editing/viewing and simulated tool failures are tested; proprietary BRD/PDF parsing, flashing and instrument acquisition are unsupported features, not passing tests. These are explicit boundaries for continuing an isolated recovery research increment; they do not authorize a commercial release or merge.

Commit `c648f1483b8feb698f30adf3396bf2b302e8c91a`, [successful run 38899256690](https://github.com/nexvary/NEXVARY-MR/actions/runs/38899256690):

- Windows2022/MSVC2022/Qt6.8.3 build with compiler warnings as errors: passed.
- Native CTest: 7/7 passed, including 10/10 QuickTest results (init/cleanup included); legacy Python: 4/4 passed.
- Portable copied outside the developer directory with SDK paths removed: GUI smoke exit0, JPEG/PNG decode and SQLite ready, screenshot produced, empty stderr.
- Inno installer built, installed, deployed runtime smoke-tested, then uninstalled: passed; empty runtime stderr.
- Downloaded archives were inspected. Both package SHA256 values matched the recorded checksums, all 1,379 portable manifest entries matched, required Qt/SQLite/JPEG/CRT files were present, and no test EXEs were bundled.

Artifacts: [portable](https://github.com/nexvary/NEXVARY-MR/actions/runs/38899256690/artifacts/12045372788), [installer](https://github.com/nexvary/NEXVARY-MR/actions/runs/38899256690/artifacts/12045492344), [Windows runtime screenshots/test evidence](https://github.com/nexvary/NEXVARY-MR/actions/runs/38899256690/artifacts/12045820881).

Visual inspection of the actual Windows minimum-size screenshot identified clipped sidebar quick actions. The subsequent QML adjustment makes that sidebar scrollable; its UI regression scrolls to and verifies the quick backup action remains reachable. The latest workflow run is authoritative for the adjusted revision.

## Executed local checks

Ubuntu 24.04, GCC 13, Qt 6.4.2, C++20, `MR_WARNINGS_AS_ERRORS=ON`:

- Complete native CTest run: 7/7 suites passed after board photos, stock restocking, diagnostic audit and strict restoration tests were added.
- Tool suite expanded to saved executable selection: 13 QtTest checks passed, including configuration persistence/security, a controlled executable child, no tool/unauthorized/disconnected states, failed start, actual 12-second timeout and stale-timer regression. No phone is contacted by these fixtures.
- Core includes bounded text-pattern log analysis with evidence counts, ordinary-log negative case and oversized-input rejection. These indicators are not confirmed diagnoses.
- Qt Quick includes clicks on all eight navigation buttons, language switching, repair-order creation through Save, manual board measurement, minimum window size/scrolling, diagnosis action, exit cancellation and exit confirmation.
- Compiled-resource GUI smoke loads original fictional board data, PNG and JPEG assets, traverses all eight pages in both languages, saves an actual runtime screenshot and closes. It fails on QML warnings.
- Legacy Python regression suite: 4/4 passed. The SQLite connection-lifetime leak was corrected without deleting the Python baseline.

## Windows evidence from earlier revisions

- Baseline `fc74d5e`: Windows C++/Qt compilation and portable assembly passed, but Python failed on an unclosed SQLite file. No UI-run proof was available in that baseline.
- `5afea10`: native build exposed MSVC's rejection of a CSV Unicode escape. Corrected with an explicit QChar BOM.
- `6236e18`, [run 38897028279](https://github.com/nexvary/NEXVARY-MR/actions/runs/38897028279): Windows 2022 / MSVC2022 / Qt6.8.3 compiled successfully; core/workshop/tool/board/firmware suites and the executable runtime smoke passed. QuickTest itself crashed before its first case, so the workflow correctly refused packaging. Setup initialization has since been moved to `applicationAvailable()`; subsequent Windows results, not this change alone, determine whether that fix passes.

- `e0de0d9`, [run 38898529718](https://github.com/nexvary/NEXVARY-MR/actions/runs/38898529718): detailed output identified the remaining UI failure as the manual measurement click (9/10 QuickTest results passed). UI tests now wait for rendering/viewport visibility while preserving real clicks. Packaging also exposed a redundant PrintSupport DLL assertion; the application uses QtGui QPdfWriter and the unused dependency was removed.
- A separate null-QString regression reproduced failure when optional repair fields were empty/null. Bindings now normalize optional strings to non-null empty text; the regression passed after the fix.

## Deployment checks

The current packaging gates explicitly require the SQL SQLite driver, Windows platform plug-in and JPEG decoder. The executable smoke decodes a tiny original JPEG fixture, so a missing qjpeg plug-in cannot pass unnoticed. App-local release MSVC CRT files come only from VS2022's official redistributable folder; their provenance, versions and hashes are recorded. No debug or System32 DLLs are copied.

The portable is copied to an isolated path with spaces; Qt SDK/PATH/import overrides are removed. An Inno per-user installer is then built, installed, smoke-tested and uninstalled. These are GitHub-hosted Windows runner checks, not clean consumer Windows VM acceptance.

## Unverified or unsupported

- No actual Android/USB/OEM-driver/scrcpy control test has been performed in this environment.
- No actual electrical measurement instrument is connected. Measurements are manual; the bundled board and voltage references are explicitly fictional.
- The native licensed JSON viewer does not parse proprietary .brd formats or circuit PDF schematics. Real model-specific board data requires licensed source material and independent verification.
- Firmware identity/checksum matching does not prove authenticity or authorize installation. Flashing/recovery/unlocking/erasure are not implemented.
- No iOS interface, ERPNext API connector, embedded external boardviewer, cloud AI diagnosis or legacy-database importer is claimed.
- Clean Windows10/11 x64 VM, DPI/accessibility/user acceptance, physical phones, publisher signing, final dependency/license/privacy review and backup retention remain commercial-release gates.

Passing automated tests does not remove these limits. This is an unsigned development build and must not be advertised as a finished commercial product.

## Recovery increment evidence

`9e315088005751dbd7690d118de6040cf84a6d5f` passed [Windows/Qt38945576237](https://github.com/nexvary/NEXVARY-MR/actions/runs/38945576237) and [Python38945576240](https://github.com/nexvary/NEXVARY-MR/actions/runs/38945576240). Windows CTest9/9, UI13/13 including lifecycle, portable isolated smoke and installer install/run/uninstall passed. Both runtime stderr files were empty. Actual Windows recovery screenshots show two original synthetic image candidates and their offsets/hashes, not a phone recovery.

Downloaded portable/setup SHA256 matched recorded sums; all1,380 portable manifest entries matched, required Qt/JPEG/SQLite/CRT files were present, and test executables were absent. APK906,642 bytes had matching SHA2561660afa3fcc0fc2a26116c93506c17f6c07a5aa143546d2c78a3bde8d76b1433. Android8/8 JVM tests, lint with0errors/6warnings, permission boundary, and API30 emulator consent/public-metadata/shell-gate UI test passed in [Android38945576262](https://github.com/nexvary/NEXVARY-MR/actions/runs/38945576262). The APK requests only the Shizuku API permission; complete MIT notice and executable code were checked inside it.

The subsequent Android-only privacy increment adds explicit cloud/device-transfer exclusions and a density-independent launcher bitmap, plus a second instrumented policy check. Its exact revision must independently pass before inheriting any APK result. Lint upgrade/target-level warnings are not suppressed. Linux recovery cases/image tests passed ASan/UBSan2/2 with leak detection disabled because this environment cannot support LeakSanitizer's process inspection.

No physical Shizuku backend, phone USB/OEM/scrcpy session, customer media or factory-reset experiment was used. Every hardware matrix row remainsnot_run. PhotoRec7.2 was independently executed on Linux against two original synthetic image fixtures with matching hashes and unchanged source; it is not an integrated desktop worker.
