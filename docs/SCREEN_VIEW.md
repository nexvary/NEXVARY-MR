# Consented phone screen view and stable device refresh

Screen view is available in both production and Laboratory through a dedicated
native consented-component policy. The generic device-service write allowlist,
Laboratory boundary, unknown-command denial and protected-device restrictions
are unchanged. The historical machine-readable build-profile identifier remains
`production-read-only` for packaging compatibility; it describes the generic service
profile, not a claim that an explicitly approved scrcpy session is passive.

## Using screen view

1. Connect the phone with a data cable. Unlock it normally when needed and approve
   USB debugging for this trusted computer.
2. Discover ADB, select exactly one authorized device and read its information.
3. Choose **Review screen view**, review the temporary component notice, confirm
   owner authorization/component execution, and enter the selected serial exactly.
4. A separate **NEXVARY MR Screen View** window opens through scrcpy 5.0.1. This
   implementation does not embed video inside the MR card. Read its session status
   under **Screen session**; missing tools, version/target checks and encoder errors
   are surfaced there. Host process startup does not prove delivery of video frames.
5. Stop through MR or close the display window. Disconnect, changed identity,
   ambiguous target, timeout and excess output stop the owned processes. No automatic
   target fallback, host detachment or reconnection to a different device occurs.

The fixed command disables control, audio, clipboard synchronization and automatic
power-on. It uploads/runs scrcpy's temporary component and may expose private screen
content. This is an active, consented operation, not a cryptographic bypass or passive
forensic acquisition. Stopping the host is not proof of on-phone cleanup. The exact
owned phone/firmware, including the reported OPPO CPH2577, still needs field acceptance.

Windows evaluation packages now include the unchanged official win64 5.0.1 runtime
under `tools/scrcpy`, prepared only during building with a pinned archive SHA-256.
No runtime download or silent tool launch is added. Explicit configured paths take
priority; otherwise bundled ADB/scrcpy are resolved before PATH. Invalid configured
paths fail closed; reset the tool to Auto in Settings to use the bundle. Portable
and installed isolation checks verify actual resolution, every retained file digest
and the official executable's version without addressing a phone.

## Blue activity/flicker repair

Previous automatic discovery used the foreground runner, toggling global busy
badges and disabling blue buttons every five seconds. Every discovery also cleared
USB correlation/capability state, including unchanged lists; periodic notifications
recreated identity delegates. Multiple device cards displayed another operation's
spinner. These code paths are now covered by native/UI regression checks.

Auto discovery uses a separate bounded, deadline-limited reader. Successful unchanged
polls retain selection, device properties, capability/driver state and foreground
status. A foreground command supersedes a pending poll; late output cannot overwrite
its state. Poll failure invalidates stale discovery and is visible, never hidden.
Identity notifications/field delegates are reused when their values do not change;
freshness expiry still updates their state. Each device card shows its own operation's
spinner. A running screen window does not keep the global progress strip busy;
preflight/cancellation still do, and Stop remains available.

A still photo cannot establish the exact physical flicker frequency or rule out a
particular Windows graphics driver problem. Synthetic regression proves these known
software causes are fixed; user-machine graphics/frame delivery remains a field test.

## Runtime provenance and third-party notices

Official release and source: https://github.com/Genymobile/scrcpy/releases/tag/v5.0.1
Pinned source commit: a60891aea193d92e7e5c3942700eca63f9d19a5f
Pinned win64 archive: b12a2c4ee8be317422451fc7dcf8ee20a71b5ea7ef9ad73ddd825a25316227a5
The exact upstream archive LICENSE.txt is retained unchanged; preparation adds a
SOURCE_PROVENANCE.json inventory. A pinned hash establishes reviewed bytes, not
Authenticode or a complete transitive licensing/security assessment.

The pinned upstream dependency recipes identify FFmpeg 9.0.2, SDL 3.4.18,
libusb 1.0.30, dav1d 1.5.4 and Android platform tools 37.0.1:
https://github.com/Genymobile/scrcpy/tree/a60891aea193d92e7e5c3942700eca63f9d19a5f/app/deps
The shipped avcodec DLL identifies its license as LGPL 2.1 or later and its build
configuration disables GPL/network features. Sources/build recipes and applicable
license/source-distribution duties must be independently reviewed before final
commercial/institutional release. Included tools remain separate upstream programs,
not original NEXVARY implementations. See INTEGRATIONS.md and the package's SPDX
inventory; this evaluation package is not a completed licensing/accreditation gate.
