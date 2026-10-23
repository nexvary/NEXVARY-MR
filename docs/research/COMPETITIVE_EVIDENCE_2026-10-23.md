# Competitive evidence snapshot — 2026-10-23

This is a scoped public-source review. No commercial tool was installed, licensed or tested in this session. No exact phone/model/firmware equivalence or overall superiority is established. A vendor's model count is not accepted as functional evidence.

Evidence levels: **V** vendor statement; **T** public technical definition/procedure; **I-index** independent report located but its results not yet reviewed; **N** NEXVARY code/fixtures; **H** authorized physical acceptance. These are separate columns, not interchangeable badges.

| Product / source | Scoped published function | Devices, versions and modes | License/access evidence | Risk and verification boundary |
| --- | --- | --- | --- | --- |
| [Chimera Device Info](https://chimeratool.com/en/features/device-info) | V: Samsung rescue information; distinguishes Download Mode Device Info from ADB Get Info | Exact model/firmware procedure still required; listed count not imported | Vendor lists Basic/Professional/Premium and function packs; entitlement not tested | Metadata can inform a repair; no proof every listed model succeeds |
| [Hydra documentation](https://docs.hydradongle.com/supported-devices/) / [device portal](https://www.hydradongle.com/devices) | V: chipset-family organization and module/device catalog | MTK, Qualcomm, Unisoc, HiSilicon, Exynos, Apple, Allwinner, Rockchip categories; per-operation versions unresolved | Vendor license/credit ecosystem; exact procedure cost and entitlement not tested | A category is not a protocol implementation or physical acceptance; writes remain potentially destructive |
| [UnlockTool](https://unlocktool.net/) | Official page returned HTTP 403 in this review | Unresolved; reseller/mirror counts excluded | Unresolved from accessible primary source | No new technical/support claim accepted from mirrors |
| [Octoplus](https://octoplusbox.com/en/features/features/) / [official portal](https://octoplusbox.com/) | V: repair/flashing product and feature families | Brand/feature-specific modules; exact firmware support not tested | Vendor describes smart-card reactivations and digital licenses | Repair writes can lose data; security-bypass features are outside NEXVARY scope |
| [Cellebrite Inseyets](https://cellebrite.com/en/products/cellebrite-inseyets/) | V: acquisition via UFED/Premium plus decoding/analysis workflow | Exact method/device/security-patch conditions not public-proof here | Commercial access; exact contract/edition not tested | I-index entry below; no current universal extraction or decryption claim accepted |
| [MSAB XRY Logical](https://www.msab.com/product/xry-extract/xry-logical/) / [FAQ](https://www.msab.com/support/faq/) | V: operating-system-mediated logical extraction and reports | Windows product; device manual describes per-device distinctions | Modular commercial offering; entitlement not exercised | Logical/physical methods must be evaluated independently |
| [Oxygen Forensic Detective](https://www.oxygenforensics.com/products/oxygen-forensic-detective/) | V: collection, import, analysis and export across multiple source types | Vendor lists broad mobile/cloud/computer coverage; no exact model acceptance here | Vendor network license; deployment and entitlement not exercised | Cloud acquisition involves credentials/network scope; unsupported for NEXVARY automatic collection |
| [Magnet AXIOM](https://www.magnetforensics.com/products/magnet-axiom/) | V: multi-source processing/analysis and investigation workflow | Import/parser versions and artifacts require separate testing | Commercial product; pricing/edition not evaluated | Parser completeness/accuracy must be measured against known fixtures |
| [Magnet Graykey](https://www.magnetforensics.com/products/magnet-graykey/) | V: mobile acquisition integrated with the vendor ecosystem | Exact device/security-patch/acquisition scope not independently reproduced | Controlled commercial access; current eligibility/entitlement not tested | Acquisition is distinct from artifact analysis; no proprietary access implementation reused |
| [Elcomsoft iOS Forensic Toolkit](https://us.elcomsoft.com/eift.html) | V: logical acquisition, media/logs and model-dependent deeper acquisition | Vendor distinguishes macOS/Windows/Linux; Windows lacks some bootloader functions | Commercial license; exact edition not exercised | Platform parity must not be inferred; protected-device bypass is outside NEXVARY scope |
| [Belkasoft X](https://belkasoft.com/x) | V: acquisition, artifact analysis, timeline, search and reporting | Multiple image/mobile sources listed; methods and versions conditional | Current page says government customers and perpetual license; not independently verified | A supported input type does not prove complete extraction or plaintext recovery |

## Independent testing, with version boundaries

[NIST's mobile tool-testing index](https://www.nist.gov/itl/csd/secure-systems-and-applications/computer-forensics-tool-testing-program-cftt/cftt-7) lists reports for Inseyets.UFED 10.3.0.260 / PA 10.3.0.3169, XRY 10.9.0, Oxygen 17.1.0.131, AXIOM 8.1.0.42087, Graykey 4.1.0, Elcomsoft 8.30 and Belkasoft X 2.7.19314. **I-index only**: the individual result tables were not reviewed here; one report fetch failed. Their presence is not an endorsement, pass claim or proof about later versions.

The [NIST mobile test specification v3.3](https://www.nist.gov/document/mobile-device-forensic-tool-test-specification-v33) separates requirements, assertions, procedures and expected results. Engineering implication: compare supported artifact accuracy/completeness against populated fixtures, including negative results. NEXVARY has not completed that specification or obtained NIST certification.

## NEXVARY comparison and measurable opportunities

| Capability | NEXVARY implementation / evidence | Remaining evidence |
| --- | --- | --- |
| Device identity and diagnostics | N: shared Qt transport, duplicate-serial rejection, structured health observations and synthetic service fixtures | H: exact model/OS/USB conditions, no phone accepted yet |
| Production safety | N: compiled default-deny command boundary; approved read-only plans; separate Laboratory binary | Native Windows and authorized hardware review |
| Firmware intelligence | N/T: local SHA-256/manifest matching plus original bounded AVB envelope inspection against public AOSP format | OEM samples, differential comparison, signature/OEM trust and physical compatibility |
| Evidence handling | N: working copies, SHA-256, case/custody records, timeline, report fixtures; external mmls adapter | Broader artifact corpus, independent repeatability, privacy/customer acceptance |
| Workshop CRM | N: local SQLite customer/device/order/inventory/report tests | Real workshop workflow and upgrade acceptance |
| Deep protected-device extraction | Unsupported; no lock/account bypass or proprietary exploit chain | Not an inferred free replacement for restricted commercial methods |

Prioritize measurable goals: zero device-writing child processes in denied production cases; exact duplicate-target rejection; deterministic corrupt-file rejection; preservation of source hashes; explicit unsupported results; and accurate per-model evidence. Performance/coverage superiority needs comparable datasets, pinned tool versions and recorded measurements. None is claimed now.
