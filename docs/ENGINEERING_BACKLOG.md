# NEXVARY-MR Engineering Backlog

Updated: 2026-11-17. Branch: `feat/intelligence-ux-polish-20261117`. Base: `e561322` (PR #24).
Each task has a test criterion. P0 = release blocker. P1 = daily operation. P2 = expansion.

---

## P0 — Release blockers

| ID | Task | Test criterion | Blocker dependency |
|----|------|---------------|-------------------|
| B01 | **Resolve audit-store discrepancy (135 vs 139)** | Reproduce the 135/139 divergence; identify root cause; native test confirms correct count | DEVELOPMENT_STATE.md CRITICAL/OPEN |
| B02 | **linux-forensic CI build failure** | `Configure and build` step passes on `feat/intelligence-ux-polish-20261117`; all linux-forensic jobs green | Compiler failure confirmed at LampyreConnection.cpp:619; inheritance under review |
| B03 | **Windows 10/11 physical acceptance** | Install portable + installer on clean Windows 10 and 11; USB device connect/disconnect cycle; no crash | Requires physical hardware |
| B04 | **Authenticode signing** | `Get-AuthenticodeSignature` returns valid result; publisher matches; SHA matches installer artifact | Certificate not provisioned |
| B05 | **Draft PR against fix/intelligence-p0-completion** | PR created; CI triggered; no force-push to main | Connected Nexvary GitHub account can create a Draft PR without storing a token on Lightsail. Comparison: `https://github.com/nexvary/NEXVARY-MR/compare/fix/intelligence-p0-completion...feat/intelligence-ux-polish-20261117` |

---

## P1 — Daily operation tasks

| ID | Task | Test criterion | Pillar |
|----|------|---------------|--------|
| B10 | Expand OEM service catalog — Samsung chipsets | ServiceCatalog returns correct ops for Exynos/Snapdragon Samsung models; unauthorized write rejected | OEM Service Engine |
| B11 | Expand OEM service catalog — Xiaomi/OPPO | Same pattern as B10 for Dimensity/Snapdragon Xiaomi and OPPO models | OEM Service Engine |
| B12 | ADB backup parser for unencrypted ab files | Known fixture: exact app member count, hashes and offsets match reference; malformed rejected | Android Service Tools |
| B13 | Workshop barcode printer acceptance | Scan printed barcode reopens correct repair order on Windows; AR and EN label tested | Mobile Repair Studio |
| B14 | scrcpy frame delivery on physical device | Frames visible in DevicesPage mirror; disconnect cleans up process; no orphan process on exit | USB and Hardware Diagnostics |
| B15 | Device compatibility field corpus | At least two owned physical devices accepted with correct chipset/build match; Unknown disclosed | Device Compatibility Engine |
| B16 | Intelligence results case attachment | Provider result attached to authorized case as unverified evidence; audit record written | NEXVARY Intelligence |
| B17 | Phone OSINT numbering-plan offline | International number returns carrier/region without network call; result retained in case | NEXVARY Intelligence |

---

## P2 — Expansion tasks

| ID | Task | Test criterion | Pillar |
|----|------|---------------|--------|
| B20 | AVB signature verification | Known signed vbmeta accepted; tampered vbmeta rejected; no OEM key assumed | Firmware Intelligence |
| B21 | PCAPNG reader | Known PCAPNG fixture: correct packet count, endianness, truncation disclosed | USB and Hardware Diagnostics |
| B22 | ADB backup app decoding | One supported app container: known record type/count; unsupported disclosed | Android Service Tools |
| B23 | Training scenario grading | Synthetic scenario: correct decisions score 100%; incorrect decisions fail with explanation | Training and Simulation Labs |
| B24 | Lampyre gateway integration | EUR 14.00 checkout against staging gateway; invoice reconciled; no double-charge | NEXVARY Intelligence |
| B25 | Offline geographic map analysis | Case map rendered without network; tiles loaded from local bundle | NEXVARY Intelligence |
| B26 | Competitive benchmark dataset — Chimera/Hydra | Same chipset op on owned device: both tools produce documented result; comparison recorded | Advanced Mobile Research |
| B27 | Research-to-product promotion gate | Research candidate passes: primary source, owned device, protocol, digest, reviewer independence | Advanced Mobile Research |

---

## Completed in feat/intelligence-ux-polish-20261117

| SHA | Description |
|-----|-------------|
| `f897373` | Intelligence UX: NEXVARY header, EUR 14.00 price banner, accent-blue category cards, dynamic circular relationship graph |
| `edde1a9` | Fix permission rule order in .opencode/opencode.json (deny last wins) |
| `d23140d` | Findings summary row: count sources-with-data / empty / failed |
| `0004b31` | 5 new QML tests: price banner, phone validation, filter, consent, pending result |
| `6ef0b40` | Replace absolute machine paths with portable ~ in opencode.json |
| `159db9c` | Add branch to CI push trigger |
| `b592acf` | Pending-result alert banner + test (11 IntelligenceCards tests total) |
