# Next gate: integrate the existing NTFS v2 replay into signed case acceptance

Completed prerequisite: bounded deterministic v2 MFT/USA/source observations and
full canonical report replay against supplied protected bytes. It remains compiled
only into mr_ntfs_metadata_tests. The following case work is NOT implemented or
accepted by these tests:

- Obtain the current authorized case/evidence/revision and verified original hash
  from existing LawCaseStore/Bridge services. Do not construct replay bindings from
  the submitted report. Working-copy hash is measured from freshly re-read bytes;
  a caller-supplied UUID/hash grants no access or original acquisition proof.
- Build a single common forensic-adapter envelope with pinned descriptor/version,
  exact parameters and NTFS v2 observations. Reuse existing evidence persistence;
  no new DB, writable path or standalone app. Never label raw record hashes,
  reconstructed resident hashes, recorded runlists or bitmap observations as a
  complete/partial recovered file or independently verified recoverability.
- Before durable preparation, re-read protected source and replay the entire
  internal report under the current cancellation/deadline; recheck authority,
  actor/session/case membership/revision at existing transaction boundaries. Put
  validation on the persistence path as well as the async callback so direct
  store acceptance cannot bypass it. A rehashed arbitrary report is not proof.
- Retained projection must enforce a strict common/nested schema and source/map/
  USA/name/stream/counter invariants against the accepted source and existing
  signed receipt/report hash. Inventory-only range bounds cannot prove byte
  mapping; arrange an authorized bounded source-replay path where needed, with
  aggregate limits/cancellation. No permissive old-v1 or unknown-field fallback.
- Add only the exact dc-ntfs-metadata case kind consistently to native prepared
  intent/public Python verification/interoperability and the production registry
  once those gates pass. Keep format/repair/erase/rebuild/recovery writes denied.
- Add actual same-case acceptance and signed receipt tests, forged rehashed source
  references, wrong/cross-case member, post-dispatch protected BLOB replacement,
  cancel/timeout/late result, logout/expiry/revoked grant, failed audit, durable
  crash-point reconciliation and restart without duplicate results. These require
  real case/identity/policy services; current UUID-binding tests do not cover them.
- Exercise Findings/Reports and any Timeline projection only after retained
  validation. If exposing FILE_NAME timestamps, retain every raw unsigned100ns
  value/basis and display precision; no deletion-event/human-activity or parent
  hierarchy inference. Add actual AR/EN controls and100/125/150DPI acceptance.

Source scope remains offset0 NTFS3/512-byte sectors,8MiB image/128MFT records;
unsupported source rejects as a whole. Larger images need existing authorized
read-only range/hash/read-coverage services, not live arbitrary paths/new stores.
Historical Audit Store divergence root, process-grandchild containment and original
Qt crash root/soak/hardware remain separate open blockers. No causal link or fix
for those issues is established by this prerequisite or passing CI.
