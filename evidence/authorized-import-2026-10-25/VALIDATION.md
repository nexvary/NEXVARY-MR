# Verified authorized import and navigation checkpoint

Latest executable source: `feed968f905cbc3d2960e94cfa208139b7642b53`.
[Native/package run 40197393089](https://github.com/nexvary/NEXVARY-MR/actions/runs/40197393089)
passed all seven jobs. Windows/Linux production/laboratory each passed 41/41
CTest; production sanitizers passed 12/12. Windows import 32/32 and UI 46/46
passed in both profiles, including header pixels before/after scrolling in
AR/EN/DPI tests. Python 40197393117 and history 40197393070 passed.

The final production evidence ZIP was downloaded and matched provider SHA-256
`f009b0ff6f73dc1d198f671a0dd949677668746241de5f4a1f32fabea4d1e1a0`.
Final actual AR/EN hosted Windows captures were visually reviewed: the scrolling
overpaint observed in source 73e3280 is absent. Both deployed worker reports
imported the expected synthetic JSON finding and source hash. Installer install
and uninstall exited 0; executable and per-user registration were removed.
See hosted-ci.json for job steps, artifact digests/expiry and inspection receipts.
Provider ZIP hashes are not the checksums of the inner installer/portable files.

## Implemented scope

Six native adapters extend protected case analysis: UTF-8 documents, JSON, XML,
CSV, standalone SQLite row samples and Android threadtime exports. The fixed
child parses a separate private working copy, independently checks its source
hash before processing, and the parent verifies the copy afterwards. Findings
retain source references, types/values, limits, truncation and provenance.
Existing protected analyses/audit/backup/PDF persistence is reused.

Native permission, owner/grant, explicit consent and purpose checks apply before
processing; session, case/actor and optimistic revision are rechecked afterwards.
Identity signals can clear the workspace, so captured target checks happen after
authorization refresh. Changing cases/signing out discards pending output.
Revoked grants and stale writers cannot retain results. Success and safe parser
rejection/cancellation are audited while the same case/session still permits a
write. Preflight denial and results discarded after scope/session/revision
changes are not silently committed under a different actor or case.

Bounds: 4 MiB input, five-second child deadline, 64 KiB output, 128 findings,
256 UTF-16 code-unit values and depth 32. SQLite samples the first 32 rowid-ordered
rows/table and eight columns; views/virtual/shadow/WITHOUT ROWID tables are skipped.
No year/timezone/event meaning is inferred from exported logs.

The new AR/EN UI provides purpose/consent, progress/cancel, findings/provenance
and retained-result reopening. Government global progress and exit cancellation
include imports. Explicit header/breadcrumb stacking and pixel invariance tests
address the Windows scrolling overpaint caught during actual screenshot review.

## Local evidence and historical visual failure

Initial integration a02c8e3 passed strict Release and full CTest 41/41 in one run,
native import 32/32, UI 46/46 plus DPI groups and Python 38/38. Final importer
guard passed import/ASan 32/32 (local leak inspection disabled); its full run
passed 39/41 because two generated outputs could not start. Relinking/restoring
output modes passed catalog, and an isolated security retry passed after an
initial fixture child-start/lock failure. Header follow-up passed UI/DPI/native
smoke 4/4. This is not a claimed single final-source local 41/41 invocation;
hosted clean builds each passed 41/41.

Source 73e3280 passed functional/package checks in run 40195552378, but manual
review found overpaint in its Windows capture. Its production evidence remains at
artifact 12600461320 with archive SHA-256
`bbf1178b6dc12d8e2ad8c6f1485833e7a22fbb60b53af37cc184646406c52998`.
It is historical failure evidence, not a final visual acceptance pass.

The initial-local-import-{ar,en}.png images here are genuine local Qt captures
from a02c8e3 with synthetic data. Final Windows PNGs and complete raw reports are
inside [artifact 12601325004](https://github.com/nexvary/NEXVARY-MR/actions/runs/40197393089/artifacts/12601325004).
Its image paths are:
- `build/Release/smoke-professional-ui-authorized-import-ar-1.png`
- `build/Release/smoke-professional-ui-authorized-import-en-1.png`

The execution environment became unavailable after final source publication,
hosted validation and Windows inspection. This receipt was completed through
GitHub APIs; verbose local scratch logs and final standalone PNGs could not all
be copied into Git. Hosted artifacts retain complete raw reports and captures
until the expiry recorded in hosted-ci.json. No private user photograph/serial
is published. No Guardian change or main merge.

## Remaining gates

Child isolation is not OS/network containment. Temporary plaintext removal is
not secure erasure; host crashes can leave traces. Explicit network temporary
roots are rejected, while OS-mounted network volumes remain a deployment
boundary. Real protected cases require Windows current-account DPAPI; Linux
case tests are synthetic. Existing clock/account/storage/backup bounds remain.

No encrypted-backup decryption, vendor container/archive import, WAL acquisition,
PDF/DOCX extraction, locked-phone acquisition, trusted publisher signing, clean
Windows 10/11 or institutional acceptance is claimed. Complete lawful backup/
document adapters next, then UTC timeline, provenance relationships and enforced
offline networking. See docs/AUTHORIZED_IMPORT.md and DEVELOPMENT_STATE.md.
