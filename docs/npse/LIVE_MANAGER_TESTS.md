# Live Shizuku manager test scope

A separate, opt-in Actions workflow tests pinned official Shizuku 13.6.0 and
Shizuku Next 14.0.29-next on independent disposable Android 15 emulators.
It refuses a physical device or root shell. Managers are downloaded from their
respective public release assets and SHA-256 pinned; apksigner verifies their
APK signatures and records the signer certificate. No manager APK is bundled
with NPSE or installed on a user's phone by this test.

The starter path is the exact nativeLibraryDir/libshizuku.so ADB command used
by both source-reviewed Starter.kt implementations. The tested client obtains
permission only by clicking its explicit manager UI dialog, then confirms the
NPSE per-operation dialog. It checks a live UID-2000 identity observation,
terminates only that emulator's shizuku_server, observes actual Binder death
and result invalidation, and checks Standard diagnostics still work.

ShizukuShellIntegrationTest is skipped in the normal no-manager matrix unless
npseManagerIntegration=true is explicitly supplied. A skipped opt-in test does
not prove live-manager compatibility. Record real workflow results separately.
Manager permission revocation, reboot/wireless pairing and physical OPPO/ColorOS
remain separate acceptance items. This test does not claim those pass.

Artifact pins: official release APK SHA-256
6e273ab0e991c4e79bc8b1bbb9b9dd739ccac1a8712a541a214078886b7b790f
(observed from the owner-hosted v13.6.0 release; release metadata lacked digest);
Next release APK SHA-256
e46623397e76e7f0292ba5fd435347cda123ead6d7e32869333c64d2dac30079
(matches GitHub's release asset digest). A checksum and verified signature do
not constitute an independent security audit of either manager.
