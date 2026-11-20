# Android recovery research — evidence snapshot 2026-10-16

## Scope and evidence standard

This review distinguishes: copying currently readable files; undeleting files from surviving filesystem data; interpreting forensic artifacts; recovering pre-reset internal data; and restoring an existing backup. None of these is evidence of the others. No phone was reset, rooted, unlocked, or acquired during this research. No physical Android or Shizuku permission experiment was performed. Product assessments are conditional triage, never numerical success probabilities.

## Research questions and findings

| Question | Supported finding and limits |
|---|---|
| 1. What happens to encryption keys? | FDE protects the userdata volume with a master key; FBE provides separate CE/DE keys and additional wrapping. AOSP recovery formats data and handles encryption metadata. Secure destruction of necessary key material can render surviving ciphertext unusable. Verify the exact OEM build/reset route; do not infer recoverability merely from nonzero sectors. [S1,S2,S3,S4] |
| 2. Do vendors differ? | Recovery builds, storage firmware, reset paths and key-management capabilities differ. Lang's source review covers Android5–9, LineageOS, OxygenOS and KaiOS, not every present OEM. It identifies changed discard behavior, not a demonstrated general decryption method on modern phones. Each exact build needs measurement. [S5] |
| 3. FDE vs FBE? | FBE appears from Android7 and is mandatory on devices launched with Android10+. Older devices upgraded to10–12 may retain FDE; AOSP removes FDE support in13. CE requires user unlock while DE supports Direct Boot. A directory becoming available after boot is not proof that old reset keys survived. [S1,S2] |
| 4. Documented post-reset recovery without root? | Simon/Anderson2015 tested21 phones, five vendors, Android2.3–4.3 and found reset shortcomings. That historical access/collection experiment is not evidence for a normal unprivileged Shizuku app on modern encrypted hardware. This review did not verify a reproducible modern stock, no-root, lost-key internal-photo recovery experiment. This is an evidence gap, not proof that every old device behaves identically. [S6] |
| 5. Partitions outside reset? | OEM-specific persistent partitions, removable media, linked computers and backups require separate inventories. Their mere persistence does not establish that personal images/documents exist there. Do not scrape protected partitions or treat FRP/persistent identifiers as customer media recovery. AOSP format logic is partition-specific. [S4] |
| 6. F2FS remnants? | Oh/Hwang2025 recover deleted data via metadata and reconstructed virtual NAT. Their experiment uses an F2FS-formatted4GB USB on Ubuntu, synthetic text files, immediate unmount and acquired image. It does not demonstrate Android factory-reset recovery or defeat fscrypt. Garbage collection, discard and overwrite remain constraints. [S7] |
| 7. When can remaining blocks be interpreted? | Only when acquisition exposes the relevant bytes, the format/metadata survive and required cryptographic material is available through authorized means. Metadata encryption additionally hides structures not protected by FBE. UFS/eMMC describe managed flash interfaces; chip-off does not itself provide a plaintext filesystem or lost keys. [S3,S5] |
| 8. External storage? | Portable SD storage may contain independently readable files; determine whether that exact reset erased it. Adopted/internal storage differs and may depend on device keys. Android7–8.1 FBE does not support adoption;9+ does. Do not reformat or re-adopt a card to inspect it. [S1,S8] |
| 9. Shizuku access? | The official API runs app code with the backend UID: ADB shell2000 or root0. SELinux, filesystem permissions and available services still constrain it. An ADB-started backend is not root. This prototype will refuse root mode, use fixed read-only metadata probes and document denials. No arbitrary shell, credential/key extraction or protected-partition acquisition. [S9] |
| 10. Official non-modifying collection? | Public Android APIs expose app/device metadata; SAF provides user-selected readable documents. Authorized ADB can provide fixed logical properties/logs, not universal private/raw access. Running Android, enabling debugging, installing software and exporting files can change state; describe logical collection as bounded reads, never forensic zero-change acquisition. Prefer existing offline images before installing an agent. [S9,S10] |
| 11. Backups and linked devices? | Google/OEM/app backups, synced photos, original SD cards and previously linked computers can contain independent surviving copies. Restore from backup is a separate outcome. Use official owner-authenticated interfaces; no cloud tokens or credentials collected. Google states backup contents/settings vary and version compatibility applies. [S11] |
| 12. Model/version differences? | Record manufacturer/model/build fingerprint/API, original launch generation when independently known, security patch, filesystem, FDE/FBE/metadata evidence, reset route/time, subsequent writes, SD mode and backup inventory. Never infer launch encryption solely from current OS version or brand. Unknown values remain unknown until documented. [S1,S2,S4] |

## Recent research that changes the assessment

**LEMON2026** provides an eBPF memory-acquisition design for modern GKI Android and Linux. Its real-phone experiments use Samsung GalaxyA15 and Pixel6a on Android14 with rooted access. Practical execution requires root or sufficient kernel capabilities; standard shell permission is insufficient. This concerns volatile memory of the running system, not the pre-reset RAM of a device that rebooted. No LEMON/root/exploit integration is added here. [S12]

**Memory production-build studies2025/2026** examine ART layouts and symbol loss. The2026 preprint validates its live approach on a rooted Pixel7. These findings are useful research context, not independent acceptance of a no-root recovery product or post-reset decryption. [S13,S14]

## Reproducible laboratory protocol

1. Use a dedicated owned/explicitly authorized test phone; document exact consent, owner, asset ID and destructive-test authorization separately from ordinary evidence collection.
2. Create an original synthetic corpus: PNG/JPEG, short video, UTF-8 documents and known random controls. Record every SHA256, size, creation method and backup membership on a separate workstation before deployment.
3. Record build/patch, storage/encryption evidence, unlock state, launch OS if verified, reset method, SD portable/adopted state, app versions and baseline visible files. Preserve unknowns.
4. First inventory existing backups/offline images without installing anything. Record whether agent/Shizuku installation is justified; create paired no-agent/agent trials when hardware permits. Installation is a write-impact decision, not a prerequisite.
5. A human performs a separately approved reset on the dedicated phone. No product code automates it. Record timestamps and every subsequent boot/unlock/write.
6. Attempt only authorized logical/public SDK/SAF/ADB-shell probes; log denied access as a result. Do not change bootloader state, root or collect keys/credentials.
7. Analyze a working copy of any lawfully obtained image. Hash source before/after, preserve originals separately, record offsets and method. Ciphertext-only findings are not recovered plaintext.
8. Validate candidate files by complete format decode and exact original SHA256. Count partial/false positives and unmatched files, distinguish backups from undeleted files, retain negative results.
9. Publish anonymized synthetic corpus, manifests, tool binary/source versions, exact fixed arguments, acquisition conditions, raw test output and limitations. Do not publish customer evidence.

Device matrix rows must remain `not_run` until a real experiment supplies evidence. Required families: Samsung, Xiaomi, OPPO, realme, Pixel, pre-FBE legacy and modern FBE; multiple builds/reset routes are separate trials. No success percentages can be extrapolated from family labels.

## Primary sources

- S1 [AOSP FBE](https://source.android.com/docs/security/features/encryption/file-based).
- S2 [AOSP FDE](https://source.android.com/docs/security/features/encryption/full-disk).
- S3 [AOSP metadata encryption](https://source.android.com/docs/security/features/encryption/metadata).
- S4 [AOSP recovery format implementation](https://android.googlesource.com/platform/bootable/recovery/+/refs/heads/main/recovery_utils/roots.cpp). Moving main is a review snapshot, not an exact OEM reproduction; pin the target device source/build for experiments.
- S5 [Lang2022 source analysis](https://appsec.at/publication/lang-2022-analysis/lang-2022-analysis.pdf).
- S6 [Simon/Anderson2015](https://www.cl.cam.ac.uk/archive/rja14/Papers/fr_most15.pdf).
- S7 [Oh/Hwang2025 F2FS experiment](https://dfrws.org/wp-content/uploads/2025/10/Advanced-forensic-recovery-of-delete_2025_Forensic-Science-International-Di.pdf), DOI10.1016/j.fsidi.2025.301976.
- S8 [Google SD modes](https://support.google.com/android/answer/12153449).
- S9 [Shizuku API official developer guide](https://github.com/RikkaApps/Shizuku-API).
- S10 [Android SAF](https://developer.android.com/training/data-storage/shared/documents-files).
- S11 [Google backup/restore](https://support.google.com/android/answer/2819582).
- S12 [LEMON2026 paper](https://dfrws.org/wp-content/uploads/2026/03/LEMON-A-universal-eBPF-based-volatile-memory-acq_2026_Forensic-Science-Inte.pdf), DOI10.1016/j.fsidi.2026.302045; [official code](https://github.com/eurecom-s3/lemon).
- S13 [2025 ART preprint](https://arxiv.org/abs/2512.18517).
- S14 [2026 production-build preprint](https://arxiv.org/abs/2607.13821).

No customer data, actual acquisition, factory-reset experiment or successful hardware recovery is reported by this document.
