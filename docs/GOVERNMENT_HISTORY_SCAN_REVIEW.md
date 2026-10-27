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

## Policy input-manifest digest review — 2026-10-26

Public-history runs 40364749537 and 40365166282 failed on twelve generic-api-key
matches introduced in evidence checkpoint 29ead97442f35c9f18ebc454b02d32cb235784b5.
The operational-history guard reported zero findings. Safe scanner metadata
identified four source-file SHA-256 values at lines 126, 127, 176 and 213 in each of:

- evidence/policy-trust-2026-10-26/source-inputs.json
- evidence/policy-trust-2026-10-26/verified-source-inputs-6597367.json
- evidence/policy-trust-2026-10-26/verified-source-inputs-8e7e174.json

The entries correspond to cpp/src/UpdateKeyStore.cpp, cpp/src/UpdateKeyStore.hpp,
cpp/tests/update_key_tests.cpp and qml/DesignTokens.qml. Their names contain key
or token vocabulary; the values are public source digests. All four were
independently recomputed from fetched UTF-8 Git source (SHA-256 implementation
checked against the empty-string and abc known vectors) and matched all three
manifests. The CI check now independently recomputes them with Python hashlib
from each manifest's exact pinned Git commit before invoking the actual pinned
Gitleaks runtime.

The new generic-api-key exception requires AND semantics: one of these three
exact manifest paths AND one of the four exact recomputed digests. No directory,
other rule, changed value or unreviewed path is exempted; historical commits and
existing seven header checks remain intact. The scanner test requires twelve
changed-value mutations and three unreviewed-path copies to be detected, in
addition to the existing seven-value/one-path negative corpus. It runs before
full-history and current-tree scanning; any failure still fails the workflow.

Scanner logs expose only report name, rule, file, line and commit. Match, Secret,
Line, author and messages are omitted. Both scans run even if the first rejects
input; nonzero scanner status still fails the gate. No history rewrite or raw
signed operational-history publication is part of this correction.
