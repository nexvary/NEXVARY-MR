# USB Doctor acceptance evidence — 2026-10-24

All phone identities are synthetic. PNGs are real Qt application frames rendered
on Linux, not mockups. Existing forensic functionality is regression-tested only.

Connection engine: 42 QtTest results passed. Focused native bridge: 3 passed.
Full production run: first 31 CTest suites passed; GUI smoke initially could not
start because the local build executable was empty/non-executable after linking.
The executable was relinked, verified as a nonzero executable ELF, and the same
source GUI smoke passed on rerun. Both records are retained. No application
source change was made to disguise that environment/build-artifact failure.

Production full UI: 38 passed, zero QML warnings. Separate scale-factor runs at
1.25/1.5 passed. Local Qt was extracted into a temporary prefix because the host
lacked Qt/CMake; explicit Qt plugin/font/QML paths were used. Production DPAPI
and optional external-tool skips are recorded in the individual CTest outputs.
Windows/phone acceptance is not inferred from these Linux/synthetic results.

Laboratory: 31 CTest suites passed in the first run; recovery-image test could
not start because its local executable was empty. Relinked that target only;
its rerun passed, completing the 32-suite set. Laboratory UI 38 passed, zero QML
warnings. Both initial and final outcomes are preserved. Exact-source hosted
Windows/Linux CI must establish a fresh complete run independently.
