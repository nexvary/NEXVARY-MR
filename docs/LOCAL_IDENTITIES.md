# Local identities and signed research/audit records

Development scope: local credentials inside one Windows OS account. The organization
must verify who receives credentials. This is not agency SSO, an external certificate
authority, a trusted timestamp service, or proof that two accounts represent two people.
Existing repair/workshop pages do not require this case/research session.

## First use

Open Law Enforcement → Sign in / accounts. Create the first administrator with a
chosen lowercase username and password (14 or more characters, at most 256 UTF-8
bytes). No production account/password is supplied. Sign in as administrator and
create investigator, reviewer, auditor, trainer or repair-technician accounts.
Administrators manage accounts/grants but cannot automatically read case contents.
They can explicitly grant an existing case UUID to a permitted case-reading role.

Investigators create cases. The creator UUID is stored atomically in the protected
case payload and owns access; no second-store grant write is needed for creation.
An administrator can grant/revoke other accounts by case UUID. Role/action policy
still applies: a grant never gives an auditor edit rights. Disable an owner account
to stop its access; owner access is not a removable secondary grant. Legacy cases
without an owner require an explicit grant; no automatic ownership migration occurs.
Missing identities fail closed. Retain protected backups under the original Windows
account; deleting the registry does not transfer ownership or recover signing keys.

Sign-out clears case/device/audit/training views. Sessions expire after 15 minutes
of total elapsed time, using a monotonic timer. Every case mutation/read request
rechecks registry state; the UI polls at one-second intervals to clear revoked views.
Five incorrect password attempts persistently lock a known account for five minutes.
The lockout clock is local and not resistant to an OS administrator changing time.

Self-service password rotation requires the current password, rewraps the same
signing key and ends current/other sessions. Earlier signatures remain verifiable.
There is no administrator recovery of a forgotten password/private signing key:
disable that account and provision a replacement under organizational policy.

## Cryptographic/storage boundary

The already pinned, hash-verified libsodium 1.0.22 distribution supplies Argon2id
interactive password hashing and a separate salted password-derived key wrapping
an Ed25519 private key with authenticated secretbox encryption. The registry itself
uses the existing Windows current-user DPAPI provider, atomic writes, owner file
permissions, bounded input, a writer lock and optimistic revisions. Signing keys
are released from sodium-managed guarded memory on logout. Temporary native password
and derived-key buffers are cleared; this is not a guarantee that Qt/OS memory has
no other copies. Production identity storage refuses on Linux; native synthetic
fixtures are explicitly separate and contain no real credentials or case data.

An attacker who controls the current Windows account/process or application binary
is outside this local credential boundary. OS permissions, account separation,
backups, institution identity proof and independent key custody remain deployment
requirements. No custom cryptographic primitive or plaintext production fallback
was added. The official Windows DLL and ISC notice are included by packaging.

## Independent research review

The signed-in investigator/reviewer records a **public or synthetic** observation.
Its author account signs the complete immutable observation and evidence hash.
A different authenticated reviewer selects the original artifact, supplies a decision
and notes, and signs a domain-separated review that includes the result digest and
artifact digest. Self-review, mismatched evidence, duplicate review, forged author
content and recomputed-checksum promotion are rejected. Reload verifies reviews
against public keys pinned in the protected local registry. Disabled/untrusted
signers fail verification; an affected ledger is not silently rewritten.

The review scope remains public/synthetic. Approval never sets `Lab Verified` device
compatibility or commercial support. Direct compatibility promotion stays denied.
Physical repeatability on an exact model/OS/firmware, implementation acceptance,
external review and the existing native capability policy remain separate gates.
This does not claim independent human/institution verification from account names.

## Retained case audit anchors

A permitted, authenticated case reader can explicitly export a new signed anchor
containing case UUID, revision, audit head and verified evidence hashes. Retain it
outside the case database. Comparing it later verifies the pinned local signature
and audit prefix. A current database older than a retained newer anchor, or a
conflicting prefix, fails verification. A newer intact history can match an older
anchor, but that older anchor cannot detect deletion of events newer than itself.

Signatures use local credentials and local clock labels. They do not prove physical
handover or trusted external time. Replacing both the database and all retained
anchors/identity registry with old backups is not detected. Independent institutional
key custody, external timestamping and retention policy are not implemented.

## Validation

`mr_identity_tests` exercises credential persistence, account roles/grants, cross-session
revocation, persisted lockout, password rotation/session invalidation, signature
preservation, independent research artifact review and forged-content rejection,
and retained-anchor rollback detection. Qt Quick tests use authenticated synthetic
investigators for case workflows and verify sign-out/admin isolation through real
buttons. Existing case/research/repair regression tests remain in CTest; Linux CI
also instruments the identity suite with address/undefined-behavior sanitizers.
