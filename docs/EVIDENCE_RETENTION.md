# Evidence retention and protected in-place archiving

Development implementation inside the existing Government workspace. No scheduled
or manual evidence-byte deletion is implemented. An approved disposal request is a
review record, not permission to bypass the separate institutional disposal gate.
No evidence, original, case, protected payload or backup is removed by this engine.

## Policy and permissions

Open a permitted case and choose **Retention & archiving** from its dashboard or
section selector. A reviewer creates an immutable policy with a label, retention
of 1..36500 days, an archive threshold of 0..retention days, and a competent policy
reference. Reviewers can assign an existing policy to a case evidence item; later
assignments append history rather than rewriting policies. The final assignment
is effective. These are recorded institutional declarations; the application does
not choose a legal retention duration or verify authority documents.

Investigator/reviewer accounts with a current case grant may place a **legal hold**
or **preservation exception** on evidence. Either blocks disposal eligibility
regardless of expiry. Only a reviewer may release a specific restriction, with a
reason and authority reference. Released restrictions remain in protected history.
Restrictions and policies can be recorded for closed, archived or suspended cases
without reopening their case metadata or evidence editing routes.

The assessment uses evidence registration UTC plus policy days, not file creation,
EXIF or device time. Exact expiry is inclusive; dates before registration and
invalid clocks fail closed. Leap days are handled by Qt UTC date arithmetic.
The local OS clock is not an independently trusted timestamp. Local timezone or
DST changes do not change UTC retention arithmetic. No policy means ineligible.
This application does not purport to determine legal admissibility or entitlement.

## Archive rules and verification

Archive is an explicit action, never a background relocation or file deletion.
Once the assigned archive threshold is reached, it verifies the protected stored
bytes and appends an immutable inventory receipt: evidence/device UUIDs, policy,
SHA-256, size, actor and UTC time. This is **protected-in-place** archival: no
second filesystem repository, cold-storage migration or WORM hardware is claimed.
A hold prevents disposal while allowing preservation archiving and backup.

Verify Archive reads and decrypts the protected evidence again and compares its
recorded hash and size with the archive receipt. A missing/corrupted blob fails
closed. It does not establish the integrity or survival of every external backup.
Receipt verification is a read-only check; policy/hold/archive/disposal mutations
append to the existing hash-linked case audit and its independently retained
signed-anchor mechanism. OS-account compromise and whole-store rollback still
require the pre-existing external anchor/key custody procedures.

## Authorized disposal review

An investigator may request disposal only for expired assigned retention in a
closed/archived case, with no active restriction and a verified evidence payload.
The request requires a reason and authority reference and pins the effective policy
and source hash. A reviewer with a separate authenticated local account and current
case access may approve or reject once. The same actor cannot review their request;
synthetic unauthenticated bridge roles cannot approve disposal. Local accounts are
not proof of two independent human reviewers or agency-verified personnel.

Approval rechecks the current policy, case status, active holds/exceptions and
source integrity in the same optimistic SQLite transaction. Stale sessions/writers
and revoked grants fail closed. A newly placed hold or reassigned policy blocks
approval. A rejected request remains available for audit. An approval does **not**
remove evidence, invalidate a new hold, schedule a purge, purge external copies or
assert the continuing validity of a disposal decision. Actual disposal requires a
separate reviewed implementation, institutional legal/records policy and approval.

## Storage, migration and recovery

The extension is `retention.schema = 1` inside the existing protected case JSON.
No plaintext SQL retention index or evidence-path column is added. A legacy v1
case without the optional extension opens unchanged and assesses as no policy.
The first authorized mutation creates the extension atomically with its audit
entry; older case/evidence UUIDs, bytes, ownership/grants and audit prefix remain.
Invalid extension schemas are rejected on load/restore. Old application versions
must not administer retention-enabled cases; operational downgrade is unsupported.

Protected backups retain policies, assignments, restrictions, archive receipts,
requests, decisions and their complete audit. Restore retains restrictions and
never restores by overwriting an existing case. Identity registry, original DPAPI
context and separately retained backup digests/anchors remain required; see
PROTECTED_CASE_BACKUP.md. All retention actions use existing revision checks and
SQLite rollback. PDF reports include retention records and retained inputs.

Bounds: 200 entries per retained collection and 512 KiB for the extension; existing
case 3 MiB write / 10000 audit events, evidence and backup bounds still apply.
Reaching a bound fails explicitly, never trims history or releases restrictions.
No automatic maintenance process or network service is used by this engine.

## Verification

`mr_retention_tests` exercises legacy migration/reopen, protected backup/restore,
exact expiry and leap-day boundaries, holds/exceptions, role/grant rejection,
closed/archive governance, malformed fields, stale writers, failed-transaction
rollback, corrupted archive bytes and independently authenticated review that
rechecks a new hold. QML tests exercise policy creation/assignment, archive, hold,
native button states and actual AR/EN application captures at desktop/DPI sizes.
The existing regression suites are preserved. Hosted Windows/Linux profiles and
production ASan/UBSan include the new suite. These synthetic checks do not establish
institutional approval, field readiness, trusted time or physical device acceptance.
