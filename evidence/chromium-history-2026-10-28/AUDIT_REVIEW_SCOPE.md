# Audit blocker review during History acceptance

Read-only review for22cad419: PolicyAudit::append checks current file digest before
and after acquiring its QLockFile, then only publishes in-memory next events after
save succeeds. save disables direct-write fallback and requires exact bounded
post-replacement readback. GovernmentOperationPolicy::available checks the file;
caseOperationStates/exportAudit enter through that gate. No source change made.

The existing deterministic replacedSnapshotMustReadBackBeforeAcknowledgement test
again passed in current case_transaction suite: an older signed snapshot injected
after replacement refuses acknowledgement and latches the writer unhealthy.
Current case/identity/retention/workspace groups passed. First broader local run
could not execute the generated policy test ELF because its mode was0644; metadata
and original4pass1-notrun XML/log retained. Added only its own execute bit, verified
ELF SHA unchanged, then the policy group passed separately. The failed aggregate
is not relabelled5/5. This is an environment observation, not an audit source fix.

No mechanism/root cause for the historical signed disk prefixes18/34 versus
in-memory22/41 has been proven. Readback verifies a commit observation, not power-loss
durability or later rollback immunity. Original private journals/keys remain
excluded. The root issue, process descendants and Qt SIGSEGV remain OPEN and are
not causally linked by this review.
