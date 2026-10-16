# Android recovery laboratory

This directory separates recovery engineering evidence from the product's existing Windows/Qt acceptance gates. Read [TOOLS.md](TOOLS.md) for upstream source/license findings, [LAB_PROTOCOL.md](LAB_PROTOCOL.md) for laboratory procedures, and [device-matrix.json](device-matrix.json) for the hardware status.

## Integration boundaries

| Layer | Scope | Status/evidence boundary |
|---|---|---|
| NEXVARY-MR core | Native C++20/Qt desktop, existing device diagnostics, workshop and boardview | Existing CI/release evidence applies only to the revision and functions actually tested. Recovery changes require their own tests. |
| Native image recovery | Original PNG/JPEG carver for user-selected regular image files | Under development in this branch; individual test/CI evidence determines readiness. A signature match is a candidate until structurally validated. No promise of original name, directory, complete fragmented file or encrypted content. |
| Evidence vault | Local encrypted case/evidence storage using Windows protection | Under development. Do not claim encryption coverage, key portability, or cross-user/cross-machine restore until exact implementation and acceptance tests prove them. |
| Optional AndroidAgent | Metadata and owner-granted Android Storage Access Framework (SAF) export protocol | Optional design/workstream. No app-private access, raw-flash access, bootloader/root modification or universal deleted-file recovery is implied by an Android permission or USB connection. |
| External tools | PhotoRec, TSK, libewf, ALEAPP/iLEAPP and the alternatives in TOOLS.md | Researched; no external recovery worker is integrated, bundled, auto-downloaded or launched by MR. A standalone synthetic experiment is an engine evaluation, not an adapter acceptance test. |
| Live-device acquisition | Consensual logical export only within available permissions; existing lawful images may be analyzed | Physical Android access, modern FBE keys and OEM-specific behavior are unresolved until a specific supported path is demonstrated. No acquisition or security-bypass promise. |
| Factory-reset research | Isolated sacrificial-device experiment using original synthetic data | Hardware tests have not run. Reset is destructive and cannot be treated as a recovery preparation step. Refer to the laboratory protocol and the separate factory-reset research when available. |

## Product wording

Use precise descriptions: **copy accessible files**, **analyze a supplied image**, **carve a validated candidate**, **parse an artifact**, or **restore an existing backup**. These operations have different prerequisites. Do not describe logical ADB/SAF access as a physical dump or deleted-block recovery.

For supported encrypted backups, decryption requires legitimately supplied credentials/keys and a tested format. Android FBE storage is not made decryptable merely by having a raw image. Do not offer FRP/iCloud bypass, IMEI modification, PIN search, credential extraction, EDL security bypass or automatic rooting/unlocking.

## Evidence requirements

Every acceptance entry identifies the exact MR revision, external engine revision/hash if any, input fixture hash, command/protocol, environment, output hashes, source-before/after hashes, exit/error/cancellation state and limitations. Keep synthetic test evidence separate from customer data. Unknown fields in the device matrix are JSON null; an untested case is `not_run`, never a guessed pass or zero recovered files.

The seven matrix categories are Samsung, Xiaomi, OPPO, realme, Google Pixel, legacy Android and modern Android FBE. The last two overlap the OEM categories; they are capability cohorts, not claims of distinct manufacturers or complete model coverage.
