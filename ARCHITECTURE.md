# NPSE architecture — 0.1.0-alpha01

Scope: Android privileged diagnostics inside the existing `recovery-agent` build.
This document describes NPSE; existing desktop architecture remains under `docs/`.
No new repository or second Android companion was created.

## Modules and reuse

- `recovery-agent/npse-core`: Java library containing the fixed `Probe` enum,
  `AccessPolicy`, bounded subprocess execution, cancellation and dependency-free
  executable security tests. No Android or Shizuku dependency.
- `recovery-agent/npse-android`: Android AAR containing standard diagnostics,
  visible launcher-app settings, optional Shizuku access, a fixed AIDL diagnostic
  helper, validated observations, AR/EN resources and the non-exported control center.
- `recovery-agent/app`: existing MR Android companion, serving as the working
  sample/first consumer. Its original metadata service delegates policy/execution
  to NPSE; its original `nexvary-recovery-agent/v1` export schema is retained.

The AAR has an API dependency on `npse-core` and implementation dependencies on
Shizuku API/provider 13.1.5. The manager APK is never bundled. An app can use the
core alone without Shizuku dependencies; an app using the AAR works without an
installed or running manager. Shizuku is an optional runtime facility.

## Data flow and boundaries

User opens MR agent → internal control center → StandardProvider by default.
Explicit opt-in → Shizuku permission → per-operation consent → non-daemon
UserService running as UID 2000 → fixed probes → bounded result validation.
Service availability means permission to attempt probes, not proof they work.

No host/client socket, TCP listener, automatic pairing or new ADB implementation
exists in NPSE. Desktop MR's existing ADB/operation engines remain authoritative.
The existing user-selected JSON file transfer is the initial offline channel;
NPSE diagnostics use their own schema and are not silently imported as recovery
reports. The original MR agent report remains the supported desktop route.

## Publication

From `recovery-agent`, JDK 17 and Android SDK 35:

```sh
./gradlew :npse-core:check :npse-android:assembleRelease
./gradlew :npse-core:publishToMavenLocal :npse-android:publishToMavenLocal
```

Maven coordinates: `com.nexvary.npse:npse-core:0.1.0-alpha01` and
`com.nexvary.npse:npse-android:0.1.0-alpha01`. A local Maven publication retains
transitive dependencies; copying the AAR alone is insufficient. Other authorized
Android builds can resolve the local publication using `mavenLocal()` for a
reviewed checkout, then `implementation("com.nexvary.npse:npse-android:0.1.0-alpha01")`.
No GitHub Packages upload or public Maven release is claimed. Pin source commits
and checksum reviewed artifacts before promoting a shared production dependency.
See `SHIZUKU_INTEGRATION.md` for the required host setup.
