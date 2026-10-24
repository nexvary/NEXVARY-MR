# Capability evidence register — 2026-10-24

Statuses: Implemented = accessible code; Tested = accepted within the stated test
scope; Experimental = separately gated prototype; Research = insufficient
implementation/validation; Unsupported = no operation offered. These labels do
not grant commercial/model support. All exact-model hardware acceptance remains
separate. No licensed competitor binary was exercised or timed.

| Capability | Manufacturer / CPU | Model / Android build | Connection / protocol | Requirements and limits | Evidence level | NEXVARY state |
|---|---|---|---|---|---|---|
| Discovery and target selection | Protocol-generic; CPU not inferred | Synthetic identities only for current automated acceptance | ADB / Fastboot | Installed official tools; authorization/state/ambiguity checks | Native process fixtures + UI | Tested, synthetic scope |
| Explain authorization/offline states | Protocol-generic | Exact OEM/Android coverage not certified | ADB server observations | Current scan; no cable failure inferred from absence | Pure rules + process fixtures | Tested, synthetic scope |
| Windows inventory without detected phone | Windows host; CPU not inferred | No phone/model claim | Windows PnP read-only script | Windows, bounded 256-node/512-KiB schema; no installation | Adapter plus host CI pending for new commit | Implemented |
| Target-scoped PnP problem diagnosis | Protocol-generic | Synthetic exact-serial/ancestry fixtures | Windows PnP + serial correlation | Reported codes; no interface-binding or physical-cause proof | Unrelated-node and malformed-input tests | Tested, synthetic scope |
| Fastboot vs fastbootd explanation | Protocol-generic | Synthetic profile; no certified OEM set | Fastboot `getvar is-userspace` | Reviewed fresh same-target profile; unknown stays unknown | AOSP definition + rule fixtures | Tested, synthetic scope |
| VID/PID/interface-number display | Windows USB instance declarations | No model identification inferred | PnP instance ID | Not actual USB descriptor acquisition | Synthetic inventory parser tests | Tested, declarations only |
| Continuous disconnect/mode-event history | Unknown until capture designed | None | USB/ADB/Fastboot | Requires bounded chronology and physical correlation | Not implemented in this slice | Research |
| OEM-specific recovery service adapters | Samsung/Xiaomi/OPPO/vivo/realme/Motorola/Pixel/Huawei/Honor | No new approved model/build | Documented OEM paths only | Fixed command contracts, consent, compatibility and postconditions | Existing generic services only | Research |
| Device intelligence field provenance | Protocol-generic | No new model approval | getprop + health/capability probe | Existing observations; structured field confidence still to extend | Existing DeviceHealth/Compatibility | Implemented, partial |
| Firmware risk analysis | CPU/model dependent | No generic flash approval | Local ZIP/manifest/AVB | Existing metadata/hash slices; trust/downgrade model incomplete | Existing firmware tests | Implemented, partial |
| Professional Training Simulator | Virtual devices | Synthetic only | No hardware executor | Needs scenario engine, decision scoring and reports | Not yet implemented | Research |
| Protected lock/account/encryption bypass | Any | None | None | No operation offered | Out of implementation scope | Unsupported |

## Vendor evidence used for capability mapping

These are research inputs, not proof of comparable accuracy or support. Existing
Chimera/UnlockTool/Octoplus and Hydra review details and source IDs remain in the
linked engineering/UI reviews; do not overwrite them with marketing summaries.

| Vendor | Published input / source | Protocol/model detail verified here | Implication for original engineering |
|---|---|---|---|
| Chimera | [Device workflow/help](https://chimeratool.com/en/docs), existing reviewed integrated USB log article | Public workflow descriptions; current binary and timing untested | Persistent target and source evidence, then service requirements |
| Hydra | [Official documentation](https://docs.hydradongle.com/) | Chipset/brand/module taxonomy; no new exact-model tests here | Keep model references distinct from capability acceptance |
| UnlockTool | [Developer site](https://unlocktool.net/) | Prior retrieval 403; reseller lookalikes rejected as authoritative UI evidence | Research gap; no unsupported equivalence claims |
| Octoplus | [Software manual](https://octoplusbox.com/en/info/octoplus-pro-software-user-manual/) | Public connection/configuration/device/output workflow only | Separate identification, execution and verification |
| Sigma Plus | [Drivers](https://sigmakey.com/en/downloads/drivers-and-addons/), [models](https://sigmakey.com/en/supported-models/), [software](https://sigmakey.com/en/downloads/software/) | Vendor publishes mode/family driver packages and model groupings; not a per-feature NEXVARY test | Scope drivers by actual mode; never enable a function from a model name |
| UMT | [UMT Pro module reference](https://umt-pro.com/?page_id=60); [other public UMT-labelled site](https://ultimatemultitool.com/help-faq/) | Website attribution and conflicting broad server-credit claims need further confirmation; no licensed execution | Record exact per-service prerequisites; do not encode blanket claims |
| EFT Pro | [Public feature listing](https://eftprodongle.com/features) and indexed Easy-Team announcements | Listing's publisher/developer authority not independently established; not accepted as OEM specification | Quarantine unverified model/operation claims; primary-author verification remains Research |

A full manufacturer/SoC/model/build/protocol/requirement row for each competitor
operation is still incomplete. Extend the machine-readable reference catalog only
when a primary source supplies those fields; leave absent values unknown. No
commercial executable, artwork, firmware, driver or proprietary implementation
was copied or bundled. Future fair comparisons require licensed tools, identical
authorized hosts/devices/builds/tasks, measured correctness/latency and failures.
