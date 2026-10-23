# Offline signed-update verification library

`mr_update_verifier` is an intentionally internal C++/Qt Core library, tested in
both desktop build profiles. It is not linked into the desktop application and
provides no download, install, execution or device-write entry point. This
increment does not make stage 57 or the secure updater complete.

## Trust and protocol

The caller supplies a reviewed `TrustedKeys` mapping, a trusted installed version
and sequence floor, and trusted current time. No production key, key-discovery
endpoint or private key is embedded. Empty trust stores fail closed. A key named
in the manifest only selects an already approved key; it cannot supply one.

Sign the exact bytes of `NEXVARY-MR update manifest v1\n` followed by the manifest
with **pure Ed25519 detached signing**. The signature is 64 raw bytes. The manifest
is at most 16 KiB and must round-trip byte-for-byte through Qt's compact JSON
serializer. This is a deliberately restricted compact ASCII string-field profile,
not RFC 8785 JCS. Duplicate keys, whitespace and alternate escapes are rejected.
The independently signed Python cryptography fixture in `samples/update/` checks
interoperability with Qt 6.4 on Linux and the configured Windows Qt build. Its
public key is test-only; its randomly generated private key was never serialized.

Exactly these string fields are accepted:

| Field | Policy |
| --- | --- |
| schema / product | `nexvary-update/v1` / `NEXVARY-MR` |
| platform / channel | `windows-x64` / `stable` |
| profile | Must match the compiled production-read-only or laboratory-experimental profile |
| keyId | Approved caller key identifier, at most 64 ASCII characters |
| version | Three canonical decimal components, each 0–65535; strictly newer than installed |
| sequence | Canonical unsigned 64-bit decimal; strictly above the supplied floor |
| packageKind | `portable-zip` or `installer-exe` |
| packageBytes | Canonical decimal, 1–536870912 |
| sha256 | Exactly 64 lowercase hex characters |
| issuedUtc / expiresUtc | Whole-second ISO UTC `Z`; issued <= now < expiry, at most 31 days validity |

All values are bounded to 128 characters. Unknown fields, unapproved keys, altered
signatures, stale versions/sequences, wrong targets/profiles and invalid time
windows fail without retaining an authenticated-looking report.

`checkPackage` authenticates and applies manifest policy **before opening a
package**. It accepts a local regular input through the shared native file
policy, hashes bounded chunks with cancellation, checks declared length and
post-read changes, and compares SHA-256. It never changes the source. Reports
exclude the local input path. File URIs are accepted; explicit network paths and
non-file URLs are rejected. OS-mounted network filesystems are not distinguished.

## Owned staging copy

`stagePackage` authenticates before opening a source or creating any output. It
requires an existing, caller-approved local staging parent and creates a fresh
owned temporary child. It copies and hashes bounded chunks, flushes, then hashes
the actual stored bytes before returning a move-only `StagedPackage`. No source
or staging pathname is exposed by the object or its report. Its retained file
handle is reused by `recheck(currentPolicy, trustedNow)`, which reauthenticates
with the latest supplied trust anchors/version/floor/time and rehashes that same
handle. Replacing the original source does not affect the staged bytes.

RAII closes the handle before removing only the owned child directory. Source
and unrelated parent files are preserved; failed hash, partial copy and mid-copy
cancellation remove temporary outputs. POSIX staging children and files are
owner-only. Windows relies on the approved parent's inherited ACL; this increment
does not establish or audit a hardened Windows DACL. Parent path resolution is
not anchored by a directory handle against a malicious same-user process. The
object is used by one worker at a time and is not a persistent journal.

The success flags `stagedCopyCreated` and `retainedHandleVerified` indicate these
specific checks. They do not grant an installer capability. `immutableSnapshotCreated`
and `installationAuthorized` remain false. The caller must not persist an old
successful report as authorization: a newer floor, revoked key, expiry or altered
copy makes a later recheck fail. No package pathname is offered for execution.

## Limits before installation can be enabled

A successful result means these observed bytes match an approved signature and
hash under the caller's trust assumptions. It does not certify publisher identity,
ZIP/PE validity, Authenticode, OEM firmware compatibility or installation safety.
The mutable source pathname is not an installable capability. Same-size concurrent
changes or later path replacement are not protected by a snapshot. The owned temporary copy is not OS-enforced immutable storage. There is no
protected/persistent sequence floor, clock rollback defense,
key rotation/revocation workflow, atomic install journal or interruption recovery.
Every success explicitly reports `installationAuthorized=false` and
`rollbackStatePersisted=false`; package results also report `installed=false`.
A future installer must add durable private staging/journaling, crash-leftover
cleanup, and revalidate the retained snapshot under current policy while enforcing these
remaining gates; it must not execute the input path based on this result.

## Dependency and build

libsodium 1.0.22 (ISC) supplies Ed25519; no custom cryptographic primitive is used.
Both official point-release archives were verified with upstream minisign and
then pinned by SHA-256 before extraction/build. CI repeats the pinned hash check,
not minisign. The exact sources, review key, signatures and hashes are recorded in
`samples/update/dependency-provenance.json`. Linux prepares a static PIC library
and runs upstream `make check`; Windows selects only official x64 Release v143
dynamic import/runtime files. The test executable owns its adjacent dependency
DLL; the desktop portable/installer does not depend on it. No CMake or application
implicit download occurs. A future integration must deploy and audit this runtime.

```sh
python3 scripts/prepare_sodium.py --destination "$PWD/build-deps/sodium"
cmake -S . -B build -DMR_BUILD_GUI=ON -DMR_BUILD_UPDATE_VERIFIER=ON \
  -DMR_SODIUM_ROOT="$PWD/build-deps/sodium" -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

A normal desktop build leaves this unused internal component disabled by default;
native CI explicitly enables it in both profiles and the sanitizer build. A
core-only build remains dependency-free by default. For verifier-only testing,
use `MR_BUILD_GUI=OFF` and `MR_BUILD_UPDATE_VERIFIER=ON`. Build dependency receipts,
upstream checks and native results are retained by CI. RAII cleanup covers normal cancellation/failure/destruction, not abrupt process
termination or power loss. No physical phone, Windows
10/11 client, production signing key or end-to-end update acceptance is implied.

## Authenticated atomic transaction journal

`UpdateJournal.hpp` adds a separate internal ledger to the same native library.
It records checkpoints, not installer commands or proof that files were installed.
The caller supplies an existing approved local private directory, an independently
provisioned random 32-byte authentication secret, and a `JournalAnchor` containing
an opaque installation identifier, trusted installed version/sequence and minimum
journal revision. Empty/invalid/all-zero keys reject; no default key is generated,
serialized or embedded. The object's private key copy is zeroed on destruction;
the caller must protect and clear its own secret. The secret must not be reused
as an update signing key.

Each bounded compact JSON checkpoint is authenticated using libsodium
`crypto_auth` (HMAC-SHA512-256), with a journal-specific domain and installation/
build-profile binding. MAC authentication does not encrypt metadata. The record
contains version/floor/revision/phase and selected candidate hashes/size/kind;
source/staging paths and keys are absent. Malformed, noncanonical, unauthenticated,
wrong-scope, out-of-range or internally inconsistent records fail without reset
or an apparently successful report. Initialization refuses every existing target,
including corrupt state. A missing file with a nonzero trusted revision floor
cannot silently bootstrap a new history.

All readers/writers cooperate through a bounded nonblocking `QLockFile` acquisition
with age-based lock stealing disabled. A revision compare-and-swap rejects stale
writers. `QSaveFile`, with direct-write fallback explicitly disabled, commits a
new whole record by replacement; write/commit failure returns an error rather than
falling back to in-place overwrite. An object also remembers its highest observed
revision and installed state. An uncertain I/O result must be followed by a fresh
read, not a blind retry or reset. Only the fixed owned state/lock names are used;
unrelated files and software installation contents are untouched.

| Checkpoint | Permitted next action |
| --- | --- |
| idle | Prepare a currently approved and rehashed `StagedPackage` |
| prepared | Recheck the exact candidate, then record applying; or discard before execution |
| applying | Record verifying; or record an independently verified restoration of the exact baseline |
| verifying | Record an independently verified candidate outcome, or verified baseline restoration |

A reopened pending transaction remains pending. It does not launch or resume an
installer automatically. Applying requires the exact prepared candidate's manifest
hash and package metadata plus a fresh signature/time/key/hash check. Confirmation
also rechecks that candidate and requires trusted native postcondition observations
matching its version and sequence exactly. Only then does the ledger checkpoint
the newer installed version/floor. Recording restoration preserves the baseline
floor; it cannot lower a committed floor. These methods **record supplied trusted
observations**; they neither inspect installed program files nor perform a rollback.
`postconditionVerifiedByJournal`, `protectedAnchorPersisted` and
`installationAuthorized` explicitly remain false.

A MAC cannot detect replay of a previously valid checkpoint by itself. The trusted
revision/version/floor anchor must live in independently protected state and be
advanced by the future orchestrator. The RAM high-water mark protects only the
current object lifetime. A test explicitly demonstrates that reopening with an
old/unprotected anchor can accept an old valid MAC; this is not described as
complete anti-rollback protection. OS key provisioning, anchor persistence/ordering,
Windows DACL and directory-handle hardening remain integration gates. POSIX rejects
parents owned by another user or writable by group/others. Explicit network paths
are rejected, but mounted/virtual filesystems cannot all be identified; the caller
must select a supported private local filesystem. QLockFile's cross-host/container
limitations apply. A malicious same-user process or administrator is not excluded
by these primitives alone.

Atomic checkpoint replacement and process-restart recovery are tested separately
from power-loss durability. The suite kills real owned children while holding a
lock, before a QSaveFile commit, and after a journal prepare commit; then reopens
and validates state. The before-commit child exercises the underlying QSaveFile
primitive explicitly, not an injected production crash hook. Two real writers
race on the same revision and exactly one may commit. Test-only ephemeral MAC keys
travel over stdin, never command-line arguments, environment variables or files;
hex framing avoids Windows CRT text-mode transformations. No failure injection or
pause switch is added to the desktop application.

There is no power-cut/filesystem certification, guaranteed directory fsync on all
platforms, cross-file atomic installer, persistent package store, crash-leftover
collector or OS-protected anchor yet. `powerLossDurabilityGuaranteed` remains false.
The journal and verifier are intentionally internal and do not enable an update UI,
unattended installation, production trust anchors or stage 57/60 approval.

References: [Qt QSaveFile](https://doc.qt.io/qt-6/qsavefile.html),
[Qt QLockFile and its filesystem limitations](https://doc.qt.io/qt-6/qlockfile.html),
[libsodium secret-key authentication](https://doc.libsodium.org/secret-key_cryptography/secret-key_authentication).

## Windows-protected local journal key

`UpdateKeyStore` can explicitly bootstrap a local journal MAC key and construct a
`Journal` without returning decrypted key bytes to its caller. This is **not** a
publisher signing key or an approved NEXVARY update trust anchor. The backend uses
the project's existing current-user DPAPI provider in `CaseVault.cpp`; CMake now
builds that unchanged provider once as `mr_user_protection`, shared by the desktop
case workflow and the optional update library. Existing recovery-case formats and
synthetic-case behavior are unchanged.

Creation requires a trusted bootstrap anchor with revision floor zero and a valid
installation identity/version. It acquires the journal lock and refuses any existing
key or journal, including corrupt files and symlinks. A random libsodium MAC key is
wrapped with a fixed purpose, installation identity and compiled profile, plus an
inner SHA256 consistency check, then protected with Windows current-user DPAPI.
Only the sealed `update-auth.dpapi` checkpoint is committed through QSaveFile with
no direct-write fallback. Decrypted temporary buffers and the journal's private
key copy are cleared after use. No key is returned in a report or written in
plaintext/hex to disk. No key creation occurs simply by opening or reading state.

`openJournal` bounds and validates the sealed file, invokes DPAPI integrity/account
checks, then validates the decrypted purpose/profile/identity/length/checksum before
constructing the native object. A successful handle means its key was loaded; the
caller must still call `read()` or explicit first-time `initialize()` and enforce
the independently protected revision/version/floor anchor. Missing, damaged,
wrong-scope or inaccessible keys fail; existing journal state prevents silent key
regeneration. An incomplete first bootstrap (key exists, no journal yet) can reuse
the protected key with the trusted original bootstrap anchor. Existing state is
never reset to recover a lost key.

Linux, including Laboratory, refuses this backend before creating any file; there
is no synthetic or plaintext key fallback. Linux tests assert that refusal. The
Windows-only cases test actual same-user DPAPI round-trip into a MAC-authenticated
journal, no overwrite/regeneration, scope/corruption/size/format rejection, protected
bootstrap-floor refusal and live journal-lock exclusion. These checks must pass on
hosted Windows and remain separate from cross-account, roaming-profile, lost-key,
clean Windows 10/11, filesystem ACL and power-cut acceptance.

DPAPI normally binds decryption to the Windows logon identity and machine; roaming
profile exceptions apply. It is not protection against arbitrary code running as
that same user or an administrator, and it does not establish publisher identity.
The existing provider's encrypted wrapper is reused; the additional inner purpose,
profile and identity checks prevent confusing recovery-case plaintext with a journal
key. The inner digest detects malformed/corrupted decrypted payloads; it is not a
replacement for DPAPI or a custom encryption scheme.

Protected **anchor** provisioning/ordering, rollback-resistant storage, Windows
DACL/directory-handle enforcement, installed-tree postcondition checks, persistent
staging, installation/rollback execution and production publisher-key approval are
still absent. DPAPI sealing a local MAC key does not solve valid-state replay.
`protectedAnchorPersisted` and `installationAuthorized` remain false. This factory
is intentionally internal, with no update UI or automatic installation enabled.

Windows key protection references:
[CryptProtectData](https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata),
[CryptUnprotectData and integrity limits](https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata).
