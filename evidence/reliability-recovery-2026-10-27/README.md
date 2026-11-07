# Reliability recovery evidence — 2026-10-27

Native full suite is source a1facce (same runtime/QML as fabe1b4), Laboratory RelWithDebInfo, Ubuntu24.04/Qt6.4.2 software renderer. This differs from the original Release optimization. See local-verification.json for exact scope. The later helper cleanup changes CI diagnosis only and has its own Python checks; its hosted CI must be verified separately.

- local-lab-full.xml and local-lab-full-last-test.log: serialized final native 48/48, policy15, QML51 and DPI12/12 each.
- initial-lock-fixture-failure.xml: original new-test failure retained. Invalid synthetic event action prevented restart; corrected to existing policy-rejection, without weakening runtime validation.
- local-gui-repeat.*: three bounded successive GUI observations; no transient reproduced or fix claimed.
- lab-asan-pair.txt: fresh Laboratory ASan/UBSan transition pair4/4, local leak checks disabled for ptrace limitations. Required hosted leak checks unchanged.
- prior-production-asan-pair.txt: surviving prior production source-fabe instrumented binary, separately scoped4/4.
- local-gdb-capability.txt: local ptrace denied; no stack.
- local-lab-receipts-ar/en.png: actual local Qt captures of existing native synthetic receipt UI, not a visual fix or Windows acceptance. Images contain synthetic values only.

No raw audit journals, private keys, identities, real case data or private resource copies are included. These passes do not close the original PR SIGSEGV or historical audit-store transient. Original failure records remain in partition-results-2026-10-27. No new government-ready build or superiority claim.
