# NPSE alpha acceptance — verified 2026-10-24

## Exact sources and runs

| Gate | Source / run | Actual result |
|---|---|---|
| Engine no-manager Android matrix | 514afe8684504ce681b74d4722d5c275cc5ca9c5 / 40126853530 | API33/34/35/36 each 4 passing runtime tests, 0 failures, 1 intentional opt-in skip |
| Real manager integration | Same source / 40126853544 | Official 13.6.0 and Next 14.0.29-next, each 1 passing real Binder test on API35 |
| Guardian host | 7f324204e2f60ab1aa42136668856676157e45b6 / 40126176694 | Build/lint/unit/signature/backend and API33/34/35 each 80 passing UI tests; no skips/failures |
| Engine JVM | Existing full core gate | 47 assertions; 12 app/library JUnit tests; lint/build passed |
| Prior native regression baseline | 0908a3363401b2ccba4292fa82a27062ab6c908f / 40113664271 | Windows/Qt/Python/public-history passed; completion changes add no desktop C++ source |

The raw final no-manager XML reports contain 5 cases, 1 skipped: the skipped
ShizukuShellIntegrationTest requires npseManagerIntegration=true and ran in the
separate manager workflow. It is not counted as a no-manager pass. Both manager
artifacts contain `OK (1 test)`; their screenshots and signer reports were read.
Guardian XML reports were read and contain both NpseIntegrationTest cases.
Artifact ZIP SHA-256 values were verified before local extraction/inspection.

The live test checks permission through the manager's actual UI, a second
operation-consent dialog, validated UID-2000 identity, killing only the test
emulator's shizuku_server, observing Binder death, clearing privileged results
and collecting Standard diagnostics again. Four typed observations are returned;
only identity success is asserted individually by this gate. Other per-operation
statuses must be read from the actual report, not assumed successful globally.

The shared binary Guardian consumes is unchanged from the reviewed 0908a33
library build. Later engine changes add tests/workflow configuration only.
The exclusively scoped file Maven repository avoids a second engine source copy
and expiring authenticated dependency URLs.

## Remaining external acceptance

No Desktop Commander device is connected. Physical OPPO/ColorOS, wireless
pairing, permission revocation on that device, reboot/restart and OEM battery
behavior remain PENDING. Live-manager tests above are on emulators using an
explicitly authorized host ADB startup, not wireless-pairing or persistence tests.
Guardian real-manager behavior on the owner's phone also remains pending.
No root or production release is claimed; supplied APKs are debug previews.

USB/NetScan integration study is closed for this alpha: they are currently
Windows products without Android modules, and no benefit justifies creating
another Android companion or linking Binder to desktop code. Their future
Android/OTG/network work needs new demonstrated scope, not an unsupported
Shizuku adapter. Both actual Android consumers are MR and Guardian.

PRs: MR #14; Guardian #13. Main is unchanged in both repositories. The phase
numbers are not a 60/60 certification. Physical acceptance still blocks a final
production-compatibility claim. See ACCEPTANCE_STATUS.json for machine-readable
scope and TEST_RESULTS.md for the retained failure/retry history.
