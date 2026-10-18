# External forensic candidates — source/license inspection, 2026-10-18

`UPSTREAM_REVIEW.json` records exact upstream HEADs, commit dates, archive flags and license-file hashes/URLs inspected in this session. These are a source snapshot, not a promise of continued maintenance. None of these candidates has a native desktop adapter in this increment; no executable or source code from them is shipped.

| Candidate | Observed licensing / integration decision |
|---|---|
| Autopsy | Apache license text found at pinned `LICENSE-2.0.txt`; bundled components have their own notices. Prefer separate user-installed application and review all dependency notices before distribution. |
| The Sleuth Kit | Mixed licensing; upstream `licenses/README.md` must be reviewed per component. Prefer a bounded external read-only CLI adapter; do not treat the whole repository as Apache/MIT. |
| ALEAPP | MIT at pinned LICENSE; dependencies and parser data still need review. Candidate for explicitly selected extracted Android files, not automatic protected-data acquisition. |
| iLEAPP | MIT at pinned LICENSE; separate dependency review. Candidate for authorized extracted backups only. |
| PhotoRec / TestDisk | GPL text in pinned COPYING. Upstream README/docs and file headers determine version/exception details; no linking or bundling selected. Official downloads distinguish stable 7.2 and development 7.3-WIP. TestDisk includes writing operations, which must not be exposed against evidence originals. |
| Volatility 3 | Volatility Software License 1.0, **not** the Volatility 2 GPL assumption. Explicit redistribution/legal review required. No memory acquisition capability implied. |
| MVT | MVT License 1.1 includes a consent restriction; **not** unmodified MPL and not assumed OSI permissive. Explicit owner consent is required by its terms. |
| Plaso | Apache-2.0 main license; heavy parser/dependency review and explicit offline operation needed. |
| bulk_extractor | Repository uses a project/copyright status notice rather than one universal permissive label; review LICENSE.md and incorporated dependencies/components before any distribution. |

Official source URLs and immutable revisions are in the JSON snapshot. Additional official references: https://www.cgsecurity.org/wiki/TestDisk_Download , https://www.cgsecurity.org/testdisk_doc/presentation.html , https://sleuthkit.org/autopsy/licenses.php , https://github.com/mvt-project/license .

No legal compatibility conclusion follows from executing an external process alone. No sandbox, Windows compatibility or integration success is claimed without a corresponding test.
