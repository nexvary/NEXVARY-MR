# Validation evidence and release boundaries

Validation date: 2026-10-16. Work is on `dev/cpp-qt-foundation` in [draft PR #1](https://github.com/nexvary/NEXVARY-MR/pull/1); `main` remains unchanged.

## Latest candidate

Use the [C++/Qt workflow](https://github.com/nexvary/NEXVARY-MR/actions/workflows/cpp-qt.yml) and [Python workflow](https://github.com/nexvary/NEXVARY-MR/actions/workflows/test.yml) for the results attached to each exact commit. A workflow configuration in this repository is not evidence that its steps have passed. Test XML, deployment stdout/stderr, PNG screenshots and checksums are uploaded as Actions artifacts. Portable/setup artifacts are published only after the Qt job's gates pass.

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
