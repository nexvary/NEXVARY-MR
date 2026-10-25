# Public build-digest findings — 2026-10-25

Government source 0b9bc08d1ac70a81f94c4305eefffdefd63ca793 passed Python checks,
but public-history run 40145065000 failed at Gitleaks. The operational-data guard
reported zero findings. Reproducing with the pinned, checksum-verified Gitleaks
8.30.1 and fetched public PR head/merge refs produced exactly seven generic-api-key
findings, all introduced by the existing base commit
8fb9b38f771da91c60f3b85a2084868b16fe95d4. No new government code finding was reported.

Path: evidence/identity-2026-10-24/windows-sodium-build-receipt.json, lines 15–18,
45–46 and 55. These entries map public include/sodium/crypto_auth*,
crypto_onetimeauth* and crypto_secretbox.h header paths to their SHA-256 digests.
The producer, scripts/prepare_sodium.py, computes them with hashlib.file_digest.
All seven values independently matched the prepared public 1.0.22 header bytes
with Windows CRLF line endings. This explains their difference from Linux LF hashes;
the values are public file digests rather than API credentials.

.gitleaksignore retains seven exact immutable historical detector fingerprints.
The current-tree check also encounters the same seven values. .gitleaks.toml extends
all default rules and adds a generic-api-key allowlist with AND semantics: the exact
reviewed receipt path AND one of the seven exact reviewed public digests. It does
not exempt an entire file/directory, other rules, unknown values or future content.

scripts/check_history_digest_allowlist.py invokes the real pinned scanner. The
reviewed receipt must pass, each of seven changed values at the same path/key must
fail, and the reviewed values copied to an unreviewed path must fail. The history
workflow runs this check before its unchanged full-history and directory scans.
Redaction remains enabled; no history is rewritten or detector disabled.

This review is consistent with the existing NPSE branch's historical-fingerprint
review, but independently verified here. No NPSE Android or Guardian source is merged.

Primary syntax reference (pinned upstream README):
https://github.com/gitleaks/gitleaks/blob/v8.30.1/README.md#configuration
