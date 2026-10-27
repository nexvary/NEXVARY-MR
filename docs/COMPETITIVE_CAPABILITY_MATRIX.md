# Government competitive capability matrix — 2026-10-25

Current trust increment (implementation; exact CI status in DEVELOPMENT_STATE.md): protected signed policy history, separate-account request/review/decision, one-time requester activation, revocation/expiry, public audit export and independent Python verifier are integrated. Direct grants are disabled. See GOVERNMENT_POLICY_AUDIT.md for tested scope and remaining institutional/OS/key/time/rotation/completion-receipt gaps. Earlier checkpoint rows below remain historical where they say durable policy audit/delegation is unimplemented. No competitor executable or comparative superiority result is introduced.


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
| Backup Analysis | Tested (inventory only) / Pending | Unencrypted/uncompressed ADB v5 app-member inventory with hashes/offsets; source ef86c995 / CI 40229014198, 52 independent fixture checks. No app decoding or generic vendor coverage. Protected case recovery is separate. |
| Evidence Integrity | Tested | Protected copies, SHA-256 and independent package verification; no external authenticity/time proof. |
| Chain of Custody | Tested | Linked declared transfers/audit; physical handover remains external. |
| SQLite / Media / Log Analysis | Tested (bounded) | Standalone SQLite samples, JPEG EXIF, classic PCAP and exported threadtime; no full application decoding. |
| Timeline Analysis | Tested (bounded) | Async timestamp-literal projection, source 3bf0485 / CI 40223653000; timeline 26/26. No complete event semantics or comparative superiority. |
| Relationship Analysis | Tested (bounded provenance) / Pending | Typed case/device/evidence/report/finding graph and recorded-digest groups; exact-source verification in DEVELOPMENT_STATE.md. No person/entity/social or causal inference. |
| Search and Indexing | Tested (bounded) / Pending | Original case-scoped retained-finding index with literal Arabic/English search, provenance paths and explicit caps; exact-source results in DEVELOPMENT_STATE.md. Persistent/full-text/semantic and complete source coverage pending. |
| Reporting | Tested / Pending | Existing AR/EN case PDFs; timeline exports/signed timeline review pending. |
| Offline / Air-Gapped Operations | Tested (application gates) / Pending | Local case workflows plus native application category gates (current verification in DEVELOPMENT_STATE.md); OS egress/descendant/share containment and air-gap acceptance pending. |
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

Finding-index/provenance verification: da857f9 / CI 40261921553, four 45/45 profiles, artifact 32/32, UI 49/49, production sanitizers 15/15. Known-answer structural corpus: 6 findings/19 nodes/20 correct edges/two digest groups, zero false/missed edges on that corpus only. Actual portable/installed EXE index receipts and real Windows frames retained. No semantic/person relationship, general population accuracy or competitive superiority claim.


### Offline feature review — official sources inspected 2026-10-26

These are **Officially Claimed**, dated/version-scoped vendor statements, not Independently Verified packet captures, licensing tests or air-gap acceptance. They do not establish current whole-suite behavior.

| Source / scope | Documented vendor statement | NEXVARY comparison / remaining gap |
|---|---|---|
| [Inseyets FAQ, October 2024, page 3](https://media.cellebrite.com/wp-content/uploads/2024/10/Cellebrite-Inseyets-FAQ.pdf) | Historical offline configuration option, with network connectivity described for full functionality | Application gating is not a unique offline capability. Direct access to the current product page returned 403; current licensing/egress details remain Unknown here. |
| [XAMN Viewer 8.1 release, 20 March 2025](https://www.msab.com/updates/xry-11-0-xamn-8-1-unify-25-3-xec-7-13-and-kte-11-0/) | Offline maps in that named Viewer release | Offline geographic-map analysis is Unsupported in NEXVARY; no whole-suite isolation inference. |
| [AXIOM 8.6/Copilot early-access feature, 24 October 2024](https://www.magnetforensics.com/blog/making-magnet-copilots-ai-capabilities-available-offline/) | Offline Q&A/image deepfake classification; the described Medex video analysis uses online metadata | Semantic Q&A/deepfake classification is Unsupported in NEXVARY. Individual feature network requirements must be evaluated separately. |

Source/access/limit records: evidence/offline-policy-2026-10-26/research-source-review.json. No licensed competing product was executed or superiority established.

Application gate verification: 7cfa1a6 / CI 40268766117, all seven jobs, four 46/46 profiles, policy 10/10, mirror 22/22, UI 50/50 and 16/16 production sanitizers. Actual deployed default-deny/grant/revoke receipts and frames retained. Complete OS air-gap containment remains Pending; no competitive superiority is inferred.

## Signed-policy increment source review — 2026-10-26

- [MSAB XAMN 7.3 release, 2022-10-06](https://www.msab.com/updates/new-release-xry-10-3-xamn-7-3-and-xec-7-3/): the vendor describes detailed evidence-review/access logging. The audit cell is now Officially Claimed for that historical version, not independently tested or assumed to describe the latest release.
- [Oxygen Review Center enterprise documentation](https://enterprise.oxygenforensics.com/blog/how-oxygen-review-center-provides-real-time-collaboration-anytime-you-need-it/): describes user logs and case-administrator/reviewer/uploader roles. Publication date was not confirmed. These belong to a separate enterprise component; no standalone Detective or cryptographic-history guarantee is inherited.
- Current official Inseyets and Autumn 2026 release URLs returned 403 on direct retrieval; an Oxygen public-sector URL returned 502. Official search excerpts were available but were not treated as full-page inspection, exact-device validation or executable verification. Access outcomes: `evidence/policy-trust-2026-10-26/competitive-source-review.json`.

NEXVARY's new independently verifiable **local operational-policy** history and reviewed activation reduce an implementation gap. They are not equivalent workloads to vendor evidence-access logs, enterprise identity or collaboration. Missing public cryptographic details remain Unknown, not absent. No competitor was executed and no superiority/parity is established. Full source/CI/deployed status and bounds: DEVELOPMENT_STATE.md / GOVERNMENT_POLICY_AUDIT.md.

## Operation-result receipt increment — 2026-10-27

Implemented first coverage: the existing ClamAV scan/FreshClam workers now bind a signed start/result to the actual reviewed approval sequence/hash, requesting principal, declared case reference, local timestamps and measured report/source digests. Government Administration exposes the real bounded native receipts. Native plus separate Python verification rejects forged approval references, content fields, duplicate/unauthorized completion and success after revocation. Exact tests/CI status: DEVELOPMENT_STATE.md; scope: GOVERNMENT_OPERATION_RESULTS.md.

Comparison axis: existing dated MSAB XAMN 7.3 audit/access logging and separate Oxygen Review Center roles/logs source records, not equivalent operation-result workloads or proof of their cryptographic internals. Prior MR gap: gate decisions were not correlated to worker outcomes. Measurement: signed known-answer histories accepted by both implementations, adversarial valid-signature histories rejected, exported completed scan bytes matching the receipt digest, cancellation/logout/revocation/expiry coverage. No competitor executable or comparative superiority result. Remaining: all other engines, actual OS/network/descendant observations, independent institutional review and field acceptance. Maintenance estimate: medium/high (lifecycle consumers, verifier compatibility, retention/recovery), not measured cost.

## Authorized partition-result correlation — 2026-10-27

P0 increment: the existing local raw-image/mmls adapter now correlates a signed worker start/result with its reviewed approval, source SHA-256 and exact exported partition-provenance payload SHA-256. Previously it had a case activity record and application gate without an operation-result correlation. Native and separate Python validators recognize this registered kind; Government Administration renders its real receipts in AR/EN. A completed background candidate is held until the GUI callback rechecks authority. Known-answer and adverse lifecycle/storage/queued-callback checks, plus real installed Linux mmls integration, are the measurement criteria; exact execution/CI evidence is in DEVELOPMENT_STATE.md. Two-store crash atomicity and guaranteed rollback remain unimplemented.

Comparison axis/source: the existing dated MSAB XAMN audit/access and Oxygen Review Center role/log source records above, reused without a new vendor-source review. No equivalent competitor partition-result workload or cryptographic audit internals were independently tested. Result: a bounded MR coverage gap is reduced from two to three registered worker kinds; no superiority/parity conclusion. Implementation cost is not measured; maintenance estimate medium (adapters, compatible verifiers, case/journal failure recovery). All-engine receipts, institutionally retained anchors and independent OS containment remain Pending.


## Prepared local case transactions — 2026-10-27

This row updates NEXVARY evidence only. Existing dated competitor source records
above are unchanged; no new vendor-capability or parity claim is inferred.

| Competitor Capability | NEXVARY Capability | Gap | Implementation | Benchmark | Evidence | Limitations | Verification Status |
|---|---|---|---|---|---|---|---|
| Cellebrite, Magnet, MSAB, Oxygen, Belkasoft and Elcomsoft cross-store crash semantics: not independently measured in this review | Signed prepared local PCAP/EXIF case intent, case/evidence/revision/source/report binding, conservative restart reconciliation and immutable terminal decisions | Original RecoveryBridge partition store and other adapters still need verified linkage/migration; OS descendants remain open | Existing protected policy journal + LawCaseStore SQLite transaction + native authenticated bridge; exact post-replacement journal readback | Seven process-interruption stages; 16 native transaction cases and 51 native/Python signed-history checks; not a throughput or power-loss benchmark | source 7844f01; evidence/case-transactions-2026-10-27; GOVERNMENT_CASE_TRANSACTIONS.md | Original audit snapshot-lag mechanism is unresolved; local accounts/time only; retained external anchors and independent institutional review required | Scoped first-party implementation/test evidence; no comparative superiority, parity, device or institutional certification |

Local first failures, failed-fixture isolation and exact subsequent observations
are retained, including 18-on-disk versus 22-in-memory signed-history observations.
A fresh-fixture pass is not used to erase the original reliability failure.
