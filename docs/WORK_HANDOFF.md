# Work handoff — NEXVARY-MR

Updated: 2026-10-16 (UTC date). Development branch: `dev/cpp-qt-foundation`.
Verified starting HEAD: `d6606f90a0bbb3b801b1336fb29ce79558227c4d` (handoff commit follows).
PR: https://github.com/nexvary/NEXVARY-MR/pull/1 (draft; do not merge without acceptance).

## Verified implementation
Native C++20/Qt Quick Windows desktop, Arabic/English UI, bounded ADB/Fastboot read-only adapters, optional external scrcpy launcher, SQLite workshop, licensed JSON board viewer, firmware preflight, portable and Inno installer pipelines. Python foundation retained. See README and docs/INTEGRATIONS.md for exact limitations.

## Evidence
For starting HEAD `d6606f90`: Windows C++/Qt workflow run 38903214803: completed/success; Python workflow run 38903214819: completed/success. Prior run 38900200998 provides portable, installer and evidence artifacts; these artifacts belong to an earlier revision, NOT necessarily this handoff commit. No local build or physical Android test was performed while preparing this handoff.

## Outstanding and blockers
- Real Android devices, OEM USB drivers, unplug/replug and scrcpy acceptance: requires hardware.
- Clean Windows 10/11, DPI/accessibility and installer acceptance: requires target environments.
- Licensed real board data, privacy/license review and signing: external acceptance.
- Legacy Python data migration: not implemented.
- Android Recovery Agent and Shizuku bridge: not verified implemented; do not claim functionality.
- Post-factory-reset internal encrypted-data recovery: research only, no proven recovery.
- No Windows build has been run against this handoff-only commit.

## Design decisions
Preserve existing native Qt architecture. No silent destructive device operations, FRP/lock bypass or implied root access via Shizuku. Treat shared files, backups and encrypted internal storage as distinct recovery sources. Obtain device-owner consent, hash acquisitions, preserve originals, and test with synthetic fixtures. Review each third-party license before redistribution.

## Next execution priority
1. Inspect current tree, QML UI tests, CI and package manifests for gaps; avoid duplicating existing features.
2. Run native Windows pipeline on updated HEAD and inspect job/artifact results.
3. Add independent, testable recovery assessment core with explicit capabilities and no unsupported success percentages; only then design optional Kotlin/Shizuku agent.
4. Research upstream tool versions/licenses and create reproducible synthetic recovery experiments.
5. Update this handoff with actual test outcomes and commit identifiers.

## Build and test
```sh
cmake -S . -B build -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python -m unittest discover -s tests -v
```
These commands are documented, not executed by this handoff commit.
