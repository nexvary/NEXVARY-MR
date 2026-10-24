# NPSE acceptance roadmap

Phase numbers organize work; they do not certify completeness.

| Phase group | Delivered scope | Remaining acceptance |
|---|---|---|
| 01–10 audit | Four named repos, branches/PRs, existing MR/Guardian Shizuku usage; upstream snapshots/licenses; additional candidates | Deeper adoption audit per additional Android product |
| 11–20 design | Two reusable modules; source/evidence matrix; optional provider; offline transfer; scoped API | Public stable API/versioning review before production publication |
| 21–30 provider | Live state/UID/permission checks, fixed UserService, caller validation, cancellation/death handling | Physical official/Next Binder, revoke/death/OEM tests |
| 31–40 tools | Standard aggregate diagnostics, scoped visible apps/settings, fixed shell metadata, JSON export | Physical probe success; app mutations deliberately unimplemented |
| 41–50 integration | Existing MR Android companion consumes both modules; original helper reuses core | Guardian dependency adoption after a reviewed package; USB/NetScan companion import only after proven benefit |
| 51–60 validation | Core security checks; Android unit/lint/build and emulator workflow matrix | Actual latest CI results; OPPO field run; release signing/distribution gates |

Do not mark 60/60 complete. No production promotion until current exact-source CI,
missing-manager emulator acceptance, both manager variants on physical Android 15
ColorOS, disclosure/license review and security-gate resolution pass. Additional
Android 13/14/16 hardware tests remain pending even if emulators pass.
