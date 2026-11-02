# Legacy recovery completion acceptance

This correction applies to existing RecoveryBridge local recovery/research
containers, not the government LawCaseBridge transaction protocol. No new engine,
format, acquisition or NTFS product enablement is included.

A deterministic existing-source counterexample completed an EXIF computation
without delivering the queued GUI callback. The worker had already saved a new
evidence entry, so a subsequent GUI cancellation could not prevent persistence.
The existing immediate-cancel EXIF regression also failed intermittently.

Workers now return candidate documents without writes. The existing GUI callback
checks original document/path and cancellation before persistence. Explicit cancel
keeps the original case bytes, evidence and audit untouched; late completion
cannot produce a saved success. Noncancelled parser rejection still retains the
existing failure audit, and successful open retains the existing opening audit.
Separate audited partition acceptance keeps its final permission/receipt gate.

No rollback/repair/replay or crash atomicity claim is added. Case writes are now
bounded synchronous callback work in this legacy workspace and can affect its
GUI responsiveness; the government transaction/performance gates remain separate.
Existing busy-state export refusal is preserved. Qt threadpool completion tests
exercise the actual native worker/callback ordering and fresh reopen. Actual
source, raw failures, checks and pending hosted/field gates are recorded in
DEVELOPMENT_STATE.md and evidence/evaluation-cancel-2026-11-02/.
