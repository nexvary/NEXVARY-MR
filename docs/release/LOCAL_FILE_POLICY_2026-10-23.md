# Shared local-file policy follow-up — 2026-10-23

Branch: `integration/gate-1-stages-08-60`; PR #8. Starting integration commit: `633119b341d3be1713899a730b93e9c47516fa67` (offline PCAP). The commit containing this document is the local-file follow-up; PR #8 records its exact SHA and Actions state.

## Change

FirmwareBridge's manifest, package and AVB reads previously used QFile directly and had separate path handling. They now reuse the forensic engine's regular-file open helper and a common localFilePath function. RecoveryBridge uses that same path policy, eliminating the duplicate normalization rules.

The path policy rejects explicit UNC/network file URLs, non-file schemes and control characters, including NUL/newline introduced by file-URL percent decoding. Existing Windows reserved-name/alternate-stream checks are preserved in the shared helper. Firmware manifest/package symbolic links are rejected before work starts; AVB already rejected a selected symbolic link. Workers verify the opened input is regular, rather than relying only on earlier pathname metadata. AVB report destination paths use the same local-path policy.

On POSIX the existing descriptor helper uses no-follow/nonblocking open flags plus fstat, refusing symlinks and FIFO/device descriptors without waiting for a FIFO writer. Windows retains its existing QFile-open/fstat implementation: this follow-up does not claim equivalent no-follow reparse/race protection on Windows. Native Windows handle/reparse/identity acceptance remains required. The policy cannot identify every OS-mounted network filesystem or stop all same-size concurrent changes. Use stable working copies; this is not an OS sandbox.

No phone commands, permissions, transport policy or production-write allowance were added. Firmware metadata/hash matching remains distinct from authenticity and model compatibility.

## Executed evidence

Final fresh warnings-as-errors Linux builds and full CTest after the shared-header/native changes:

| Check | Result |
| --- | --- |
| Production CTest | **21/21 passed**, 79.72 s |
| Laboratory CTest | **21/21 passed**, 86.59 s |
| Firmware Qt cases | **10 passed**, zero failed/skipped per profile, including init/cleanup |
| AVB/PCAP/case/UI regression | Included in those full runs; AVB 20, PCAP 16 and UI 27 cases per profile |
| New local-file cases | Scheme/UNC/control and decoded-control rejection, ordinary local/file-URL compatibility, native directory/symlink/FIFO descriptor refusal, direct Firmware/AVB entry rejection |

Actual symlink/FIFO descriptor cases ran on Linux. Windows reserved-name/stream assertions are compiled conditionally and were not run on Windows here. Current full suites retain the disclosed skips from `PCAP_INCREMENT_2026-10-23.md`: credential-dependent real-server TLS, separately installed real mmls/ClamAV, and production-only skips of Laboratory write fixtures. These are not physical-device success claims.

Python's 39 tests and PCAP AddressSanitizer/UBSan evidence belong to the preceding PCAP increment in the same session; Python and parser bodies did not change in this follow-up, and those results are not described as rerun here. Raw current XML/text: [`evidence/file-policy-2026-10-23/`](evidence/file-policy-2026-10-23/).

## Remote state and next gate

At `633119b`, native Actions `39893471803` and Python `39893471795` / `39893467502` failed with empty runner names and zero steps; one core job was cancelled. Root cause unproven. Saved observation: `evidence/pcap-2026-10-23/remote-ci-633119b.json`. Inspect this follow-up's Actions before resuming; PR #8 records those latest links.

Keep PR draft and main stable. Stage 60/RC remain blocked by exact-head CI, clean Windows 10/11 and package lifecycle acceptance, physical Android/model/USB evidence, signing, dependency/SBOM/license review, upgrade/rollback, privacy and independent review. No Windows download is asserted.

Next independent engineering work: Windows regular-handle/reparse review and fixtures, then rights-cleared independent PCAP corpus comparison. Fetch integration HEAD first; do not repeat completed AVB/PCAP implementations or convert synthetic-only support records into device approvals. No background execution is scheduled by this checkpoint.
