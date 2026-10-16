# Data recovery workspace — development 0.4.0

The ninth workspace retains Source assessment and adds Cases & local analysis with eight steps. No success percentage is shown. First assess the source and write impact; select a pre-acquired raw image, a currently readable local file or an offline Agent report. Record exact device facts/unknowns, loss type and documented owner consent. Create a local case, explicitly authorize this source, run, review candidates/audit, then confirm plaintext exports separately.

## Implemented boundaries

- Native C++20 PNG/JPEG contiguous carving from regular .dd/.img/.raw/.bin files, up to2GiB,8MiB/candidate,100 candidates and64MiB total. Complete image decoding, dimensions<=4096x4096 and format checks are required. No PDF/ZIP reconstruction, fragmented-file reconstruction, EWF input, F2FS parser, decryption, raw-device handles or protected Android partition acquisition is implemented. The Python signature-report research helper retains broader signature matching; its matches are offsets only and are not the native decoded-file carver.
- Image SHA256 is checked before and after three-pass analysis; errors, changes and cancellation discard candidates. Resource-limited successful analysis is explicitly recorded as limited. Reading a mounted filesystem can update OS access metadata; no forensic write-blocker or universal zero-change claim is made.
- Logical import copies an explicitly selected existing file, up to8MiB, after two matching reads. It does not decompress or restore a full backup. Agent JSON import is bounded, versioned and untrusted metadata; no phone software is installed automatically.
- Embedded original evidence bytes, SHA256, offsets/source digest, device facts, consent and audit are persisted in a case container. Cases are separate from the workshop SQLite database. Sources stay outside the container and are never overwritten by exports.

## Encryption and privacy

Production case containers use Windows current-user DPAPI with integrity protection. They normally require the same Windows account and machine for decryption. Protect the account and storage; plan managed backup/account recovery. A copied container alone does not guarantee portability. This is not password-based cross-platform archive encryption or protection against an attacker controlling the logged-in account.

Non-Windows production case creation is disabled. Automated Linux tests use an explicitly labeled unencrypted synthetic profile; it is never advertised as encrypted or used as a customer fallback. Plaintext is present in working memory during analysis. Secure memory/media erasure is not claimed.

Exports require a separate explicit plaintext confirmation and a new destination filename; reports omit payload bytes but include sensitive identity/source metadata. Protect all exports. Case deletion removes only the selected container, never sources or exported copies, and does not claim physical sanitization. Existing SQLite customer data/backups remain unencrypted and require protected local storage.

## What tests can establish

Synthetic tests exercise hashes/offsets, EXIF/JFIF decoding, bounds, changed input, malformed/truncated files, cancellation, vault tamper protection and consent/output safety. Windows runtime smoke exercises the deployed JPEG decoder, image scanning and encrypted case persistence on original synthetic PNG/JPEG bytes. This does not establish a real phone's recoverability, historical file deletion, ownership, or pre-reset origin.

No real phone Factory Reset, real Shizuku session or internal lost-key recovery experiment has been performed. Read `research/android-recovery/FACTORY_RESET_RESEARCH.md` and `TOOLS.md` for sourced research, and `device-matrix.json` for the deliberately unexecuted hardware matrix. PhotoRec was tested independently on Linux with original synthetic fixtures; it is not bundled or invoked by the desktop.
