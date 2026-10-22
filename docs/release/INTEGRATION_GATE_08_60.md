# NEXVARY-MR — Integration Gate 1 and stages 08–60

Status: IN PROGRESS. This document is a release control plan, not evidence of completion.

## Merge policy
- Integration base: feature/device-service-engine. Never merge the stacked PRs independently without comparing the final combined diff.
- Core forensic, workshop, reporting and read-only diagnostics may be promoted only after exact-head CI and reviewer acceptance.
- All device writes (APK installation, UI mode changes, sideload, erase, reboot) remain EXPERIMENTAL until physical-device acceptance for each operation and model. Destructive operations must not be exposed as commercially supported.
- Acknowledgment from ADB/Fastboot or recovery is not proof of completed action.
- Windows 10/11 clean VM installation, portable execution, uninstall, signing and hardware validation are separate release gates.
- PR #5/#6/#7 are stacked; verify ancestry and re-run checks on the resulting integration SHA before merging.

## Evidence checklist
For every promotion record: commit SHA, exact workflow run URLs, test counts and skips, OS/build, device model/serial redacted, transport, expected vs observed results, screenshots/logs, operator authorization, reviewer and date. No device/customer secrets in artifacts.

## Stage roadmap
08-12: inventory branch ancestry, patches, CI and documentation; isolate risky commands; classify capabilities.
13-18: add regression coverage for serial ambiguity, disconnection, cancellation, timeouts, and command allowlists.
19-24: stabilize read-only battery/storage/partition diagnostics, JSON schema, USB driver evidence and RTL presentation.
25-30: verify workshop SQLite migrations, reports, custody, exports and privacy boundaries.
31-36: validate TLS portal, publication/revocation, key rotation, backups, restore and failure handling.
37-42: controlled Android physical-device read-only acceptance and ADB/Fastboot identity/transport matrix.
43-48: laboratory-only write acceptance by model and firmware, explicit consent, fail-closed and postcondition checks.
49-53: Windows 10/11 clean install/portable/uninstall, Qt rendering, accessibility and security checks.
54-57: release signing, provenance/SBOM, reproducible build evidence, upgrade/rollback and packaging.
58-60: exact-head full regression, reviewer sign-off, release candidate decision; publish only verified capabilities.

## Gate decision
Gate 1 passes only when exact integration HEAD is green, changed files reviewed, all write-capable paths verified disabled by default in the release profile, and known skips explicitly documented. Until then: DO NOT MERGE TO MAIN / DO NOT CLAIM COMMERCIAL OEM SUPPORT.


## Current continuation

See [2026-10-23 checkpoint](DEVELOPMENT_CHECKPOINT_2026-10-23.md) for the actual
implemented native policy, integrated branch stack, test evidence and remaining
stage-specific blockers. The build defaults to read-only device operations.
Stages 37–42 and the final RC decision remain blocked by physical acceptance.
