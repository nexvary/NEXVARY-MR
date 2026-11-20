# Local firmware integrity inspection

This feature reads a local JSON manifest and a local package. It performs no download, flashing, bootloader unlocking, or device modification.

**An untrusted manifest cannot prove firmware authenticity.** A matching hash only means that the supplied bytes match the supplied digest. An attacker can replace both. Obtain the firmware and its digest from an independently authenticated official source; exact model matching alone does not establish flashing compatibility, signing keys, partition layout, anti-rollback level, carrier variant, or data safety. `compatible` reports only exact declared brand/model equality and a matching local SHA256. It never authorizes flashing.

## Schema version 1

| Field | Rule |
| --- | --- |
| `schemaVersion` | JSON number exactly 1 |
| `brand` | Nonempty explicit brand, maximum 128 characters; must exactly equal the device value provided by the caller |
| `model` | Nonempty explicit model, maximum 128 characters; must exactly equal the device value provided by the caller |
| `build` | Nonempty firmware build identifier, maximum 256 characters; informational, not inferred compatibility |
| `sourceUrl` | HTTPS URL, maximum 2048 characters, with hostname and without user credentials or fragment; recorded provenance, not verified by this feature |
| `sha256` | Exactly 64 hexadecimal characters; compared against the local package digest |

The manifest is limited to 64 KiB. Brand/model/build reject control characters and surrounding whitespace. Case, spaces inside names, and model suffixes must match literally; the validator does not infer aliases or device families. Unknown actual identity, a missing or empty package, invalid metadata, mismatched hash, cancellation, read failure, or packages larger than 64 GiB keep `compatible` false.

The application must supply explicit identity from the selected device's `ro.product.brand` and `ro.product.model`. Do not silently substitute manufacturer for brand or manually entered guesses. The public API accepts strings so it can be tested independently; the caller remains responsible for binding them to the selected authorized device and calling `invalidate()` when selection, device properties, manifest path, or package path changes. Invalidation immediately closes the gate and cancels any in-flight hash, preventing a late result for an earlier device from opening the gate.

Hashing runs in a QtConcurrent worker with 1 MiB reads, progress, and cancellation checks between chunks. Only metadata validation (at most 64 KiB) runs on the UI thread. The operation does not execute package contents or interpret archives. No archive extraction or format-specific safety checks are claimed.

`samples/training-firmware.json` is a deliberately invalid-for-real-use training placeholder. It contains no genuine firmware, device credentials, or vendor endorsement; its all-zero hash must not be treated as a trusted reference.
