# External integration and license register

Review date: 2026-10-16. Repository identities, default-branch commits and license files were checked using GitHub API/source, not inferred from names or GitHub license badges. No third-party project source has been incorporated into the C++ application.

| Project | Official/candidate source | License observed | Technical fit | Decision |
|---|---|---|---|---|
| scrcpy | [Genymobile/scrcpy](https://github.com/Genymobile/scrcpy/tree/a60891aea193d92e7e5c3942700eca63f9d19a5f) | Apache-2.0 (`LICENSE`) | C client/server; official Windows binaries | External process on explicit user consent; Windows evaluation package includes the pinned official runtime and its license/provenance receipt. |
| OpenBoardView | [OpenBoardView/OpenBoardView](https://github.com/OpenBoardView/OpenBoardView/tree/cc76e697c85efd2285134dfe2df0b7809e4a49d0) | MIT (`LICENSE`; dependency licenses still separate) | C++ / SDL / ImGui desktop viewer | Optional separately installed viewer; native JSON boardview remains the embedded path. |
| TRACE Boardviewer | [trace-boardviewer/trace-boardviewer](https://github.com/trace-boardviewer/trace-boardviewer/tree/4082c45bf2ab58f6ff3edbcfc1272bcf189c2be7) | MIT (`LICENSE`); dependencies carry their own terms | Electron / React / TypeScript; Windows desktop | Reference only; no Electron runtime, parser or assets copied. |
| PixelFlasher | [badabing2005/PixelFlasher](https://github.com/badabing2005/PixelFlasher/tree/081286d6baf8ac969df29e8b0ab3a5b6ae4cfb54) | Conflicting: root `LICENSE` GPLv3; `phone.py` SPDX AGPL-3.0-or-later | Python / wxPython; platform-tools orchestration | No source reuse. Native read-only commands and compatibility validation developed independently; flashing deferred. |
| FixTrack (candidate) | [Ahshika/FixTrack](https://github.com/Ahshika/FixTrack/tree/fc31944e12040f311584cd8a42f6612c7b408dbf) | No root license found; GitHub license metadata null | Flutter / SQLite / local LAN server | Name is ambiguous; candidate matches repair-shop domain. No permission to reuse assumed; native SQLite implementation. |
| ERPNext | [frappe/erpnext](https://github.com/frappe/erpnext/tree/f6e6e2ca705cf0ed77adfcf6a462e88e05dea0af) | GPLv3 (`license.txt`) | Python / Frappe server application | Future official REST API only after explicit server setup. No embedded ERPNext or copied source. |
| Open Device DB (candidate) | [RightToRepairMobile/open-device-db](https://github.com/RightToRepairMobile/open-device-db/tree/5412194ddb75e233d54d7a487c4e120b970749ae) | Code MIT (`LICENSE`); README specifies data CC BY-SA 4.0 | Python / Streamlit / SQLite; contributed and AI-enriched catalog | No data imported. Data accuracy, provenance, attribution and share-alike obligations need review first. |
| Zdartix Flash Tool | [Zdartix/Zdartix-Flash-Tool](https://github.com/Zdartix/Zdartix-Flash-Tool/tree/045db1bc119a5a7abba049b54854ad27cebf111c) | Actual `LICENSE`: proprietary; personal non-commercial use only; README MIT claim conflicts | README describes Python frontend; no verified source-based integration | Excluded from redistribution/source reuse without written permission. |

## Reviewed source revisions

- scrcpy: [`a60891aea193d92e7e5c3942700eca63f9d19a5f`](https://github.com/Genymobile/scrcpy/commit/a60891aea193d92e7e5c3942700eca63f9d19a5f).
- OpenBoardView: [`cc76e697c85efd2285134dfe2df0b7809e4a49d0`](https://github.com/OpenBoardView/OpenBoardView/commit/cc76e697c85efd2285134dfe2df0b7809e4a49d0).
- TRACE Boardviewer: [`4082c45bf2ab58f6ff3edbcfc1272bcf189c2be7`](https://github.com/trace-boardviewer/trace-boardviewer/commit/4082c45bf2ab58f6ff3edbcfc1272bcf189c2be7).
- PixelFlasher: [`081286d6baf8ac969df29e8b0ab3a5b6ae4cfb54`](https://github.com/badabing2005/PixelFlasher/commit/081286d6baf8ac969df29e8b0ab3a5b6ae4cfb54).
- FixTrack (candidate): [`fc31944e12040f311584cd8a42f6612c7b408dbf`](https://github.com/Ahshika/FixTrack/commit/fc31944e12040f311584cd8a42f6612c7b408dbf).
- ERPNext: [`f6e6e2ca705cf0ed77adfcf6a462e88e05dea0af`](https://github.com/frappe/erpnext/commit/f6e6e2ca705cf0ed77adfcf6a462e88e05dea0af).
- Open Device DB (candidate): [`5412194ddb75e233d54d7a487c4e120b970749ae`](https://github.com/RightToRepairMobile/open-device-db/commit/5412194ddb75e233d54d7a487c4e120b970749ae).
- Zdartix Flash Tool: [`045db1bc119a5a7abba049b54854ad27cebf111c`](https://github.com/Zdartix/Zdartix-Flash-Tool/commit/045db1bc119a5a7abba049b54854ad27cebf111c).

ERPNext was observed on an actively changing development branch; the pinned SHA above is the reviewed snapshot, not a supported ERPNext release. None of these upstream versions is represented as bundled or fully compatibility-tested.

## Implemented adapters

- Android platform-tools: discover ADB/Fastboot; structured read-only device enumeration, properties and bounded logs. The official scrcpy distribution supplies bundled ADB in Windows evaluation packages. No firmware, driver installer, shell console, unlocking or flashing is added by that bundle.
- scrcpy: dedicated consented screen-view policy in both profiles; exact 5.0.1 CLI, pinned target/ADB, identity/disconnect checks, bounded output/deadlines and owned stop/kill lifecycle. No control, audio or clipboard synchronization. Other generic write operations remain Laboratory-only. Bundled Windows runtime resolution/version/digests are checked during portable and installer smoke. See SCREEN_VIEW.md; actual phone frame delivery/cleanup remains an external gate.
- Boardview: native C++ importer and QML renderer for the documented NEXVARY JSON format; original synthetic training data. This does not import OpenBoardView or TRACE file parsers and does not imply compatibility with their proprietary board formats.
- Workshop: independently implemented local Qt SQL / SQLite workflows; no FixTrack/ERPNext source or data copied.

## Distribution boundary

License permission for an application does not license the phone manufacturer's schematics, firmware, product photos or device-data sources. Users import only files they have permission to use. No protected schematics, production repair voltages, firmware or customer information is bundled. External tools must be obtained from their official sources and remain separately licensed.

No bypass of FRP, iCloud, device locks, authorization prompts or IMEI identity protection is exposed. No external project is claimed as a direct source integration when it is only researched, independently reimplemented or launched as a separate process.


### Bounded local ZIP metadata inspection

Firmware → ZIP structure and risks reads a regular local ZIP working copy into a bounded memory snapshot (22 bytes–128 MiB, at most 4096 entries). The shared native ZIP structural validator provides declared entry names, compression methods and sizes; the report lists at most 256 entries and explicitly flags truncation. SHA-256 describes the same memory snapshot used for parsing. The original synthetic test ZIP was generated independently with Python `zipfile`; its known SHA-256 is recorded in the test fixture/evidence.

No extraction/decompression, payload CRC, AVB/OEM signature verification, device compatibility, partition-layout approval or flashing is performed. Image filenames are candidates only. Large ROMs, ZIP64, encryption, multiple disks and ambiguous/non-UTF-8 entry-name encodings are unsupported. Unsafe names/control formatting and malformed local/central headers are rejected. Existing SHA-256/model matching and raw AVB inspection remain separate tabs using the same FirmwareBridge lifecycle; no device-write permission is granted by any ZIP result.

Export requires explicit filename/hash sharing approval and a new local output file; source/other files cannot be overwritten. Cancellation and input/device invalidation clear results. The AR/EN interface and synthetic fixtures do not establish acceptance of any physical phone, OEM ROM or Windows 10/11 installation.
