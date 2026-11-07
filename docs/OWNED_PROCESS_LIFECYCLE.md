# Owned worker lifecycle — 2026-11-02

This mechanism addresses controlled detached descendants of product-launched
workers. It is not a process/network sandbox or hostile same-user isolation.
The source was reconstructed from recorded edits after the local execution
environment became unavailable. Exact-source results and package-source pins
are recorded separately in DEVELOPMENT_STATE.md; preceding local observations
and older Windows packages do not accept later source changes.

## Product boundary

OwnedProcess replaces the native QProcess instances in TskImage, SecurityBridge
(ClamAV/FreshClam), EvidenceImportRunner, ToolBridge foreground/discovery/device
services, and MirrorSession preflight/viewer. Existing fixed arguments, private
copies, input/output/time limits and original-request/late-result gates remain.
No external effect is replayed or undone; unproved cleanup refuses success.
No signed journal operation-kind wildcard or duplicate forensic engine is added.

Windows uses a fresh non-inheritable Job with KILL_ON_JOB_CLOSE, no breakaway
permissions and a 512-active-process limit. PROC_THREAD_ATTRIBUTE_JOB_LIST in
STARTUPINFOEX assigns the worker as part of CreateProcess, before its code can
create descendants. Job/attribute failure refuses creation; there is no late
assignment fallback. Stop terminates the Job; completion also cleans descendants
and queries membership for at most 2 seconds. Owner crash closes its Job handle.
Windows10-era API/nested-job compatibility requires exact-source hosted tests
and separate physical/agency-host review, not inference from documentation.

Linux launches the exact sibling mr_process_supervisor, without a PATH/helper
environment override. A dedicated subreaper is set before fork. The worker gets
its process group and parent-death signal before exec and /dev/null as stdin.
The supervisor alone retains the product stdin control/death pipe. EOF, stop
signal or root exit initiates cleanup; product SIGKILL leaves the supervisor
alive to adopt double-fork/setsid descendants, stop/kill owned unreaped children
and the original owned group, then reap for at most 2 seconds.

PID ownership is checked against the mounted procfs parent namespace, and NSpid
is translated before signalling. The local test host exposes ancestor procfs
and omits task/children. The fallback streams at most 65536 PID entries, filters
PPid before retaining namespace IDs and retains at most 4096 owned children /
32 namespace levels. This single thread never reaps between ownership inventory
and signalling, preventing reuse of those unreaped child PIDs. No unrelated
enumerated process is signalled or its metadata included in a receipt. Failed
inventory/cleanup remains unproved. No cgroup is created: local root was not
writable; cgroupUsed=false is explicit. Process groups alone do not stop setsid.

## Receipts and authority

The bounded nexvary-process-cleanup/v1 runtime observation records mechanism,
cleanupComplete, cause class, observed/reaped or remaining counts and failures
where available. It contains no case title/path, report bytes, credential or key.
Missing observations are false/unknown. TSK returns observations for its two
invocations; the government adapter strips internal observations before canonical
report replay and persistence, preserving the forensic report schema and kinds.
Security reports carry observations; ToolBridge, MirrorSession, RecoveryBridge
and LawCaseBridge expose observations without changing QML layout.

These observations are not separately signed durable forensic evidence. Existing
signed operation status and retained case protocol remain separate. A killed
product cannot publish a fresh in-product receipt: the parent-crash test uses
an independent marker witness. Restart is not proof of orphan cleanup; an
unproved shutdown requires OS/operator review and refuses the current result.
This mechanism does not discover arbitrary pre-existing processes on restart.

Cancel, logout, session close/expiry and permission withdrawal keep their native
propagation and original-request binding. New grants do not authorize old work.
Explicit case-import cancellation invalidates its persistence context before
requesting worker shutdown: a late callback retains neither a successful result
nor a rejected parser report. Ordinary parser rejection during an authorized
request remains auditable. Tests compare both live and freshly reopened case
snapshots, including revision, analyses and audit, before/after cancellation.
A native shortened identity lifetime applies solely to synthetic storage fixtures;
production remains fixed at 15 minutes. Synthetic expiry exercises the actual
checkSession/timer path, not a real 15-minute field-session acceptance claim.

## Retained negative boundaries

The new published-source marker probe linked against unchanged cc23ccb TskImage
(runtime bytes from 5e0ac5b) observed cancellation followed by detachedSurvived=1.
OwnedProcessTests keeps raw QProcess direct-kill escape as a negative control and
tests owned cancellation, shutdown labels, timeout, normal root exit with a
detached child, repeat use, failed launch and actual product-parent SIGKILL.
OfflineTests separately exercises real policy revoke/cancel/logout, approved
grant expiry and synthetic identity session expiry, signed terminal status and
zero retained-result assertions.

Isolated Linux deployment probes copy the test executable into a fresh directory
and cover absent, non-executable, symlinked and zero-exit/no-receipt supervisors.
Each refuses cleanup proof and must not launch the marker worker. The build's
shared helper is never renamed or modified. Windows uses creation-time Jobs,
so these Linux sibling-deployment rows are explicitly excluded there.

On Linux, a hostile worker with the same user identity can kill its supervisor
and leave a detached descendant. A retained negative requires cleanupComplete=false
and verifies that escape. Stronger hostile-worker isolation requires a separately
enforced privileged boundary/cgroup and independent review. Uninterruptible kernel
tasks, kernel/Job tampering, privilege changes, unrelated processes, network
egress and simultaneous supervisor/kernel failure are not proved contained.
Receipts never convert a missing cleanup proof into a successful result.

See evidence/process-ownership-2026-11-02/ for the recovered probe/observations
and the explicit source/availability limits. No new visual acceptance is claimed.
Original Audit Store prefix lag, Qt/DPI root/soak, full benchmark and NTFS product
binding remain independent open work. CI/physical signing/institutional gates
remain distinct; Guardian, main and upstream repositories are untouched.

Primary references checked 2026-11-02:

- https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-updateprocthreadattribute
- https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects
- https://github.com/qt/qtbase/blob/v6.8.3/src/corelib/io/qprocess.h
- https://man7.org/linux/man-pages/man2/PR_SET_CHILD_SUBREAPER.2const.html
- https://man7.org/linux/man-pages/man2/PR_SET_PDEATHSIG.2const.html
- https://man7.org/linux/man-pages/man5/proc_pid_status.5.html
