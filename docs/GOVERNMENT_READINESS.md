# Government Edition evaluation readiness

This is a development increment for controlled evaluation, not a completed institutional deployment. Latest fully hosted-verified source/test checkpoint: `ef86c995ebf93bcf47e0d25ff7e8353cb117046e`, CI 40229014198: all seven jobs passed; four native profiles 43/43 each; production ASan/UBSan 14/14; Windows Qt Quick 48/48, imports 36/36 and independent populated ADB interoperability 52/52. Portable/installed app and new ADB worker provenance, install/uninstall and downloaded package checksums passed. Actual final Windows AR/EN captures were reviewed. Local default LeakSanitizer environment failure and its limited rerun remain explicit in DEVELOPMENT_STATE.md; full hosted checks passed without disabling leak inspection. Earlier checkpoint results remain historical.

Review PR: https://github.com/nexvary/NEXVARY-MR/pull/15. The [acceptance matrix](GOVERNMENT_ACCEPTANCE_MATRIX.md), [competitive matrix](COMPETITIVE_CAPABILITY_MATRIX.md) and [superiority roadmap](COMPETITIVE_SUPERIORITY_ROADMAP.md) define remaining institutional/comparison gates. No field acceptance, accreditation or comparative superiority is established.

| Area | Implemented and tested | Practical boundary / remaining work |
|---|---|---|
| Government interface | Default 12-section AR/EN workspace; separate repair navigation; internal Back; real licensed icons; responsive/DPI regression | Real Qt captures are development fixtures, not real case data. |
| Cases | Create/search/reopen/edit, authority/agency/court/prosecutor fields, devices, closure/archive | Native retention/hold/archive/disposal-review governance added; document-specific attachment classification remains. |
| Evidence | Protected imported copy, SHA-256, case/device association, manifest, verified working-copy export | 16 MiB per source, bounded case/export count; explicit exports are unencrypted and inherit destination security context. |
| Custody and audit | Matching-custodian transfer, hash-linked protected audit, local signed anchors and rollback-prefix comparison | Recorded handovers are declarations. External time/key custody and physical handover proof are not established. |
| Identity and RBAC | Local Argon2id credentials, password-wrapped signing keys, native role/action checks, ownership/grants, revocation, lockout and session expiry | Local accounts are not institutional identity/SSO or proof of independent people. |
| Protected storage | Windows current-user DPAPI payloads, bounded SQLite transactions, stale-writer rejection | Bounded protected case backup/restore now validates original ownership, every evidence blob and a separately supplied digest; native retention governance now persists within snapshots; physical disposal execution remains. Linux real-case storage fails closed. |
| Device assessment | Selected-target manufacturer/model, SoC/hardware, firmware, OS/patch, boot/encryption observations with source/time and unknown states | Live USB observations are not automatically attributed to a seized case device. No protected-phone acquisition, key extraction or iOS capability inferred. |
| Case analysis | Existing classic-PCAP and JPEG-EXIF parsers on hash-verified case bytes; repeatable report hashes; protected run provenance | 8 MiB sources. Seven bounded authorized-import adapters include standalone SQLite samples, exported logs and tested unencrypted/uncompressed ADB v5 member inventory (52 independent fixture checks; see DEVELOPMENT_STATE.md); broader vendor backup/container/app decoding, full artifact indexing and entity relationships remain. New timestamp projection status is tracked separately in CASE_TIMELINE.md and DEVELOPMENT_STATE.md. |
| Case timestamp timeline | Original asynchronous projection of retained import timestamp literals; explicit UTC offsets, unresolved states, source/report provenance, literal search and pagination | Tested at the checkpoint above; independent laboratory/field acceptance remains pending. No full activity semantics, numeric epoch inference, social graph or timeline export is claimed. |
| Research | Existing Atlas/tool catalog, experiment intake, public/synthetic author/reviewer signatures and evidence verification | Research approval does not grant commercial/device support; exact model/OS/firmware repeatability remains required. |
| Training | Existing three synthetic scored decision scenarios and separate training reports | Not a complete VM/device-simulation/data-set laboratory or accredited training course. |
| PDF reports | Case/evidence/custody/analysis/export/backup/recovery/audit tables, Arabic labels, LTR technical results/digests, repeated table captions | Native synthetic five-page Linux report visually/textually checked, including exact ASCII `ff` in a digest; independent Windows 10/11 field review remains. |
| Windows delivery | Portable/installer builds and hosted isolation/install/run/uninstall checks | Development packages are unsigned. Hosted Windows Server validation is not clean Windows 10/11 acceptance. |
| Deployment | Local case/analysis workflows do not need Guardian or a cloud service | Policy-enforced air-gap mode, institutional backup/recovery/privacy/license review, trusted publisher signing and independent release approval remain. |

## Next executable slice

The protected case snapshot/restore slice is implemented; see PROTECTED_CASE_BACKUP.md
for bounds, current-account recovery and retained-anchor semantics. It is not full
OS/identity-registry disaster recovery.

Retention/archive/disposal-review governance is implemented with no byte deletion; see
EVIDENCE_RETENTION.md. Timeline projection is tested at the pinned checkpoint above. The bounded ADB inventory passed hosted verification. Next integrate typed artifact/provenance paths through the existing engines. Preserve the
legacy forensic feature freeze: case integration is not authorization to duplicate
or expand its unrelated parser backlog.

## Evidence discipline

Keep native CI, synthetic fixtures, physical owned-phone acceptance and independent
institutional review separate. Evidence under `evidence/government-2026-10-25/`
retains the first failing packaged resource check, its passing corrections, intentional
local external/Laboratory skips, public-digest false-positive review and actual Qt
screenshots. A green build does not prove legal authority, device compatibility,
physical custody, cryptographic bypass or final institutional readiness.
