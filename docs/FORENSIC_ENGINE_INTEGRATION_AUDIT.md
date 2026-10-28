# Three-source forensic integration — source audit and first native slice

Tracking: #16; PR #17; branch `integration/forensic-engines-audit`.
Status: three selected original functions adapted into executable C++/Qt;
full upstream audit and final acceptance remain in progress. This is not a
complete migration of the three applications, government accreditation, or a
comparison against commercial products.

## Exact inspected inputs (2026-10-28)

| Repository | Inspected main HEAD | Transport / integrity |
|---|---|---|
| NEXVARY-Digital-Forensics-Workbench | `8b35940b1c879e0ce5f5c13e0d6c89aeab023757` | Workflow-selected `source_payload_v1222/00..12.b64`; 93,656-byte tar.xz SHA-256 `9eff37d785b6e44df2e3a611f9d44387b5a78dccd9bf240a4b352edb8b14bb3e` |
| NEXVARY-Browser-Forensics-Lab | `d7754cd8f051d285406403465b6d7daa3bc4b060` | `.github/source-package-v420/part01..08.b64`; 84,692-byte XZ SHA-256 `62b9af68d8d8a0a78a7126b113b8344852e80b7f199c76c721844edb5d7ee226` |
| NEXVARY-DC | `d8e2baf61b43d044b33a1dd76a444d093c80023c` | Direct text sources at exact commit; source hashes in inventory |

The decoded Browser TAR hash is recorded by its reconstruction driver as
`fa5241aab80e7a9bc0839ea35c56e3945be1751e2fdfba271bcc4403c0f620a0`.
Workflow overlays modify the updater/UI smoke/build, not the selected bookmarks
function. The historical ZIPs in both repositories are not assumed to be the
current workflow payload. No upstream code was executed during extraction;
archive paths/links were checked by Python's data extraction filter.

Initial MR integration HEAD was `08d8fb6a9a830ee607b8e5fd495593a7c0ee8740`.
Government HEAD was `eb2322140956a55a50ee7494570f35573a1ce0c0`, incorporating
prepared case transactions and the additional summary-card/MSVC corrections.
It was merged locally without conflicts before this implementation. Its Push
40593541867 and PR 40593551259 each concluded success; PR had seven successful
jobs. Those receipts do not test this changed integration source. Historical
Qt crashes, signed-disk/in-memory journal divergence and surviving detached
children remain retained/open, despite these successful latest runs.

## Source/module → MR equivalent → decision matrix

Permission column refers to the exact [scoped owner permission](FORENSIC_SOURCE_PERMISSIONS.md), not a nonexistent general open-source license.

| Source module / real API | Existing MR equivalent | Reuse decision | Target adapter | Permission / dependency | Test evidence |
|---|---|---|---|---|---|
| Workbench `core.py:ForensicDB.add_evidence/verify_evidence/log`, SHA-256 helpers | LawCaseStore protected evidence, membership, SHA-256, custody, PolicyAudit | Keep MR; no second case/evidence/audit database | Existing MR | Original NEXVARY; Python sqlite3/hashlib/shutil | Workbench decoded baseline 69 pytest passes; MR case/evidence tests remain required |
| Workbench `advanced.py:export_case_html`, core timeline/findings/entity/review tables | Native PDF, retained analyses, artifact/timeline projection, identity and reviewed policy | Do not copy competing stores/report UI | Existing MR | Original NEXVARY; stdlib HTML/CSV/JSON | Baseline tests; no institutional report equivalence claim |
| Workbench `analyzers.py` SQLite/ZIP; `advanced.py` PCAP and generic SQLite | Native bounded PCAP, import worker, ZIP/SQLite inventory | Do not duplicate these engines | Existing MR | Original NEXVARY; stdlib | Existing tests are preserved; not all Workbench catalog semantics are validated |
| Workbench `pro_tools_v1221.py:export_validate` | Actual MR evidence package verification exists; generic foreign manifest structure inspection does not | Port structural entry/hash/duplicate checks; strengthen missing hashes, duplicate JSON names and limits; never assert payload verification | `workbench-export-manifest` | Scoped owner permission; Qt Core only | New native parser/real case tests; original baseline test suite |
| Workbench updater / internal CA / signed-release scripts | MR policy trust, update verifier, independent journal verification | EXCLUDE | None | Separate trust model | No borrowed signing/accreditation assertion |
| Browser `core.py:CaseManager`, evidence snapshots, reports | Canonical MR protected store/export/report | EXCLUDE parallel store | Existing MR | INTERNAL_USE is not a redistribution license | Baseline core5/integrity9 passes |
| Browser `parsers.py:ChromiumParser.bookmarks`, `core.py:chrome_time` | Generic JSON import only; no semantic bookmark analyzer | Native port: folder/name/URL/date-added traversal, source pointer, raw timestamp and cautious confidence | `chromium-bookmarks` | Scoped owner permission; Qt Core only | Known-answer UTC, malformed/duplicate/deep/limits/cancel/real case tests |
| Browser Chromium/Firefox history/downloads/cookies metadata, `browsers.py` profiles | Generic standalone SQLite sample only | DEFER: schema-specific adapters must preserve verified SQLite families and bounded execution | Not enabled | stdlib sqlite3; separate pytsk3/lz4/PySide6 dependencies not imported | Selected upstream tests pass; no migrated SQLite/WAL acceptance |
| Browser `session_recon.py`, `cache_recon.py`, `deleted.py`, `sqlite_recovery.py`, `images.py` | Partial unrelated MR imports/recovery | DEFER: session/cache reconstruction and carving confidence need separate fixture review | Not enabled | Optional TSK/lz4/Pillow-related paths must be independently audited | No claim of decryption, complete sessions, physical extraction or recovered-visit truth |
| DC `Partitions.cpp:imageVolumes` MBR/EBR/GPT | Existing external mmls; no native memory-only GPT/EBR parser | Port only partition parser; explicit 512-byte sector scope, CRC/bounds/overlap/EBR-cycle checks; remove raw-volume fallback and all file access | `dc-image-partitions` | Scoped owner permission; Qt Core only | Synthetic MBR/GPT/backup/corruption/overlap/cycle and mutation corpus; actual case retention |
| DC `FilesystemRecovery.cpp` NTFS/exFAT/FAT32/deleted output | Existing TSK/file signature recovery partially overlaps | DEFER: source includes destination writes and read-map semantics; split read-only analysis before any future reuse | Not enabled | Qt Core; compressed/encrypted NTFS explicitly rejected by source | `tests/test_advanced.cpp` source inspected; upstream native suite not run locally |
| DC `ReadCoverage`, `MediaRead`, `Recovery`, `RescueEngine`, smart/Crystal controllers | Partial MR recovery/tool wrappers | DEFER; not linked wholesale | Not enabled | Qt/OS/raw handles/external tools | No transplanted hardware capability claim |
| DC `Operations`, storage/policy/hybrid/boot/firmware/direct/relay helpers | Repair/lab functions outside government scope | EXCLUDE: writes/capacity tests/format/partition/boot/controller operations | None | PowerShell, GRUB/efibootmgr, OpenSuperClone/USB relay; third-party notices apply | New native allowlist rejects format/erase/repair/write; none of these modules linked |

## Inventory and execution boundaries

The machine-readable `research/forensic-upstream-source-inventory.json` records
file sizes/hashes and Python classes/functions/imports from the decoded payloads
and DC text source/build/test/docs snapshot. Binary assets and unused embedded
older archives are not copied into MR. This is an inventory and a targeted
function review, **not proof that every catalog entry was deeply audited**.
Unreviewed modules remain deferred.

Workbench: `run_lab.py` → `nexvary_lab/app_v9.py` (Tkinter), older `app.py`,
`core.py`, `analyzers.py`, `advanced.py`, staged engine/catalog modules and
`pro_tools*`. Runtime engine selection and source tests expose real parsers and
heuristics; high stage/catalog counts do not prove comprehensive forensic engines.
Dependencies: stdlib runtime/Tkinter; pytest and PyInstaller build dependencies.
The inspected Windows workflow reconstructs v12.22 and performs tests/build/
install/run/uninstall. Latest returned run35842950760 failed at different source
4332a804..., not the inspected SHA; no exact-HEAD upstream CI pass is claimed.
A first local pytest command used the wrong cwd (63 pass/3 missing-relative-file
failures, two selected UI-source tests omitted); corrected full baseline ran all
69 tests successfully. This is source-unit evidence, not a Windows launch test.

Browser: `app.py` / `nbf_cli.py`; `nbf/core.py` SQLite case store; parser,
profile discovery, report, image, residual/cache/session and UI modules. Windows
requirements list PySide6, lz4, PyInstaller and optional pytsk3. The selected native
port needs none of those. The decoded baseline had 89 passes/1 missing-icon asset
failure under pytest, before CI icon restoration/updater overlays. Selected
core5/integrity9 tests passed; original full unittest output stopped before a
summary, so it is not promoted to a full pass. Latest returned workflow runs at
23e7cda4... failed; no exact-inspected-HEAD CI success is claimed.

Browser SQLite snapshots preserve filenames and copy DB/WAL/SHM/journal with
per-file hashes. Sequential copying plus matching hashes does not independently
prove a simultaneous consistent snapshot of an actively changing family. The
source `mode=ro` URI and unbounded row materialization need further hardening
before migration. The bookmarks adapter consumes only one registered JSON byte
array; it never opens a live profile or SQLite. Thus no WAL is silently dropped
by an enabled SQLite adapter: no such adapter is enabled in this increment.

DC: CMake Qt6.8 C++20 app; `src/app` controller/external tool UI; `src/core`
parsers/recovery/operations; scripts for powerful administrative actions;
C++/Python/PowerShell/VM tests. Linux/Windows use different raw access and tools.
The original native parser tries 512/4096 GPT sectors, but this first MR port
supports 512 only and limits table entries4096/table2MiB/input8MiB/findings128.
No FAT/NTFS/exFAT or damaged-filesystem recovery support is inferred from this
partition adapter. Latest returned DC runs38022683250/38022679120 succeeded at
2bc8ea1d..., a different SHA; exact inspected-HEAD acceptance is unproved.

No general original-code LICENSE was found in the inspected inputs. Selected
original function reuse relies on the project owner's explicit migration grant.
Qt/third-party licenses remain separately scoped; no proprietary third-party
binary, controller patch, key, case database or private evidence is imported.
External legal/distribution review remains open.

## Native architecture and actual case integration

`ForensicAdapters.hpp` provides a byte-only request/result/source/finding/receipt
boundary. An exact three-ID allowlist has no generic command, writable device,
file path, dynamic plugin or Python execution entry point. The native case bridge
obtains actual protected evidence from the existing store, verifies its hash,
records case/evidence/revision/actor and dispatches a bounded QtConcurrent task.
The parser verifies the supplied digest again. Cooperative five-second deadline,
input8MiB/output64KiB/findings128 and bounded traversal prevent unbounded
ordinary work; no OS-enforced memory cap or hard thread termination is claimed.

On callback, cancellation, current session/actor, actual membership/grant,
active/current case revision, source bytes and all report identifiers are
rechecked. The existing local v1 prepared intent and SQLite case/result/audit
transaction then retain the report. The strict native and Python signed-history
allowlists recognize only these three additional kinds. Restart reconciliation
uses the existing exact binding and cannot rerun a parser or duplicate a result.
A new deliberate user analysis creates a new invocation; retry/restart idempotency
is not a claim that all repeated user analyses are deduplicated.

Real Government Analysis has bilingual run/cancel/evidence/adapter controls and
plain-text actual results, with stable layout ownership. New Qt tests execute a
real bookmark result and capture AR/EN frames at responsive sizes in the full
suite and both existing125%/150% DPI groups. These frames and tests require actual
execution; source presence alone does not verify layout, flicker or crash freedom.
Retained reports appear in existing case PDF analysis tables. Findings and timeline projections now validate the retained report hash, exact
source descriptor, case/evidence and metadata schema. Bookmark date-added remains
source-reported metadata, never a visit. Raw microseconds are retained; submillisecond
values remain unresolved under the existing timeline precision gate. No hidden
reinterpretation as generic authorized imports.

## Acceptance discipline / remaining work

All final Windows/Linux profiles, full Qt/DPI, signed interop and ASan/UBSan need
new exact-source results. Historical packages/screenshots/CI remain scoped to
their original source. Retain first failures. No main merge, branch deletion,
upstream modification, Guardian dependency or institutional certification.
Remaining source review includes browser SQLite families/history/downloads/
cookies/sessions, DC read-only filesystem splitting, and any genuinely new
Workbench capability. Existing Audit Store divergence, process-tree containment,
physical Windows10/11/install/device/hardware-renderer and external custody/
identity/time/signing/comparative acceptance are not closed by these adapters.
