# Evaluation scope freeze and permission-owner fixture

Parent government4b9bd65 and integration5a4efc9; merge-basea23c9d8.
Published government Push41447294513 and PR41447305711 both failed the Linux
permission-transition row, while Windows jobs passed. This is not the original
135/139 audit lag root. After replacement denial, QLockFile cannot unlink its
lock. A same-process constructor on normal procfs correctly sees its owner alive
and refuses: the old test incorrectly treated that as restart.

Move the permission-denial write into a bounded subprocess. Preserve replacement
refusal, unchanged protected bytes/anchor, unhealthy writer, no silent retry and
retained lock. Only after actual child exit must a fresh reader recover and an
explicit new append extend the exact old history. No runtime lock removal or
failure-policy relaxation. Original Linux/Windows platform exclusions remain.

The first new local extra assertion of same-process lock refusal failed under
this host's ancestor procfs namespace (exit85). That extra assertion was not in
the published parent and is not portable to this namespace; replace it with
proof the original lock still exists, then test actual process exit/recovery.
The final corrected row passes3/3 including setup/cleanup. permission-original.txt
shows the old row passed locally despite failing on hosted normal procfs.
No historical135/139 root cause is inferred from this different discrepancy.

Fresh Qt6.4.2 official Ubuntu packages extracted to a private prefix; pinned
libsodium1.0.22 built with the unchanged reviewed helper. First full build left
an empty non-executable generated worker despite reported linker completion.
The first native run was interrupted and its output retained. Artifact-only
serial relink restored executable bytes; no source workaround or chmod of an
empty output. Full subsequent native results are tracked separately, not erased.

A separately authored two-record JSON truth corpus has8 typed scalar findings.
The direct existing worker check verifies exact paths/values/types/order, source
SHA256/size, deterministic report bytes, and refusal of wrong hash/duplicate keys.
No missing/unexpected findings on that corpus. It does not prove product UI,
case authorization, timeline/PDF/restart, devices or competitor parity. Registered
as a new CTest reference group without changing existing assertions/deadlines.

Frozen scope and6 finite blockers: docs/GOVERNMENT_EVALUATION_RELEASE.md.
NTFS product linkage and new engines are explicitly deferred. Actual earlier
serialized UI logs are preserved separately and remain tied to their predecessor.
Current source-specific hosted/package and field gates remain pending at commit
preparation. Private keys/original protected stores/real cases are not published.
