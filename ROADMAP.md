# NPSE acceptance roadmap

Phase numbers organize work; they do not certify completeness.

| Phase group | Delivered scope | Remaining acceptance |
|---|---|---|
| 01–10 audit | Four named repos, branches/PRs, existing MR/Guardian Shizuku usage; upstream snapshots/licenses; additional candidates | Deeper adoption audit per additional Android product |
| 11–20 design | Two reusable modules; source/evidence matrix; optional provider; offline transfer; scoped API | Public stable API/versioning review before production publication |
| 21–30 provider | Live state/UID/permission checks, fixed UserService, caller validation, cancellation/death handling | Physical official/Next Binder, revoke/death/OEM tests |
| 31–40 tools | Standard aggregate diagnostics, scoped visible apps/settings, fixed shell metadata, JSON export | Physical probe success; app mutations deliberately unimplemented |
| 41–50 integration | Existing MR Android companion consumes both modules; original helper reuses core | Guardian adoption completed/tested; USB/NetScan desktop adapters not applicable in this alpha |
| 51–60 validation | Core security checks; Android unit/lint/build and emulator workflow matrix | Emulator/CI gates passed; OPPO field run and production signing gates pending |

Do not mark 60/60 complete. No production promotion until current exact-source CI,
missing-manager emulator acceptance, both manager variants on physical Android 15
ColorOS, disclosure/license review and security-gate resolution pass. Additional
Android 13/14/16 hardware tests remain pending even if emulators pass.

## Validated completion slice

MR no-manager API33–36, official/Next real Binder API35 and Guardian API33–35
(80 UI cases each) passed. See docs/npse/COMPLETION_EVIDENCE.md. Reusable artifact
consumption, actual screenshots and test APK delivery are complete for this
alpha; physical/OEM and final release gates remain pending. Phase 60 is not
certified by emulator success.
