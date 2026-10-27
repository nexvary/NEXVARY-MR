# Protected case backup and restore

Development feature within NEXVARY-MR. It snapshots one case and its registered
protected evidence, not the complete workstation, identity registry or operating
system. Production uses the existing Windows current-user DPAPI provider; Linux
real-case storage fails closed. Non-encrypted synthetic fixtures are explicitly
labelled and are not a production fallback.

## Backup

1. Sign in as an investigator or reviewer with ownership/a grant for the open case.
2. Open Reports & Audit, confirm protected backup, and choose a **new** .nmrbackup file.
3. Retain the displayed SHA-256 separately through an approved channel. The source
   case also records the digest, backup UUID, source revision and audit head.
4. Retain a signed audit anchor separately when required by the institution.

Every evidence blob is decrypted and checked against its recorded size/SHA-256
before inclusion. The snapshot contains the existing protected case/evidence blobs
inside a second protected container. Account credentials/signing keys are excluded.
Combined protected payloads are limited to 32 MiB; the container is limited to
64 MiB. Oversized cases are refused without silently omitting evidence. Each case
has at most 100 recorded backup exports. Existing output files are never replaced.
Case status may be draft, suspended, closed or archived; preservation backup is
allowed without enabling editing or device operations in those states.

The snapshot records the case state **before** the backup-export audit event. The
source case appends that event and its file digest after output succeeds. Normal
failures roll back the source database and attempt removal of a newly created file.
The file and database do not form one crash-atomic filesystem transaction: a power
or process failure may leave an uncommitted protected container. Review it against
separately retained receipts/anchors; do not infer completion from a filename.

## Restore

An investigator needs the original supported Windows protection/recovery context,
the original identity registry or an explicitly provisioned case grant, and the
separately retained backup SHA-256. The native API compares the supplied digest;
it cannot prove that a value was retained independently rather than calculated
from the same untrusted file. A matching digest is not institutional provenance.

Choose the backup in Reports & Audit after supplying its digest and confirming
restoration. The application verifies the container/profile, protected source case,
audit chain, complete device/evidence inventory, unique identifiers and every
registered file digest **before** database insertion. A current identity/grant and
native RestoreCase permission are checked against the decoded case itself.

Restoration never overwrites an existing case or evidence identifier. One SQLite
transaction inserts the case and all protected blobs; a conflict/write failure
rolls back all new rows. Case UUID, owner UUID, metadata, device/evidence identifiers
and original evidence bytes remain unchanged. A recovery record and audit event
are appended to the recovered state. Closed/archived status is retained.

A restored snapshot is a recovery point, not proof of the latest history. An older
retained signed anchor can match its preserved prefix. A newer/different anchor
must report an older or divergent history, including when the new recovery event
occupies a revision formerly used by the source's export event. The application
does not silently treat that difference as accepted institutional recovery.

Do not delete an existing case merely to bypass the no-overwrite rule. Preserving
originals, selecting a separate approved recovery database/environment, validating
OS-account recovery and authorizing any operational replacement are deployment
procedures. This feature does not merge cases or restore lost account signing keys.
Cross-account/machine DPAPI recovery needs its actual supported Windows/provider
process; no generic portability or disaster-recovery guarantee is claimed.

## Security and validation boundary

Backup/restore are native, separate actions. Investigators can back up and restore;
reviewers can back up; auditors/trainers/repair technicians cannot export backups or
restore cases. Ownership/grants are never changed by restoring a backup. The
container has no extraction paths or executable entries; protected blobs are bound
to SQL identifiers, not restored as arbitrary filesystem paths.

Windows file permission bits are not an independently managed NTFS ACL policy.
Choose an institution-approved destination and preserve the Windows protection
context and identity registry with supported OS-level backup procedures. Filenames
and filesystem metadata are outside the encrypted container. Local-account/process
compromise, agency SSO, external signing/time custody, physical disposal execution and
full operating-system/account recovery remain separate deployment requirements.

Native tests cover round-trip preservation, wrong digest/consent/role/grant,
revocation, immutable status, duplicate case/evidence identifiers, missing/duplicate/
corrupt blobs, malformed containers, size limits, stale writers, forced write
rollback, original ownership and comparison with retained older/newer anchors.
Actual cross-Windows-account recovery and clean Windows 10/11 field acceptance are
not inferred from synthetic or same-account hosted tests.
