# NEXVARY-MR ↔ NEXVARY-USB: integration assessment (2026-10-24)

Status: **proposal only; no code imported, no hardware compatibility claimed**.

## Verified starting points
- MR PR #10 (`feature/usb-doctor-intelligence`) already implements a bounded, read-only USB Doctor and Windows inventory with target-scoped diagnosis. Its base is PR #9 (`feature/professional-workstation-ui`), not `main`.
- MR PR #6 contains Windows USB correlation work; PR #5 contains structured diagnostics. Review their overlap before porting anything.
- USB Studio README describes SIM/USIM EF_DIR metadata, AT command capability evidence, bounded CSIM/TPDU transport, opt-in virtual PC/SC loopback, and sanitized JSON/CSV/PNG exports. Some device-specific functions remain unverified on physical hardware.
- USB Studio's root contains a LICENSE and separate licenses/ directory. No redistribution or code-copy decision is authorized without reviewing every applicable notice.

## Recommended architecture
1. **MR owns the Windows workstation shell**, device selection, audit UI, report registry, and user approval workflow.
2. **USB Studio retains its specialized SIM/USIM implementation and independent release lifecycle** until the dependency, licensing and API review passes.
3. Integrate via a **versioned, least-privilege local adapter** that launches an explicitly installed, verified USB Studio executable or uses an agreed read-only library API. Do not embed arbitrary shell commands or untrusted output in QML.
4. The adapter must declare `capabilities`, `tool_version`, `schema_version`, `collection_time_utc`, `device_scope`, `evidence_status`, `limitations` and redacted findings. Treat missing/ambiguous/stale data as unknown.
5. Default to **read-only**. SIM authentication, PIN/PUK, provisioning, write operations, destructive disk operations and network calls are **out of scope**. No implied support for physical PC/SC, AKA, ePDG, IMS or operator calling.
6. Keep USB Doctor's Windows PnP/ADB observations separate from USB Studio's AT/SIM evidence. Correlation must use explicit consent and a verified target; never infer identity from VID/PID alone.
7. Keep forensic evidence immutable and separately permissioned. Do not auto-upload identifiers or raw traces.

## Implementation gates
- [ ] Review active MR PR dependencies (#5, #6, #9, #10) and choose an integration base after their resolution.
- [ ] Inventory USB Studio modules, entrypoints, supported output schemas, runtime dependencies and license obligations.
- [ ] Write a minimal JSON schema with fixtures: valid, missing, malformed, stale, multiple devices, timeout, unsupported capability.
- [ ] Implement an isolated read-only adapter with bounded subprocess lifetime, no secrets in logs, and strict schema validation.
- [ ] Add Windows tests and Linux synthetic regressions; preserve existing Qt/QML suites and Arabic/English RTL.
- [ ] Validate on explicitly authorized physical devices; label simulated evidence separately.
- [ ] Produce real application screenshots, packaging results and rollback documentation.
- [ ] Merge only after CI passes and a maintainer reviews feature parity and licensing.

## Explicit non-goals
No repository deletion, no bulk subtree merge, no duplicate USB Doctor, no unsupported hardware claims, no changes to `main`, and no forced coupling of MR to USB Studio availability.

## Acceptance criteria
MR works unchanged when USB Studio is absent. When present and consented, it can display only supported, timestamped, scoped diagnostic findings with provenance. A failure must be isolated and explainable, with no device mutation or regression in MR's repair/law-enforcement workflows.
