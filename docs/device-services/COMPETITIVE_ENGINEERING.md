# Competitive engineering research — 2026-10-18

## Scope and evidence rules

This is an independent implementation, not an integration of Chimera, UnlockTool or Octoplus. No commercial executable, protected artwork, firmware, secrets or bypass code was copied. Research uses public documentation; none of the three commercial programs was licensed, installed or exercised in this environment. Their statements are **vendor claims**, not NEXVARY test results. Published pages describe behavior, not internal architecture. Architectural conclusions below are our engineering inferences.

`sources.json` records URLs, retrieval date and access limitations. Video landing pages/descriptions were read; video frames and complete transcripts were not reviewed. Model counts change and are not acceptance criteria. Unknown Android releases, chipset variants, driver bindings and server requirements stay unknown.

## Repository baseline, inspected before development

Fetched remote heads: main `7446b17d0d83d4e7d9da3c8d4bbdd7dae6d4de17`; native `0bfe9b113cfac6c9f50f0e278d6faa8ff6733ffc`; Python OEM `ef302c9555dd4fc0b5189182be5bd109d932bbdf`; forensic `0f979797f03cabf0dc0a504f23dc5c5f9c2cd116`.

The forensic head already contains the native workshop, ClamAV single-file adapter, model-guide import, driver-source catalog, firmware preflight, evidence/custody/timeline/reporting and tracking-server work. Its [native run 39140999146](https://github.com/nexvary/NEXVARY-MR/actions/runs/39140999146) and [Python run 39140999163](https://github.com/nexvary/NEXVARY-MR/actions/runs/39140999163) succeeded. Those runs do not validate this increment. Current execution results belong in the PR/checks for the new commit. The old OEM branch is Python and remains unmerged; its brand/model requests inform data, not a second executor.

## Chimera: connection first, capabilities second

[C1] describes detecting a device and selecting a suitable processor protocol when a brand-specific module is unsuitable. This suggests separating transport identification from the model catalog. A generic MTK grouping is not proof that an arbitrary MTK phone accepts any procedure or Download Agent. [C2] distinguishes minimal Samsung Download information from fuller ADB information; the selected mode determines what can be read. A brand name alone must not enable Download or EDL.

[C3] exposes model/procedure filtering, while [C4] records fixes to detection and individual procedures. These should be represented as versioned capability evidence, not a single supported-brand flag. NEXVARY implements fresh protocol observations and expiring snapshots; exact-model OEM adapters remain absent.

[C5] describes firmware mode selection, verified sources, stable power and data-loss risks; its language workflow involves region firmware/CSC, not adding arbitrary translations with a locale setting. [C6] concerns device-specific calibration/safety backup, not a universal user-data backup. NEXVARY retains hash/metadata preflight and encrypted case storage, without claiming equivalent calibration backup.

[C7] documents PC attachment/license rules; [C8] lists per-procedure credits. Firmware-page pricing and the credit table are not completely uniform, so cost must be checked for the exact current operation rather than hard-coded as free. [C9] supplies installation/account instructions. [C10] is an advanced-firmware tutorial description with partition-operation risks. We infer a useful UX: detect, explain requirements, review compatibility/consent, execute with bounded logs, then report. Proprietary detection confidence and retry algorithms are not observable from these pages.

## UnlockTool: source qualification and coverage limits

The requested `www.unlocktool.org` explicitly identifies itself as an independent resource/authorized reseller, not the software developer [U1]. Its advertised brand/chipset/functions are **reseller statements**. They are not imported as verified capability grants.

Direct requests to `unlocktool.net`, `/models/`, `/download/` and `file.unlocktool.net` returned HTTP 403 [U2]. Search indexing exposed an update-file listing, but its update body was inaccessible. We cannot verify current subscription/account limits, server/credit requirements, firmware-validation behavior, cancellation or error handling from it. No access controls were circumvented.

The accessible `edl.unlocktool.net` reference library [U3] categorizes brand/model/SoC labels; its footer identifies ChuyềnMobileGsm, so it is not an OEM specification. An OPPO CPH2577 / MT6768 label is retained as attributed reference data, with unknown Android and **no supported NEXVARY OEM operation**. No copyrighted test-point images are reproduced. Some labels can conflict with other specifications; an exact OEM source plus hardware validation is required before trusting one for execution.

The engineering lesson is a searchable brand → exact model → chipset/build → connection mode → prerequisites → operation matrix. NEXVARY ships 14 brand groups, five chipset families and 44 provenance-bearing references. Forty-two are user-requested OPPO/vivo references with unknown SoC/Android; two are attributed vendor references. All remain unvalidated here. Qualcomm/MediaTek/Unisoc/Exynos/Kirin are taxonomy, not operating capabilities. Root, a signed loader, authentication, partition layout and current firmware are separate facts.

## Octoplus: specialized modules and explicit dependencies

[O1] separates Samsung, LG, Huawei and FRP activations and digital licenses; some operation paths require a box/dongle, while documented digital licensing can avoid it. [O2] is a legacy LG Qualcomm workflow: select its module, identify the device, then choose compliant firmware. Its NVM/firmware functions cannot be generalized to modern Snapdragon devices.

[O3] is a feature list, not a model-specific test matrix. [O4] lists devices but cannot certify every feature on every listed phone. [O5] contains driver packages by family/mode, including old Windows releases; availability is not evidence of Windows 11 compatibility or permission to redistribute them. [O6], dated 2026-10-08, explicitly ties certain new Xiaomi Recovery/EDL operations to server credits and lists Samsung model/bootloader-bit conditions. NEXVARY does not implement those protection procedures.

Our inference: each module needs a UI, evidence-backed device scope, fixed command plan, common error/cancellation contract, private diagnostics, tests, versioned metadata and explicit permissions. Current ADB/Fastboot adapters satisfy this contract for generic protocol operations. Native executable adapter updates still require an application release; only reference-data updates are independent. A hot-loadable binary-plugin system is not implemented.

## Comparison matrix

`V` = public vendor documentation; `R` = reseller only; `?` = not verified; `I` = implemented here, synthetic tests only. Commercial entries never imply NEXVARY support. Driver/licensing abbreviations are expanded immediately below.

| Function | Chimera | UnlockTool | Octoplus | Scope / driver / server-license | NEXVARY and open implementation path | Risk; priority |
|---|---|---|---|---|---|---|
| Automatic detection | V C1,C4: module/protocol selection | R U1; actual confidence ? | V O2: select LG Qualcomm/read info; full automatic UX ? | Exact mode; D1/D2; L1/L2/L3 | I bounded ADB/Fastboot discovery; AOSP tools | Wrong target; P0 |
| Manufacturer/model/Android | V C2: mode-specific fields | R U1; U2 blocked | V O2/O4: model workflow | Authorized ADB or vendor mode; D1/D2; L1/L2/L3 | I allowlisted properties; missing fields unknown | Identifier disclosure; P0 |
| Chipset identification | V C1: processor module | Reference labels U3; runtime ? | V O2/O6: Qualcomm-specific scope | SoC/build evidence, not USB VID alone; D2 | I reported ro.soc fields only; no inferred loader support | Wrong firmware/loader; P0 |
| ADB/reboot/settings | C2 confirms ADB info; settings specifics ? | R U1 | O2 confirms info; general ADB scope ? | Authorized debugging, resolved activity; D1 | I serial-pinned reboot, supported settings and dark readback | Disrupts device session; P1 |
| Fastboot/bootloader | Detailed exact-model behavior ? | R U1 | Exact OEM/mode support varies; generic API ? | D1, OEM eligibility; commercial cost ? | I getvar/eligibility; no unlock/relock; AOSP | Wipe or boot failure; P0 |
| Recovery/sideload | Exact sideload behavior ? | R U1 | O6 documents specific Xiaomi Recovery server path | Recovery/OEM approval; D1/D2; L3 for O6 | I reviewed ZIP transfer via AOSP; TWRP must already match | Data loss/boot failure; P1 |
| Download/EDL/BROM/Preloader | V C1,C2: distinct module/mode | U3 reference labels, operation ? | V O6: model/bit and server restrictions | Signed OEM loaders/auth, D2; L1/L2/L3 | Unavailable; no generic OEM adapter | Severe corruption/protection risk; defer |
| Firmware management | V C5: modes/verified packages | R U1 | V O2/O3: matching file/write | Exact build/layout/signature; D1/D2; per-operation license | Existing SHA/manifest checks + I staged transfer; no general flasher | Hash ≠ authenticity or rollback safety; P0 |
| Partition information | V C10 description | R U1 | V O2/O3 legacy memory operations | Protocol/permissions; D1/D2; pricing ? | I /proc/partitions observations and fixed getvar keys | Incomplete/unavailable is allowed; P1 |
| Backup/restore | V C6: calibration/safety scope | R U1 | V O2/O3: NVM/security backup | Exact model/rights, D1/D2; L1/L2/L3 | Existing evidence-copy/case vault; general phone restore absent | Privacy/loss/incompatible restore; P0 |
| USB/driver diagnostics | C9 setup; automatic binding ? | R U1 | V O5 packages, O1 install guide | Signed exact mode/OS package; admin only for deliberate install | Existing PnP inventory + six official source links; physical correlation pending | Wrong driver/elevation; P0 |
| Error/log/progress | C4 documents defect fixes; algorithms ? | Runtime semantics ? | O1 update log, O2 guided steps | Server errors depend on licensed vendor; L1/L2/L3 | I phases/live bounded output/cancel/JSON; Qt QProcess | Host cancel cannot roll back device; P0 |
| Account/license | V C7/C8 PC/license/credits | U2 blocked, reseller only | V O1/O6 digital/box and server credits | Vendor services; private API availability unknown | Local services need no vendor account; no private API integration | Credentials/billing; defer |
| Official account recovery | Vendor protection features outside scope | Not implemented here | O6 proprietary protection operations outside scope | Owner-authorized OEM recovery only | I guided device settings, explicit unavailable explanations | No protection-bypass promise; P0 |

**D1:** the installed ADB/Fastboot Windows interface/driver must support the current mode. **D2:** vendor/chipset-specific driver/loader/box requirements must be verified for the exact operation; no universal driver is assumed. **L1:** Chimera account/license; additional credits depend on procedure and current terms. **L2:** UnlockTool official terms/server dependencies unverified due to access restrictions. **L3:** Octoplus module activation/digital license or supported hardware; explicitly server/credit based where O6 says so. Generic local NEXVARY operations do not contact any of these vendors. No statement here proves offline availability of a proprietary operation.

## Open-source candidates and implementation choices

| Candidate | Primary license/source | Decision / technical limit |
|---|---|---|
| ADB / Fastboot | AOSP sources [A1,A2]; Google Platform Tools binary terms [A3] | Existing external user-installed processes; native adapters added. No SDK binary redistributed. Tool version/USB backend matters; no promise that every OEM implements every command. |
| Qt / SQLite | Existing `docs/THIRD_PARTY.md` and deployed notices | Reuse current QProcess, QtConcurrent, QML and SQLite. Preserve LGPL distribution gates. |
| scrcpy | Upstream Apache-2.0 [A4] | Existing separately installed mirror launch; not a firmware, permission-escalation or full backup engine. |
| libusb | Upstream LGPL-2.1-or-later COPYING/README [A5] | Researched only; low-level USB access does not supply OEM protocols, signed loaders or Windows driver binding. Exact linking/distribution obligations need review before adding it. |
| Heimdall | Official MIT repository/license [A6] | Candidate Samsung protocol project, **not integrated**. Read its exact LICENSE and release/device coverage before any adapter; no claim of modern-model coverage or maintained signed drivers. |

First priority is trustworthy detection/target selection and observable failure, then exact-model official firmware/backup adapters. A universal commercial OEM flasher, licensed server replacement, automatic driver installation and broad manufacturer acceptance are not completed by this increment. Remaining hardware, license and security gates are in `IMPLEMENTATION.md`.

## Source links

[C1]: https://chimeratool.com/en/docs/what-is-the-use-of-the-mediatek-mtk-module
[C2]: https://chimeratool.com/en/features/device-info
[C3]: https://chimeratool.com/en/models
[C4]: https://chimeratool.com/en/download
[C5]: https://chimeratool.com/en/features/update-firmware
[C6]: https://chimeratool.com/en/features/restore-store-backup
[C7]: https://chimeratool.com/en/docs/how-many-computers-can-be-used-on-your-license
[C8]: https://chimeratool.com/en/docs/chimera-procedure-prices
[C9]: https://chimeratool.com/info/installation-instructions
[C10]: https://chimeratool.com/en/video-guide/general/advanced-update-firmware-with-chimeratool
[U1]: https://www.unlocktool.org/
[U2]: https://unlocktool.net/
[U3]: https://edl.unlocktool.net/category/oppo
[O1]: https://octoplusbox.com/en/info/how-to-activate-octoplus-tools-register-octoplus-box/
[O2]: https://octoplusbox.com/en/info/working-with-phones-based-on-qualcomm-microcontrollers/
[O3]: https://octoplusbox.com/en/features/features/
[O4]: https://octoplusbox.com/en/features/models/
[O5]: https://octoplusbox.com/en/download/drivers/
[O6]: https://octoplusbox.com/en/news/octoplus-frp-tool-v-3-1-0-is-out/
[A1]: https://android.googlesource.com/platform/packages/modules/adb/+/refs/heads/main/docs/user/adb.1.md
[A2]: https://android.googlesource.com/platform/system/core/+/refs/heads/main/fastboot/fastboot.cpp
[A3]: https://developer.android.com/tools/releases/platform-tools
[A4]: https://github.com/Genymobile/scrcpy/blob/master/LICENSE
[A5]: https://github.com/libusb/libusb/blob/master/COPYING
[A6]: https://github.com/Benjamin-Dobell/Heimdall
[A7]: https://developer.android.com/reference/android/provider/Settings
[A8]: https://source.android.com/docs/core/architecture/bootloader/locking_unlocking
[A9]: https://twrp.me/faq/ADBSideload.html
[A10]: https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/services/core/java/com/android/server/UiModeManagerService.java
