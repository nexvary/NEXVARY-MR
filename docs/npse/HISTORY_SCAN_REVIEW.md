# Historical scan review — 2026-10-24

Initial public-history CI run 40109291525 reported seven Gitleaks generic-api-key
findings. All seven are in the files mapping of the Windows libsodium build
receipt added by commit 8fb9b38f771da91c60f3b85a2084868b16fe95d4, path
`evidence/identity-2026-10-24/windows-sodium-build-receipt.json`, lines
15–18, 45–46 and 55. Keys are public include/sodium/crypto_*.h header paths;
values are 64-character hexadecimal SHA-256 file digests.

The receipt producer in that same commit, scripts/prepare_sodium.py, computes
file SHA-256 values with hashlib.file_digest and writes them under files.
The generic detector matched words such as auth/secret in public header names.
No API credential is present in these reviewed receipt entries.

.gitleaksignore records the seven exact detector fingerprints (commit, path,
rule and line). It does not exclude the directory, rule, file or future commits.
No history rewrite or detector disablement is used. Behavior was checked against
the official v8.30.1 README and the checksum-verified binary. New findings must
be independently reviewed; do not broaden these exceptions.

Repeat from the repo root:

```sh
gitleaks git --log-opts='--all --full-history' --ignore-gitleaks-allow --redact=100 --no-banner .
gitleaks dir --ignore-gitleaks-allow --redact=100 --no-banner .
python3 -m unittest discover -s tests -p test_public_history.py -v
```

Actual repeated results are recorded in TEST_RESULTS.md. Local scanning includes
fetched branch history; hosted CI additionally fetches public PR head/merge refs.
