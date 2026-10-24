# NPSE capability matrix

Evidence levels: **source** means reviewed implementation/documentation;
**host test** means simulated/JVM logic; **emulator** means Android runtime test;
**physical pending** means no connected authorized phone was used. Never convert a
source statement into a device-success claim. See TEST_RESULTS.md for current runs.

| Function | Standard Android | Shizuku shell | Authorized ADB | Root |
|---|---|---|---|---|
| Android/model/security patch | Public Build API implemented | Not needed | Existing MR system-property queries; no new NPSE transport | Refused by NPSE |
| Memory totals / battery percent | Public ActivityManager/BatteryManager implemented | Fixed aggregate `/proc/meminfo` probe; device success pending | Shell command depends on OS/SELinux; existing host authorization needed | Refused |
| Visible launcher packages / own version | Scoped PackageManager query implemented; not all installed packages | Broader inventory not implemented | Package-manager shell exists; NPSE does not expose a generic pm command | Refused |
| Open app settings | Public settings intent for a currently visible package | Not needed | Not needed | Refused |
| Grant/revoke permissions; disable/uninstall/clear data | Not implemented; user acts inside Android settings | Deliberately absent from initial alpha | Existing MR operations retain their own gates; no NPSE mutations | Refused |
| Boot/crypto observation | Legacy agent standard metadata remains | Fixed id/getprop allowlist implemented; missing properties remain observations | Existing MR ADB engine; no automatic connection or pairing | Refused |
| Controlled subprocess | App UID only; no elevated rights granted by runner | Typed fixed read-only probes, 1500 ms/4096 bytes each | NPSE transport not configured | Refused |
| USB visibility / permission count | UsbManager counts implemented; device permission still needed for I/O | No general USB access is inferred | ADB authorization does not grant arbitrary USB peripherals | Refused |
| Bugreport / logcat / broad service dumps | Not implemented | Not implemented: unnecessary personal-data exposure | Outside this NPSE alpha | Refused |
| Camera/microphone/screen capture | Existing explicit Android permissions/consent apply | No bypass implemented or promised | No bypass added | Refused |
| JSON export | Explicit local document selection; standard report implemented | Shell observations session-only; cleared on leaving screen | Original MR agent export schema retained separately | Refused |
| Reboot persistence | Standard functions resume normally | No NPSE boot receiver; manager must be checked again | User-authorized connection required | Refused |

## Sources

- [Shizuku API guide](https://github.com/RikkaApps/Shizuku-API/blob/a27f6e4151ba7b39965ca47edb2bf0aeed7102e5/README.md): UID 2000 vs 0, UserService, Android permissions and SELinux limits.
- [Android package visibility](https://developer.android.com/training/package-visibility): app inventory filtering.
- [Android USB host](https://developer.android.com/develop/connectivity/usb/host): enumeration and per-device permission.
- [ADB documentation](https://developer.android.com/tools/adb): explicit USB/wireless authorization.
- [Media projection](https://developer.android.com/media/grow/media-projection): capture consent requirements.

## Runtime evidence added after the source audit

Source 514afe8: Standard mode/missing manager, Arabic and RTL passed on API33–36;
UID-2000 consented collection and real Binder death/fallback passed with official
13.6.0 and Next 14.0.29-next on API35. The live gate individually asserts identity
success; it does not certify every property/memory probe on every OEM. Guardian
host no-manager/PIN entry checks passed on API33–35 as part of 80-case suites.
Physical/OEM and wireless/reboot observations remain pending; Root remains refused.
