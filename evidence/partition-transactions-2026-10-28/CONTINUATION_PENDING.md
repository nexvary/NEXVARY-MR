# Corrective source checkpoint awaiting new CI

The commit containing this entry follows source7dc5fa913c678372c46d6f7a0373b27233b7edde.
Resolve its actual current SHA/tree with Git. The earlier source is retained in
FIRST_SOURCE_CONTINUATION.md and first-ci-observation.json; it is not acceptance
of this changed source.

Corrective changes: strict portable MBR Hex bytes, three stable summary slots,
a four-cycle real UI identity/frame regression, and an extension of the existing
one-shot CI diagnostic to synthetic main smoke on a private copy. Existing
Columns, old crash/negative tests, timeouts, warnings and fail-closed gates stay.

Local: corrected native28/28; full Qt54 and DPI15 each passed, plus focused
summary3/3 at both DPI settings and Python42. The local main smoke then produced
a2.65s SIGSEGV; local GDB cannot trace. One native-stack diagnostic instead
reproduced original audit-digest failure: memory139 versus valid signed disk135.
These failures stay OPEN, not green. Private originals and exact ELF retained
outside public Git (archive checksum in FIRST_FAILURES.md).

No accepted Windows package or new Push/PR result exists yet for this corrective
commit. Inspect new exact-source events separately, preserve first failures,
verify PR checkout tree and actual package/deployment receipts before final
checkpoint. No main merge, branch deletion or Guardian changes. No general
crash-free, OS/network/descendant containment or institutional/superiority claim.
