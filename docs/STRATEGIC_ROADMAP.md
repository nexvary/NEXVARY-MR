# NEXVARY-MR Strategic Roadmap

**Identity:** Elite Mobile Technology Lab — C++20/Qt Windows desktop.
All ten product pillars must be preserved. This is not an Intelligence-only product.

---

## Product pillars and status

| # | Pillar | Core engines | Status |
|---|--------|-------------|--------|
| 1 | Mobile Repair Studio | WorkshopBridge, RepairBridge, Code39Label | Implemented — CRUD, inventory, invoices, barcodes |
| 2 | Device Intelligence | DeviceIntelligence, DeviceProtocol, ToolBridge | Implemented — USB/ADB/Fastboot discovery, health |
| 3 | OEM Service Engine | ServiceCatalog, ServiceCommandPolicy, DeviceCompatibility | Partial — 14 chipset groups, 44 model refs, write-gating |
| 4 | Firmware Intelligence | FirmwareBridge, AvbMetadata, PackagePreflight | Implemented — ZIP/AVB/SHA-256 analysis |
| 5 | USB and Hardware Diagnostics | UsbDriver, ConnectionDiagnosis, MirrorSession | Implemented — USB Doctor, scrcpy integration |
| 6 | Android Service Tools | DeviceProtocol, ServiceOperationPolicy | Partial — ADB/Fastboot ops, authorized write gating |
| 7 | Device Compatibility Engine | DeviceCompatibility, DeviceAssessment | Partial — identity/build matrix; field corpus pending |
| 8 | Advanced Mobile Research | ResearchBridge, research/ catalog | Partial — architecture atlas, experiments; promotion pending |
| 9 | Professional Training and Simulation Labs | LawTraining, SyntheticPolicyExercise | Partial — synthetic scenarios, AR/EN; curriculum pending |
| 10 | NEXVARY Intelligence | LampyreConnection, LampyreEvaluationBridge, PhoneOsint | Implemented — EUR 14.00 price, consent, MCP tools |

---

## Mobile repair competitive benchmarks

Sources: official vendor pages and public documentation only. No competitor executable was run. No parity or superiority is established without a controlled comparison dataset.

| Competitor | Public capability (source) | NEXVARY status | Gap |
|-----------|---------------------------|---------------|-----|
| **Chimera Tool** | Samsung Download Mode device-info per [chimeratool.com/features](https://chimeratool.com/en/features/device-info) | DeviceIntelligence covers USB info; no Samsung-specific DL-mode ops | Chipset-specific service ops for Samsung, write-authorized flash |
| **Hydra Tool** | Chipset-organized support catalog per [docs.hydradongle.com](https://docs.hydradongle.com/supported-devices/) | ServiceCatalog has 14 groups; no exact-device op success claimed | Expand OEM model corpus; field-test each op on physical devices |
| **UnlockTool** | Official source returned HTTP 403 — capabilities Unknown | Unknown | Do not assume features; obtain primary source before crediting |
| **Octoplus Box** | Repair/firmware/information features per [octoplusbox.com/features](https://octoplusbox.com/en/features/features/) | FirmwareBridge covers ZIP/AVB analysis; no flash/unlock ops | Authorized write ops for specific chipsets; per-model acceptance |
| **EFT Pro** | Qualcomm/MTK device unlock and firmware tools — public documentation limited | DeviceCompatibility tracks Qualcomm/MTK; no EDL/BROM ops | Verify source; do not implement bypass without authorization |
| **UMT** | Universal Mobile Tool — multi-brand repair per official site | No UMT-specific ops implemented | Needs primary source review before any implementation |
| **SigmaKey / Sigma Plus** | Multi-brand repair dongle — public feature list limited | No SigmaKey-specific ops implemented | Needs primary source review before any implementation |

**Benchmark protocol:** Pin exact competitor version, hardware, OS and settings. Use a lawful synthetic or consented device corpus. Record SHA-256 of dataset and tool binary. Report exact result counts and failures. Never infer support from model counts alone.

---

## Intelligence competitive benchmarks

| Competitor | Role | Public capability | NEXVARY position |
|-----------|------|------------------|-----------------|
| **Maltego** | OSINT graph analysis | Entity/link graph transforms, public and commercial data sources | NEXVARY Intelligence uses Lampyre as backend; no Maltego integration planned |
| **OSINT Industries** | Commercial OSINT lookup | Email/phone/username search, relationship graph | Contract verification required before enabling commercial resale; EUR 14.00 confirmed |
| **Lampyre** | Our backend provider | MCP tool catalog, unified lookup, specialized services | Integrated via LampyreConnection — NOT a competitor |

---

## Engineering priorities

See `docs/ENGINEERING_BACKLOG.md` for ordered tasks.

### Release blockers (P0)

1. **Audit-store discrepancy** — 135 vs 139 records, root cause unresolved. `DEVELOPMENT_STATE.md` documents it as CRITICAL/OPEN.
2. **Windows 10/11 physical acceptance** — hosted CI currently has QML failures; clean consumer hardware with USB device unverified.
3. **Authenticode signing** — packages are unsigned; publisher certificate not provisioned.
4. **linux-forensic CI failure** — `Configure and build` fails on `feat/intelligence-ux-polish-20261117`; compiler diagnostic confirmed at LampyreConnection.cpp:619; inherited classification requires source comparison.

### Active development branch

`feat/intelligence-ux-polish-20261117` — Intelligence UX polish on top of PR #24 (0.4.10).
Latest commit: `b592acf`. PR target: `fix/intelligence-p0-completion`.

---

## Roadmap phases

### Phase 1 — Windows 0.4.x stabilization (current)
- Complete Intelligence UX: price banner, search confirmation, results display
- Resolve linux-forensic CI build failure
- Physical Windows 10/11 + USB device acceptance
- Authenticode signing prerequisite

### Phase 2 — Mobile Repair depth (next)
- OEM service catalog expansion: Samsung, Xiaomi, OPPO chipset ops
- Authorized write operations with per-model field acceptance
- Barcode printer and scanner hardware acceptance
- Workshop backup/restore full cycle on physical Windows

### Phase 3 — Firmware and device intelligence
- Firmware flash risk rules per chipset/build
- AVB signature verification (current: structure only)
- Device compatibility field corpus (owned devices)

### Phase 4 — Intelligence commercial readiness
- Lampyre gateway deployment and authenticated adapter
- EUR 14.00 checkout and invoice reconciliation against live gateway
- Case-linked evidence attachment for search results

### Phase 5 — Training and research
- Graded scenario assessment with independent reviewer
- Research-to-product promotion gate
- AR/EN curriculum documentation

## Verification scope

“Implemented” above describes source presence, not production or physical-device certification. Model counts and competitor capabilities are historical planning references, not independently repeated acceptance tests. Current CI run 44110539719 failed on both Linux GUI builds and Windows QML assertions. Hosted CI, physical devices, commercial gateway activation and signing remain separate gates.
