# Recovery laboratory validation protocol

Protocol version 1, 2026-10-16. This is a reproducible test specification, not a record that hardware tests have run. The source of truth for device acceptance is [device-matrix.json](device-matrix.json). All seven device entries currently remain `not_run` because hardware is unavailable.

## 1. Laboratory scope and evidence handling

Use original synthetic photos, short videos, text, contacts and messages generated for this laboratory, or explicitly licensed public fixtures. Use sacrificial devices and storage owned/authorized for destructive experiments; customer devices and live customer media are outside this protocol. Record the operator, purpose and informed consent in the case metadata. Do not collect unrelated accounts, private credentials or personal communications.

Select a separate output directory. For image-only work, open a regular source file read-only, reject source/output path collisions, and hash the source before and after. For any future recovery directly from removable media, use a different physical destination. Never save results back onto the original media being recovered.

Keep the original acquisition immutable; work from verified copies. Record hashes and provenance separately from explanatory notes. A failed/partial/cancelled run keeps its failure state and does not overwrite a previous result. Retention and disposal cover fixtures, images, exports, logs and backups.

## 2. Record the exact environment

For every executed scenario record:

- MR commit/build hash; Qt version; Windows edition/build/architecture; relevant USB driver; external executable version, SHA256, source URL and license if used.
- Exact device manufacturer/model, Android/build fingerprint, SDK, security patch, original launch Android version when verified, storage medium/type, filesystem and encryption state.
- Bootloader/root state **as found**, existing ADB authorization, Android user/profile, unlock state, granted SAF tree/document scope and whether the acquisition is logical or physical.
- Fixture inventory with content SHA256, sizes, names and known placement; exact operation, UTC timestamps, source/output hashes, permission/error states and evidence paths.

Unknown values stay null with a reason. Do not infer filesystem, encryption, launch version or support from a manufacturer/model marketing name. Mask device serials and sample account identifiers in exported reports.

## 3. Gate A — image-only native correctness

Before external workers or physical devices are added, test the original C++ parser/carver using regular synthetic image files:

| Case | Expected evidence |
|---|---|
| Intact PNG and JPEG at known offsets with deterministic filler | Exact fixture hash match; reported offset and validated length; source digest unchanged |
| Signatures crossing input chunk boundaries | Same bytes/results as non-boundary placement; no omission or duplicate candidate |
| Truncated/corrupt header, invalid lengths/CRC/chunks and missing terminators | Rejection or explicit partial/corrupt classification; no claim of complete recovery |
| Fragmented content and false signatures inside other content | Explicit limitations and validation status; no invented filename or silent concatenation |
| All-zero/TRIM-like image and random deterministic filler | No fabricated valid recoveries; candidate false positives remain identifiable |
| Very large declared length, deeply nested structures and repeated signatures | Bounded memory, candidate/output limits and understandable failure/cancellation |
| Output full, permission denied, Unicode/long path and source/output collision | Predictable error, no source modification, no GUI freeze or success report |
| Cancel mid-scan and close while busy | Work stops predictably; partial files/status are clear; repeat run remains possible |

Success on synthetic contiguous images is not evidence of recovery from Android encrypted or discarded flash blocks. Native recovery tests must remain separate from device-connection UI tests.

## 4. Gate B — independent external-engine comparison

Only after the image-only source/outputs are validated, evaluate an official pinned external engine **outside MR**. Record binary and source archive digests, license and actual argv. No shell interpolation, physical-device path or elevated access is needed for a regular-image comparison. Use a dedicated scratch working directory and bounded runtime.

For PhotoRec, use the officially documented `/d` output and `/cmd` image command list, restricting file options to the fixture types. Verify output directory suffix behavior, process exit semantics, logging, cancellation and repeat invocation. Do not label it integrated until a native Qt adapter, Windows packaging/license review and corresponding integration/failure tests exist.

Compare exact recovered hashes and source before/after hashes. Treat disagreement between engines as a finding to investigate rather than declaring one correct by name or reputation. Validate byte offsets and output lengths against the original fixture manifest.

## 5. Gate C — consensual logical Android export

On a sacrificial already accessible test phone, first record metadata and authorized ADB status. If an optional AndroidAgent is tested, grant SAF access to a dedicated fixture folder only and record the granted scope. Denied/revoked permission, a locked profile, cable removal, app termination, storage full and duplicate filenames must produce honest partial/error states.

Compare exported files against the fixture inventory. Record any differences caused by app export/transcoding or provider behavior. This gate proves visible-file copying only; it does not prove access to app-private databases, unused storage blocks, deleted files or the raw userdata partition.

Do not unlock a bootloader, install root, flash recovery, bypass device locks, trigger EDL acquisition or search PINs/credentials. Pre-existing root, if encountered, is an environment property; it does not itself prove access to plaintext deleted FBE data.

## 6. Gate D — removable-media recovery

Use a dedicated laboratory SD/USB medium containing synthetic fixtures. Distinguish ordinary portable storage from Android adoptable/encrypted storage. Make a verified image, perform a controlled deletion on the test medium, and analyze the image without changing the source. Repeat with a controlled filesystem reformat only on sacrificial media, with the exact route and timestamps recorded.

Record which bytes remain, whether original filesystem metadata survives, and whether the result is a filesystem export or carved candidate. Do not extrapolate SD-card behavior to internal encrypted Android flash. Never choose customer media to fill a missing matrix entry.

## 7. Gate E — factory-reset research only

This gate is destructive and is separate from routine recovery. It requires a sacrificial device, the original synthetic corpus, a verified pre-reset export and a documented vendor reset route. Reset is owner/operator controlled, not an automatic action in MR. Record the exact reset UI/route, firmware/build, encryption/key state when legitimately observable and UTC time.

Evaluate the legitimate post-reset access that actually exists. If no authorized raw image/plaintext path exists, mark physical/post-reset recovery **blocked with that reason**; do not manufacture an image or weaken the phone. Compare only evidence actually acquired. Record zero confirmed recoveries separately from a scan that could not run.

Android FBE keys, discard/TRIM and reset behavior vary by device/version. Bootloader unlocking is documented to perform a factory reset and must never be used to prepare the device for this data-preservation experiment. No successful post-reset recovery claim is accepted without exact content hashes, acquisition provenance and reproducible evidence on the named device/build.

## 8. Gate F — vault and data lifecycle

Validate the implemented encrypted vault separately: encrypted bytes at rest, correct user unlock, failure on corrupted/truncated records, atomic writes, no plaintext remnants from failed saves, and predictable access under another Windows account/machine. Describe the exact key scope and restore/export policy; do not imply DPAPI ciphertext is portable across accounts or computers without a tested explicit transfer design.

Exercise application restart, crash recovery, insufficient disk space, permission denial and backup restoration using synthetic evidence only. Inspect logs for plaintext credentials and private content. Vault encryption does not establish source authenticity or make an acquired Android ciphertext image decryptable.

## 9. Acceptance and reporting

A device/cohort passes only the scenarios actually executed on the exact listed model/build. Use `not_run` for unavailable hardware, `blocked` when a prerequisite prevented the run, `failed` when an attempted criterion failed, and `not_applicable` only with a recorded reason. Never translate an unknown/null field into false, zero or passed.

Publish a concise result: what was acquired, which engine ran, confirmed hash-matched outputs, rejected/partial candidates, unchanged-source check, failures, unsupported conditions and the remaining gates. Preserve exact MR and upstream versions for reproduction. Broaden support only after independently repeated results; one OEM example is not coverage of every phone in that family.
