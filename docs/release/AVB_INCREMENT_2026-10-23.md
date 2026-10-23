# AVB research-to-capability increment — 2026-10-23

Branch: `integration/gate-1-stages-08-60`; PR: https://github.com/nexvary/NEXVARY-MR/pull/8. Starting remote HEAD rechecked: `26752a707714fddd03f6966ce60f1a257577e473`. Use the commit containing this document as the implementation revision; PR #8 records its exact SHA and remote Actions runs. No main merge, release tag or RC approval.

## Native changes

- Original bounded AVB metadata parser, integrated in existing FirmwareBridge rather than a second transport or application. No child process, network access or phone write is involved.
- Async read-only local inspection/cancellation; exact 64-bit metadata, SHA-256 and JSON export, explicit false trust/compatibility flags, source-overwrite rejection.
- AR/EN Firmware Lab tab, scrollable plain-text report and deployed synthetic screenshots at 1180×760.
- Fixed the existing diagnosis TextArea/ScrollView geometry loop exposed by full smoke. The first regression run correctly failed on a QML warning; final production/Laboratory smoke passed without suppressing the warning gate.
- First source/technology/capability/evidence registry. Device, chipset and firmware approvals remain empty. Repository integrity checks reject dangling references and synthetic-to-physical promotion without scoped evidence. This registry is not an authorization engine.
- Public competitor/source review and integration selection updated, including inaccessible primary-source gaps and independent report index versus reviewed results. No commercial product was run or reverse engineered.

## Actual validation

Environment: isolated Linux `/tmp` builds, Qt 6.4.2, GCC 13, Release, warnings as errors. This is not native Windows or Android acceptance.

| Check | Actual result |
| --- | --- |
| Production CTest | **19/19 groups passed**, 70.32 s |
| Laboratory CTest | **19/19 groups passed**, 73.25 s |
| AVB Qt cases | **19 passed**, zero failed/skipped per profile, including initialization/cleanup |
| AVB deterministic mutations | **30,000 cases** per executable run; no allowed trust promotion |
| QML UI cases | **26 passed**, zero failed/skipped per profile |
| Production service Qt cases | 24 passed, 21 intentional lab-only skips, zero failed |
| Laboratory service Qt cases | 45 passed, zero failed/skipped |
| Real external Linux tools | `MR_TSK_MMLS_PATH=/usr/bin/mmls`, `MR_CLAMSCAN_PATH=/usr/bin/clamscan`; TSK 8/8 and security 14/14 cases passed without skips |
| Python core/release/knowledge | 14 passed |
| Python recovery | 19 passed |
| Python portal | 6 passed |
| AddressSanitizer + UBSan parser mutation target | Passed, 0.83 s, `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1` |
| LeakSanitizer | **Unavailable**: runtime fatal error because ptrace/thread inspection is unsupported in this environment. Original failed log retained; no memory-leak success claimed |
| Actual screenshots | Production Qt render, AR/EN dark, synthetic vbmeta only; no physical device |

CTest's credential-dependent real-server cloud case remains skipped by default; the prior verified Caddy/Gunicorn integration on the previous code checkpoint is separately recorded in `LOCAL_VALIDATION_2026-10-23.md`. It was not silently counted as current phone/cloud acceptance. Local Python portal tests above did run again.

Raw logs, JUnit XML and screenshots: [`evidence/avb-2026-10-23/`](evidence/avb-2026-10-23/). Tests ran against the same C++/QML inputs uploaded in this increment. Additional documentation/registry integrity edits do not change the compiled parser.

## Stage effects and remaining gates

This extends the research track and firmware diagnostics within stages 19–24, adds hostile-input regressions within 13–18, and validates UI work within 49–51. It does not complete entire groups, approve an OEM model or advance 60 to complete. Stage status in `DEVELOPMENT_CHECKPOINT_2026-10-23.md` still applies.

Unresolved: admitted exact-head GitHub runners/builds; clean Windows 10/11, installer/portable/uninstall; physical Android identity/read/USB-disconnect/postcondition acceptance; signing; full transitive SBOM/license review; upgrade/rollback; privacy and independent review. No downloadable Windows artifact is asserted in this document.

## Resume

1. Fetch integration HEAD and PR #8, then inspect new Actions run jobs/steps/artifacts. Previous runs failed before checkout with empty runner/step data; root cause unproven.
2. Keep the existing compiled default-deny policy and Laboratory separation. AVB parsing must never enable a service command.
3. Expand firmware research with stable OEM samples/rights and pinned official-tool comparison; add signature verification only with explicit OEM trust and partition checks.
4. Independent available work: scoped offline PCAP/authorized-backup adapter PoCs and broader CRM/privacy acceptance fixtures, subject to license/input-isolation checks. Do not label detected executables as integrated capabilities.
5. Continue physical and Windows gates when their environments are available. Do not publish a final/RC build based on fixture success.
