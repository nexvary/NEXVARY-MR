# Case finding index and recorded provenance

Government → Case evidence analysis → **Finding index and provenance** rebuilds
a read-only in-memory view of this case's retained authorized-import findings.
Enter an optional literal query, choose Build index and links, page through the
findings and select Show provenance path. The five-step path describes the case,
registered device, source evidence, retained analysis and individual finding.
The native result also contains typed graph nodes/edges and recorded-digest groups.
No evidence or case database/schema is duplicated or migrated.

## Implemented scope

- Search in retained values, references, source names, kind/adapter, record IDs,
  source/report/member hashes. Unicode case-insensitive literal matching; Arabic
  works literally. No regex, token/phonetic matching, stemming, diacritic removal,
  letter normalization, OCR or semantic search. JSON field names are not searched.
- Deterministic, case-scoped finding/node/edge identities and sorting. Source
  SHA-256 and report SHA-256, analysis/evidence/device IDs and original finding
  ordinal/reference/shortening flag remain attached. Same labels do not merge
  nodes. Reordering reports below the result cap preserves the result.
- Typed `registers-device`, `registers-evidence`, `input-to-analysis`,
  `reports-finding` and `records-sha256` edges. The first four describe recorded
  provenance, not a real-world event. SHA nodes refer to **recorded digests**.
  Native graph identities include the case ID, even for equal hashes elsewhere.
- Digest groups contain distinct evidence/finding nodes with an equal recorded
  digest. Evidence digests come from case metadata; member digests come only from
  complete SQLite BLOB SHA observations or ADB v5 member inventory fields.
  Directory hashes are empty. App/name/phone/contact/IP values never create a
  person/entity relationship. Repeat imports are not independent corroboration.
- The UI shows 12 findings/page, a selectable actual native five-node path and
  the first eight digest groups with four references each. Search a digest to
  narrow results. This is not a general interactive graph canvas.

## Integrity and authorization

The native bridge checks session/AnalyzeEvidence/current case grant and exact case
revision before dispatching a snapshot to QtConcurrent. The worker receives value
data and an atomic cancel flag only, without file/database/QObject/network access.
Completion repeats identity/actor/grant/revision checks before publishing a result.
The retained result clears on observed revision/case changes, local identity
changes, logout or cancel. Search input and selected path clear on case change.
External identity/grant changes are detected through the existing session refresh
and operation checks, not a new push/polling identity service.

Before indexing, all supported reports are checked against exact report bytes/hash,
case evidence membership/hash/size, parser/adapter/version and protected provenance
case/device/evidence IDs. Duplicate decoded JSON keys, malformed fields, unknown
adapter versions and inconsistent digest/member metadata fail the whole view.
Checks continue after the retained-result cap. Rejected imports are counted but
not indexed; PCAP/EXIF and other non-import analyses are counted outside this view.
Registered evidence with no successful retained import has no finding rows here.

This operation **does not reread, decrypt, rehash or reparse original evidence**.
Use the existing Verify Evidence operation for current-byte integrity. A valid
report hash proves consistency with the retained record, not original-device,
manifest, source-time, signature, person or legal authenticity. Equal recorded
digests do not independently verify current bytes or ownership. Training/synthetic
profiles retain their existing explicit separation from real protected cases.

## Bounds and reproducibility

Snapshot ceilings: 100 devices, 1,000 evidence records, 100 analyses, 64 KiB/report
and 128 findings/report. Existing case intake/storage limits may be lower.
Search is capped at 256 UTF-16 code units. Escaped display-safe values/references
retain the importer bounds (up to 1,536/3,072 code units after control escaping).
At most 2,048 matching findings are retained, chosen in stored report/finding order
**before** sorting by stable finding ID. It is not a global chronological sample.
Observed/matching/omitted counts and already-truncated reports remain explicit.

Graph and digest groups describe retained matching rows only. Maximum graph:
8,192 nodes/12,288 edges; up to 64 distinct member references/group with an
omitted-member count. All graph edges are deduplicated; source evidence is not
counted multiple times merely because its report has many findings. Results have
no new wall clock/random IDs; rebuilding from the same protected source reports
reproduces rows, graph and groups. Protected backup/restore retains those source
reports and reconstructs the view. The current projection is not itself a signed
review, persistent/full-text index, export or added audit-history event.

The worker is bounded and cooperatively cancellable; it has no new hard execution
deadline. Snapshot retrieval uses the existing bounded case store on the UI thread;
parsing, matching, graph construction and sorting run off that thread. Large UI
and long-session/physical-host performance still require acceptance testing.

## Reproduce tests

```sh
python scripts/check_artifact_reference.py
ctest --test-dir build -R 'mr_(artifact_tests|artifact_reference_validation|ui_tests|ui_scale_)' --output-on-failure
```

The independent Python reference supplies a hand-labelled synthetic topology:
6 findings, 19 nodes, 20 explicit typed edges and two digest groups. Repeated Arabic
names and equal device/file labels deliberately create no identity links. Native
tests compare every expected edge (false/missed counts on that corpus only),
literal Arabic queries, cross-case identity separation, duplicate/malformed/
wrong-provenance/hash/count negatives, limits/full validation after truncation,
12,800-observation benchmark, ADB/SQLite digest scope, protected restore,
cancellation, roles, logout, stale revision and revoked grants.

Qt tests operate real controls: two authorized sources, imports, index rebuild,
three pages, selectable provenance path, Arabic literal search, regex-as-literal
negative, query/result clearing and AR/EN multi-size/DPI actual captures. Exact
source/CI/raw outcomes belong in DEVELOPMENT_STATE.md. A synthetic structural truth
corpus is neither application/forensic recall coverage nor independent laboratory,
field or institutional verification. No competitive superiority is established.

The explicit synthetic application `--smoke-test` profile additionally creates a
case and JSON evidence, runs the actual deployed import child/index, verifies one
Arabic match and a five-node path, and captures AR/EN from packaged QML resources.
Windows portable/installed smoke checks independently require the fixture source
hash, 6 graph nodes/5 edges, receipt and both screenshots. This is development
fixture testing, not a normal-production data path or field acceptance.
