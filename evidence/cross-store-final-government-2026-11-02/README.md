# Exact-source Windows delivery verification — 2026-11-02

Runtime/package source: `5e0ac5b9c07daca4a46949f145c3eed09b21daa2`. Push [41426917935](https://github.com/nexvary/NEXVARY-MR/actions/runs/41426917935) and PR [41426928607](https://github.com/nexvary/NEXVARY-MR/actions/runs/41426928607) separately completed successfully. The commit containing this evidence is a subsequent **unshipped evidence-only checkpoint**, not the source of these binaries. PR merge trees matched the source tree before this checkpoint.

Downloaded all three provider artifacts and compared provider SHA-256; verified every inner file manifest and SPDX file, AMD64 main/worker bytes, source/profile, shipped guides and verifier, exclusion of the manual benchmark, and deployment receipts with identical actual executable hashes. Independently reverified both downloaded synthetic 23-event signed audits using the pinned source verifier. Installer install/launch/uninstall and per-user registration cleanup receipts passed. Full hashes, native totals and intentional skips: [verification.json](verification.json). Provider IDs/digests: [provider-artifacts.json](provider-artifacts.json).

Windows CTest: 50 groups, zero failed/error groups; signed C++/Python interoperability: 77. Subtest skips remain visible (optional decoders, separately configured TLS/mmls, POSIX symlink and Laboratory-only write fixtures). This is not zero-skip coverage.

Reproduce verification by downloading the three artifacts in provider-artifacts.json, then running:

```sh
python3 verify_delivery.py --repo /path/to/exact-source-checkout --sha 5e0ac5b9c07daca4a46949f145c3eed09b21daa2 --portable portable.zip --installer installer.zip --evidence evidence.zip --metadata provider-artifacts.json --output verification.json
```

Use the exact source checkout and its Python dependencies (cryptography), not an arbitrary later verifier. No PE is executed by this downloaded-artifact checker. No raw archives, keys or private fixtures are in this checkpoint. The local environment reset made earlier transient original synthetic fixtures/binaries unavailable; their published sanitized observations are historical evidence, not retained original bytes.

Limits: unsigned hosted evaluation only, no physical Windows10/11/device, process-tree/network isolation, trusted external anchor, power-loss, institutional/legal/security/license/retention or competitor acceptance. Both-store consistent rollback requires a trusted external anchor. Original Audit Store 135/139 prefix cause and local125%DPI timeout/root remain open. Existing tests and CI are not proof of those root causes being resolved. NTFS remains an internal bounded engine, not product-enabled. No new visual change/acceptance.

Next executable task: reproduce retained detached-worker negatives on current government source, then implement pre-descendant Windows Job assignment and Linux owned process supervision/groups with bounded cancellation receipts, testing Cancel/Logout/Expiry/Revocation/ParentCrash/Detachment/Timeout before transferring to integration.
