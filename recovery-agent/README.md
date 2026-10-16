# NEXVARY Recovery Agent 0.1.0 — experimental metadata companion

Kotlin Android26+ with optional official Shizuku API/provider13.1.5. Public SDK metadata works without Shizuku. The app never installs or starts Shizuku, requests root, unlocks a bootloader, resets a phone, copies private app data, accesses block devices, or restores erased keys.

**Assess installation/write impact first.** Use existing backups/offline images before deciding to install this agent. Its presence is not a prerequisite for the Windows image-analysis workflow.

The native Arabic/English interface requires explicit owner authorization before collecting manufacturer/model/OS, aggregate volume descriptions or hashing a locally selected SAF document. File contents are not put into the report. Optional Shizuku permission is requested visibly; the AIDL UserService accepts shellUID2000 only, exposes fixed probes with bounded timeout/output, and refuses root/unknown identity. Revocation/death clears cached privileged observations. No INTERNET or broad storage permissions are requested; no arbitrary command/path IPC exists.

## Offline protocol

Owner selects a local destination using Android's document picker. Transfer the JSON using an authorized local file-copy method and import it into the Windows Data recovery workspace. No server, authentication token, port or network transport is deployed. JSON `schema` is `nexvary-recovery-agent/v1`, capped64KiB, with `device`, `consent`, `shizuku`, `storage`, `probes`, optional `selectedFile` name/size/SHA256 and bounded `audit`. Consent scope is `device-metadata`.

Valid state/UID pairs are `shell`, `permission-required`, `denied` with2000; `unavailable`, `unsupported-root` withnull. A report is unsigned and does not authenticate device origin or prove ownership. The desktop preserves its complete original bytes and SHA256 inside the encrypted case and labels identity unverified. Metadata collection and file hashing are not deleted-file recovery.

## Build and test

JDK17, Gradle8.9 (wrapper/distribution checksums pinned), AGP8.7.3, Kotlin2.0.21, compile/target35. Run `./gradlew assembleDebug testDebugUnitTest lintDebug`; an AndroidSDK is required. CI additionally runs the native consent/public-metadata/shell-gate UI test on an API30 emulator and captures a real emulator screenshot. Build tasks, policy unit tests, emulated UI and physical Shizuku acceptance are separate evidence levels. See Actions for the result of each exact revision; the existence of this workflow does not prove success.

A debug APK is an unsigned-for-production lab artifact, not a signed customer release. Real Shizuku binding/permission/revocation, OEM behavior, SAF documents, Android14/15/16 and phone USB transfer still require dedicated physical-device trials. No factory-reset trial has been performed by this implementation.

## Dependencies

Shizuku API/provider13.1.5 are MIT licensed (RikkaApps); matching source and notices: https://github.com/RikkaApps/Shizuku-API. Shizuku manager is a separate user-installed project with its own license: https://github.com/RikkaApps/Shizuku. Gradle wrapper is Apache2.0; version8.9 sources/notices: https://github.com/gradle/gradle/tree/v8.9.0. Android SDK/build components are obtained under Google's applicable SDK terms. Test-only dependencies include JUnit, AndroidXTest and JSON-java; they do not replace the C++ commercial desktop core.
