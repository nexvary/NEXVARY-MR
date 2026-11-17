# NEXVARY-MR — Agent Instructions

## Project identity

NEXVARY-MR is an **Elite Mobile Technology Lab** — a native C++20/Qt Windows desktop application combining professional mobile repair, device intelligence, forensic evidence management and OSINT search. It is not an Intelligence-only application.

## Ten product pillars

1. **Mobile Repair Studio** — Workshop orders, inventory, invoices, barcode cards, customer records.
2. **Device Intelligence** — USB/ADB/Fastboot discovery, device health, OEM service catalog, diagnostics.
3. **OEM Service Engine** — Chipset-aware service catalog, firmware compatibility rules, model registry.
4. **Firmware Intelligence** — ZIP/AVB/vbmeta analysis, SHA-256 manifest verification, risk reporting.
5. **USB and Hardware Diagnostics** — USB Doctor, driver correlation, connection diagnostics, scrcpy.
6. **Android Service Tools** — ADB/Fastboot service operations, authorized write gating, recovery.
7. **Device Compatibility Engine** — Model/build/chipset compatibility matrix, firmware risk rules.
8. **Advanced Mobile Research** — Competitive benchmarking, security research, open-source evaluation.
9. **Professional Training and Simulation Labs** — Synthetic scenarios, Arabic/English, graded exercises.
10. **NEXVARY Intelligence** — Lampyre MCP integration, unified search, EUR 14.00 pricing, case linkage.

## Architecture

- **Native layer**: C++20, Qt 6.8, QML. `cpp/src/` engines, `qml/` UI.
- **Python layer**: `nexvary_mr/`, `tests/`, `server/intelligence/` — billing, tracking portal, unit tests.
- **CI**: `.github/workflows/cpp-qt.yml` — Windows 2022, Ubuntu 24.04, Qt Quick tests, installer smoke.
- **Active branch**: `feat/intelligence-ux-polish-20261117`, base `e561322` (PR #24 / 0.4.10).

## Development rules

- **Never merge into main** or force-push any branch.
- **No paid external queries** — Lampyre calls require user consent and EUR 14.00 confirmation.
- **No unlicensed competitor code** — benchmark only from public primary sources.
- **No AWS/SSH/GitHub credentials** in committed files.
- **Preserve all ten pillars** — do not reduce the project to a single workspace.
- **Every new feature needs a QML or Python test** before commit.
- **Audit-store discrepancy** (135/139 original cause) is a release blocker — do not close without evidence.
- After each milestone update `docs/DEVELOPMENT_HANDOFF.md` with branch, SHA, CI result and next task.

## Competitor reference

Mobile repair: Chimera Tool, Hydra Tool, UnlockTool, Octoplus Box, EFT Pro, UMT, SigmaKey/Sigma Plus.
Intelligence analysis: Maltego, OSINT Industries.
Lampyre is our backend provider, not a competitor.
Benchmark only from documented public capabilities. Never assume a feature exists without a primary source.
