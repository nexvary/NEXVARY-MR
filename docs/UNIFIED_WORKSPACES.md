# Unified repair, case and research workspaces

Date: 2026-10-24. Development increment, not an institutional or commercial release.

## Implemented paths

- One Qt application. Existing repair/USB/firmware/workshop pages retain their page IDs. Law Enforcement is page 12; Research Center is page 13.
- Native `LawCaseBridge`: create, open, search, edit, close and archive case records; configurable jurisdiction and recorded legal scope; prosecutor/court/date fields remain optional. Completeness never grants legal or device-operation authority.
- `LawCaseStore`: SQLite transactions and optimistic revisions; protected metadata, device records, evidence manifests, custody declarations and audit events. No plaintext case-number/name search index. Search decrypts at most 1000 local case records in memory.
- Current-account Windows DPAPI uses the existing `CaseVault` provider. Linux refuses production case storage. Only native synthetic test/smoke composition enables the visibly labelled non-encrypted Linux fixture profile.
- Registered devices can copy existing Device Intelligence observations without starting a command. Screen lock/PIN, FRP, FBE, hardware protection, bootloader declarations and acquisition limits are assessed separately; no bypass/exploit or protected-phone extraction is added.
- Evidence import copies a regular local file of at most 16 MiB into a protected database blob; it records SHA-256, size, device and initial custodian. No file preview, content parser, executable launch or network transfer occurs. Symlinks and nonregular inputs are rejected; mapped network filesystem provenance cannot be inferred from a local path.
- Custody transfers require matching current custodian, a different receiver and a reason. The case state and custody events are covered by a hash-linked protected audit. Case edits fail on a stale revision. Reports reload current state and verify every stored evidence blob before creating a new consented PDF.
- Native action-level role policy: investigator, reviewer, auditor, trainer, repair technician and unknown/default-deny. A QML role picker cannot grant permissions. Training has three synthetic scored decision scenarios and exportable session results.
- Research Center: dashboard, searchable Atlas and tool records, compatibility intake, experiment plans/results, validation view, public research JSON export and training. The source-backed catalog has 16 Atlas topics, 10 tool records and 3 planned experiments. No physical-device result is pre-populated.
- Public research results record required methodology/environment/scope/version fields and the SHA-256 of an explicitly selected synthetic/public artifact. The local ledger uses atomic save, a concurrent-writer lock, revision checks and an integrity checksum. Reload also rejects fabricated commercial support and unreviewed `Lab Verified` assertions even if the checksum is recomputed.
- Shared live context comes from the existing ToolBridge Device Intelligence/USB diagnosis and FirmwareBridge AVB/package reports. `research/advanced-mobile/knowledge.json` stays the native capability-provenance catalog. Research never overrides native service policy.

## Trust and deployment limits

The desktop now requires an authenticated local identity and a case ownership/grant for case access. Administrators provision local accounts and grants without automatic case-content access. Password rotation preserves signing identity and invalidates sessions. Institution identity verification/SSO, independent human review and court accreditation remain deployment gates. See [LOCAL_IDENTITIES.md](LOCAL_IDENTITIES.md) for the exact credential, recovery and threat boundaries.

A separately authenticated reviewer can hash the original public/synthetic artifact and sign an immutable review. Author/reviewer signatures are checked on reload against pinned local public keys; self-review and forged promotion are rejected. This does not promote a synthetic experiment into commercial/device compatibility. Qualcomm, Knox and OEM-update topics now have scoped reviewed primary references; MediaTek smartphone architecture remains a hypothesis with Genio/IoT documentation explicitly separated.

A consented signed audit anchor can be retained outside the case database and compared against its audit prefix. Tests detect a database restored to a revision older than the retained anchor. This is local credential signing, not an independent trusted timestamp or evidence of physical handover; restoring all identity/anchor/database copies together is outside this check. DPAPI recovery depends on the original Windows account and its supported backup/recovery process; no plaintext fallback is introduced.

No malware sample, leaked source, proprietary binary or exploit implementation is imported. The historical Hacking Team record concerns the cited 2014 study; Galileo/DaVinci version mapping and present-day support are not inferred. Current vendor pages describe vendor claims, not independent NEXVARY experiments.

## Package provenance and canonical data

`research/program/IMPORT.json` pins the supplied ZIP SHA-256. The user authorized integration of this NEXVARY-authored foundation; the ZIP contains no standalone license. Compiled `__pycache__` files are excluded. Third-party sources remain links and original summaries with explicit rights notes. The Amnesty source URL was corrected against the actual published report.

`research/program/catalog.json` is the executable source catalog. Imported `research/program/research/*` files retain package provenance; their small original validator is archival structural coverage, not an alternative certification authority. `research/compatibility/schema.json` now matches native compatibility intake states/fields. No duplicate physical-device capability database is introduced.

## Validation and resume

```sh
python3 scripts/validate_research_catalog.py
python3 -m unittest discover -s tests -v
python3 research/program/tools/validate_research.py
(cd research/program && python3 -m unittest discover -s tests -v)
python scripts/prepare_sodium.py --destination build/sodium
cmake -S . -B build -DMR_SODIUM_ROOT=build/sodium -DMR_BUILD_GUI=ON -DBUILD_TESTING=ON -DMR_WARNINGS_AS_ERRORS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

`mr_identity_tests`, `mr_law_workspace_tests`, `mr_research_tests`, the existing 432-check case suite, the core research data gate and the expanded Qt Quick navigation test are included in existing Windows/Linux CI. The Quick test creates/reopens a synthetic case, registers evidence, visits every new section and captures real AR/EN runtime PNGs. Existing repair, firmware, workshop and security regression suites remain in CTest.

Final acceptance still requires exact-source hosted Windows/Linux results, a successful packaged Windows run, institution-verified identities/independent review, external key/time custody and owned-device/clean-host validation. See the current `DEVELOPMENT_STATE.md` and retained test output for actual results; no physical-device or final-release claim follows from synthetic tests.

## Windows Qt 6.8.3 compiler compatibility

Run 40090972966 at db373b7 passed both Linux profiles, both OS core jobs and
tracking-server. Windows stopped during generated QML C++ compilation: MSVC C4702
in Qt qmetatype/qvariant/qjsengine headers became C2220 under /WX. No native
case/research source diagnostic was reported in that failure.

For the pinned MSVC + Qt 6.8.3 combination, the application uses Qt's documented
`QT_QMLCACHEGEN_ARGUMENTS=--only-bytecode` mode. QML bytecode remains precompiled;
project native warnings-as-errors and all runtime/packaging tests remain enabled.
Other platforms/SDK versions retain their existing compiler path. Exact-source
Windows packaging must pass after this change before a binary is presented.
Reference: https://doc.qt.io/qt-6.8/qt-add-qml-module.html#caching-compiled-qml-sources

## Report visual verification

The case PDF now renders metadata, devices, evidence, custody and audit as labelled
AR/EN tables rather than raw JSON paragraphs. Technical identifiers/digests use
LTR blocks and table captions repeat across page breaks. The synthetic Arabic
fixture was exported by the native CaseBridge, rendered with Poppler and visually
reviewed on all three pages. Text extraction confirmed that the complete evidence
SHA-256 was preserved. The native case suite passed all 9 cases after the change.
`evidence/unified-2026-10-24/synthetic-case-report.pdf` contains synthetic data only.
