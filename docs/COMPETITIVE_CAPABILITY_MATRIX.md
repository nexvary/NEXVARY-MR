# Government competitive capability matrix — 2026-10-25

Scope: a bounded primary-source review, not a completed deep validation of every product. No commercial product was licensed, installed or exercised. No comparative superiority is established. The [machine-readable matrix](../research/competitive/government-capabilities.json) contains all 20 requested axes for all 14 products/tool families. Each cell records classification, sources, scope and confidence. Unknown means not determined, never absent. Acquisition tools do not inherit an analysis product's capabilities because they share a vendor.

Competitor classes: Officially Claimed / Independently Verified / Publicly Documented / Unverified / Unknown. Vendor manuals can be Publicly Documented without being Independently Verified. No cell is promoted from a NIST index entry: individual version-specific report tables still need review.

| Product | Scoped dimensions with a source | Sources / boundary |
|---|---|---|
| Cellebrite UFED | Unknown in this review | Review leads: NIST |
| Cellebrite Physical Analyzer / Inseyets | SQLite / Media / Log Analysis; Timeline Analysis; Reporting; Research Extensibility | PA |
| MSAB XRY | Authorized Data Acquisition | XRY |
| MSAB XAMN | SQLite / Media / Log Analysis; Timeline Analysis; Relationship Analysis; Search and Indexing; Reporting | XAMN |
| Oxygen Forensic Detective | Timeline Analysis; Relationship Analysis; Search and Indexing | OXYGEN |
| Magnet AXIOM | Timeline Analysis; Relationship Analysis | MAGNET-INTEGRATION |
| Magnet Graykey | Unknown in this review | Review leads: MAGNET-INTEGRATION, NIST |
| Belkasoft Evidence Center X | Timeline Analysis | BELKASOFT |
| Elcomsoft iOS Forensic Toolkit | Android / iOS Support; Authorized Data Acquisition | ELCOMSOFT |
| Chimera Tool | Device Coverage | CHIMERA |
| Hydra Tool | Device Coverage | HYDRA |
| UnlockTool | Unknown in this review | Review leads: UNLOCK |
| Octoplus | Device Coverage | OCTOPLUS |
| Official OEM tools | Unknown in this review | Review leads: product-specific OEM review pending |

## Source register

- **PA** — [Officially Claimed](https://cellebrite.com/en/products/cellebrite-inseyets/physical-analyzer/): Vendor describes timeline, decoding/import, scripting and reporting.
- **XRY** — [Officially Claimed](https://www.msab.com/product/xry-extract/xry-logical/): Logical acquisition product; exact model and method acceptance unresolved.
- **XAMN** — [Officially Claimed](https://www.msab.com/product/analyze/xamn-pro): Vendor describes timeline, connections, filters, file viewers and reports.
- **OXYGEN** — [Officially Claimed](https://www.oxygenforensics.com/technical-resources/10-analytical-features-available-in-oxygen-forensic-detective/): Published 2024 feature article describes timeline, social graph and search; no current version equivalence inferred.
- **AXIOM** — [Officially Claimed](https://www.magnetforensics.com/products/magnet-axiom/): Multi-source investigation product; not a device coverage validation.
- **MAGNET-INTEGRATION** — [Officially Claimed](https://www.magnetforensics.com/blog/automatically-import-and-process-mobile-images-in-magnet-axiom-from-a-graykey-verakey/): Historical vendor integration/analytics article; exact versions and device success unresolved.
- **BELKASOFT** — [Publicly Documented](https://belkasoft.com/timeline_full): Vendor user documentation describes UTC/local chronological views; not independent verification.
- **ELCOMSOFT** — [Officially Claimed](https://us.elcomsoft.com/eift.html): Method/model/platform-dependent Apple acquisition; no universal deeper access inferred.
- **CHIMERA** — [Officially Claimed](https://chimeratool.com/en/features/device-info): Samsung Download Mode device-information feature; not universal device support.
- **HYDRA** — [Publicly Documented](https://docs.hydradongle.com/supported-devices/): Chipset-organized support catalog; no exact-device operation success inferred.
- **OCTOPLUS** — [Officially Claimed](https://octoplusbox.com/en/features/features/): Repair/firmware/information feature families; no exact-device success or forensic equivalence.
- **UNLOCK** — [Unknown](https://unlocktool.net/): Official source returned HTTP 403; reseller/mirror support figures excluded.
- **NIST** — [Publicly Documented](https://www.nist.gov/itl/csd/secure-systems-and-applications/computer-forensics-tool-testing-program-cftt/cftt-7): Mobile testing index located; individual report result tables not reviewed. No certification or general pass inferred.
- **RCS** — [Publicly Documented](https://citizenlab.ca/research/mapping-hacking-teams-untraceable-spyware/): Original 2014 defensive infrastructure research; no leaked code/materials or implant reuse.

RCS is excluded from the forensic product matrix. Its historical defensive research informs egress/provenance testing, not covert collection implementation. No leaked source, licensed commercial code, implants or proprietary assets were used. OEM tools require named product/version rows before crediting a capability.

## NEXVARY implementation and verification

Every status is scoped. Earlier tests belong to executable feed968f and CI 40197393089; current timeline tests belong to their exact source/CI in DEVELOPMENT_STATE.md. No capability here is Field Verified. Lab Verified requires a separately reviewed reproducible laboratory protocol, not just CI.

| Axis | Status | Proof / remaining boundary |
|---|---|---|
| Device Coverage | Implemented / Pending | Existing USB/device assessment; exact model/OS/firmware acceptance pending. |
| Android / iOS Support | Implemented / Unsupported | Existing authorized Android metadata; no native iOS acquisition capability inferred. |
| Authorized Data Acquisition | Tested (local-file intake) | Existing bounded authorized file registration/copy; no protected-phone physical extraction. |
| Backup Analysis | Pending | Protected NEXVARY case recovery exists; vendor backup/container parsing is different work. |
| Evidence Integrity | Tested | Protected copies, SHA-256 and independent package verification; no external authenticity/time proof. |
| Chain of Custody | Tested | Linked declared transfers/audit; physical handover remains external. |
| SQLite / Media / Log Analysis | Tested (bounded) | Standalone SQLite samples, JPEG EXIF, classic PCAP and exported threadtime; no full application decoding. |
| Timeline Analysis | Tested (bounded) | Async timestamp-literal projection, source 3bf0485 / CI 40223653000; timeline 26/26. No complete event semantics or comparative superiority. |
| Relationship Analysis | Pending | Source/provenance IDs only; no person/entity/social inference. |
| Search and Indexing | Tested / Pending | Existing case search; timeline literal search in this increment; full artifact index pending. |
| Reporting | Tested / Pending | Existing AR/EN case PDFs; timeline exports/signed timeline review pending. |
| Offline / Air-Gapped Operations | Implemented / Pending | Local case workflows; actual enforced egress/child/share containment and air-gap acceptance pending. |
| Enterprise Security | Tested / Pending | Existing native local identities/DPAPI; enterprise identity and independent audit pending. |
| Role-Based Access Control | Tested | Existing roles/grants; no SSO/personnel verification claim. |
| Auditability | Tested / Pending | Existing hash-linked audit/local signed anchors; external key/time custody pending. |
| Reproducibility | Tested / Pending | Existing deterministic reports; independent cross-institution reproduction pending. |
| Research Extensibility | Tested / Pending | Existing catalogs/experiments/reviews; production promotion and hardware validation pending. |
| Deployment and Maintenance | Tested / Pending | Earlier hosted portable/install lifecycle; current source needs exact-source CI; clean Windows 10/11 and publisher signing pending. |
| Cost of Ownership | Pending | No measured agency support/TCO or equivalent competitor contract comparison. |
| Training and Localization | Tested / Pending | Existing synthetic scenarios and AR/EN; full training curriculum/independent assessment pending. |

## Scoped requirement review

[NIST Mobile Device Forensic Tool Specification v3.3 (January 2025)](https://www.nist.gov/document/mobile-device-forensic-tool-test-specification-v33), sections 6.2.4 and 6.2.7, was inspected. MDT-RO-15 concerns temporal ordering and MDT-AO-34 presentation of supported timestamps. The bounded NEXVARY projection requires cap/unsupported disclosure and further validation; no conformance is claimed. MDT-RO-11 concerns active-table coverage; NEXVARY's first-32-row sample is incomplete for that purpose. WAL coverage in MDT-AO-28 is unsupported by the current adapter. This review exposes gaps rather than converting internal tests into accreditation.

The index's AXIOM 8.1 and Inseyets 10.3 report PDFs returned HTTP 403, so their result tables remain unreviewed. No Independently Verified product cell is inferred. URLs/access outcomes are retained in evidence/timeline-2026-10-25/research-source-review.json.
