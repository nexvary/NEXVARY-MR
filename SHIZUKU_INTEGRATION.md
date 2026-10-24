# Shizuku integration and source audit — 2026-10-24

## Reviewed upstream snapshots

| Project | Reviewed HEAD | Commit date UTC | License file | Latest release observed |
|---|---|---|---|---|
| RikkaApps/Shizuku | b844bc491f1790c72328e1a8e5b2349f8978f0ea | 2025-06-18 | Apache-2.0 | v13.6.0, 2025-05-25 |
| RikkaApps/Shizuku-API | a27f6e4151ba7b39965ca47edb2bf0aeed7102e5 | 2025-05-29 | MIT | Releases endpoint v12.1.0; API source documents 13.1.5 |
| rushiranpise/Shizuku-Next | b20e4c2305a238f87ecf1b983aebd5a00254cd9e | 2026-10-24 | Apache-2.0 | v14.0.29-next, 2026-10-23 |

All three URLs resolved and none was archived at inspection. Release-list tags
are not interchangeable with Maven API versions. NPSE pins Maven API/provider
13.1.5, already used by the MR companion. Full upstream license texts were read;
only the API dependency is linked. Shizuku/Next manager/server code is not copied.
The MIT API notice remains packaged under the companion assets; NPSE also
includes that notice in its assets. Android platform icons use system resources.
NPSE authored modules have a scoped MIT license, not a relicensing of MR or USB.

## What Next adds, and what is unproven

The Next `manager/build.gradle` retains `moe.shizuku.privileged.api` as its
application ID. Its README describes a different signing key; package-name
presence cannot distinguish it from official Shizuku or verify authenticity.
NPSE therefore reports live Binder identity/permission, not a guessed brand.

Source review found boot/start/watchdog code in
`manager/src/main/java/moe/shizuku/manager/receiver/BootCompleteReceiver.kt` and
related services, including method-dependent start-before-unlock decisions.
These are source-level additions, not tested OPPO restart guarantees. NPSE does
not copy methods that alter debugging/security settings. It uses the shared
upstream-compatible Binder contract and requires a live, authorized shell UID.
Both managers still require separate physical interoperability acceptance.

## Host integration

1. Add the NPSE Android dependency or project module. Start
   `Intent(context, com.nexvary.npse.ControlCenterActivity::class.java)` from an
   internal user action. The existing MR agent has this action.
2. Call `ShizukuProvider.disableAutomaticSuiInitialization()` from the host
   Application's `attachBaseContext`, as the MR agent already does. Root is refused
   independently by client state policy and each helper method.
3. Keep the provider protected with `INTERACT_ACROSS_USERS_FULL`; its exported
   flag is required by upstream Binder delivery. The diagnostic Activity is not
   exported and the UserService is not registered as a public Android Service.
4. The AAR ships consumer rules preserving its UserService for R8. Preserve the
   original host metadata helper as well when enabling minification. Current MR
   alpha has minification disabled. Require manager API v13+ for the Context
   constructor/caller-UID check; older service binding may fail closed.
5. Never run a collection just because permission was granted. Per-action
   consent, foreground lifecycle, current permission and result validation apply.

## OPPO / ColorOS acceptance procedure

Use the owner's unlocked Android 15 phone on a trusted network. First install the
agent without any manager: collect standard data and export a local JSON report.
Then start the manager via its normal wireless-debugging pairing flow, authorize
the agent, enable optional diagnostics and confirm one fixed-probe collection.
Record each probe's status, manager version/signing certificate, SDK and OEM build
locally; exclude serial numbers, pairing codes, account data and raw logs.

Repeat: deny permission, revoke permission while foregrounded, stop manager,
restart manager, leave/resume the control center, rotate, cancel while collecting,
reboot, and test ColorOS's normal battery policies. Confirm standard mode survives
every interruption. Do not disable security features to force a pass. Reboot
recovery is pending until observed, and each Android 13/14/16 device requires its
own acceptance. The wizard describes setup; it never pairs or restarts the manager.
