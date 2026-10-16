# NEXVARY MR — Requirements and verification gates

## Architecture decision
C++20 + Qt 6 Quick/QML + CMake + CTest, aligned with NEXVARY Avionics Lab.
Python v0.1 remains available until feature parity is verified. No destructive flashing actions in foundation.

## Current implementation
- C++ core: device family catalog, guided diagnostic recommendations, repair-case validation.
- QML: RTL-aligned navigation shell for five sections, explicitly marked as foundation where integration is pending.
- Windows CI: separate core and Qt build/test jobs.
- Existing Python foundation retained unchanged.

## Release gates
- [ ] C++ Windows core CTest green
- [ ] Qt Windows build green
- [ ] UI screenshots from real Windows runtime
- [ ] RTL and keyboard navigation checked
- [ ] ADB/Fastboot read-only discovery integrated
- [ ] SQLite repair case persistence integrated
- [ ] Real Windows 10 device tests
- [ ] Portable package and installer signed or documented unsigned
- [ ] License and third-party dependency audit

## Safety
Never run flashing, unlocking, FRP bypass, identity modification or data erasure automatically.
