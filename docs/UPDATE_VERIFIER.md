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

## Limits before installation can be enabled

A successful result means these observed bytes match an approved signature and
hash under the caller's trust assumptions. It does not certify publisher identity,
ZIP/PE validity, Authenticode, OEM firmware compatibility or installation safety.
The mutable source pathname is not an installable capability. Same-size concurrent
changes or later path replacement are not protected by a snapshot. There is no
immutable staging copy, protected/persistent sequence floor, clock rollback defense,
key rotation/revocation workflow, atomic install journal or interruption recovery.
Every success explicitly reports `installationAuthorized=false` and
`rollbackStatePersisted=false`; package results also report `installed=false`.
A future installer must revalidate an owned staging snapshot and enforce these
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

A core-only build remains dependency-free by default. For verifier-only testing,
use `MR_BUILD_GUI=OFF` and `MR_BUILD_UPDATE_VERIFIER=ON`. Build dependency receipts,
upstream checks and native results are retained by CI. No physical phone, Windows
10/11 client, production signing key or end-to-end update acceptance is implied.
