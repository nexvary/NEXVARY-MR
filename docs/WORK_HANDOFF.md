# Work handoff — NEXVARY-MR

Updated 2026-10-16. Branch: `dev/cpp-qt-foundation`. PR #1 remains draft.

## Current execution
- Native C++20/Qt Quick Windows workstation, eight pages, local SQLite, read-only ADB/Fastboot, optional scrcpy, licensed JSON board view and firmware preflight were already implemented.
- Synthetic evidence manifest module added at `research/android-recovery/evidence_manifest.py`.
- Regression tests added at `research/android-recovery/test_evidence_manifest.py`; Windows Python CI now runs this suite.
- QML navigation symbols, accessible button focus states, and panel layout have been modified. Visual acceptance is **pending**; system glyphs are not a licensed SVG icon library.
- Most recent interface commit before this handoff: `170518b4b92f8a930b1f74d06ec82b631e13ac67`.

## Verified outcomes
- Python checks run 38910210284: completed, success, for earlier commit `1d8e144`.
- Qt Windows run 38910210308: cancelled by concurrency; this is **not** a passing result.
- No new Qt build, physical Android acceptance or Windows 10/11 acceptance has been verified for the latest UI changes.

## Outstanding
1. Inspect latest Qt CI for this branch and fix any QML compilation or UI test failures.
2. Replace temporary system symbol navigation with bundled, properly licensed, tested SVG icons after verifying Qt SVG support and redistribution terms.
3. Verify RTL, keyboard focus, small-window scrolling, accessibility, exit confirmation and all page navigation.
4. Complete licensed hardware and OEM acceptance gates, installer, signing, privacy, and legacy migration.
5. Design consent-first Android recovery capability assessment. Shizuku does not grant root or recover cryptographic keys erased by factory reset.
6. Run synthetic evidence experiments with independent input/output directories; preserve original evidence.

## Build verification
```sh
cmake -S . -B build -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python -m unittest discover -s tests -v
python -m unittest discover -s research/android-recovery -p 'test_*.py' -v
```
Do not mark commands as executed unless supported by actual test logs.
