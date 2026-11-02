# Next bounded Downloads increment — source review only

Browser source pin d7754cd8f051d285406403465b6d7daa3bc4b060;
reconstructed nbf/parsers.py SHA256 5edd1440ac7d5e3f29e892f9c419736267211c64ad82eeecd769ca251b372ad7. This review adds no enabled capability.
The same pinned ChromiumParser.downloads was inspected while History CI runs.

Original behavior selects available download columns, optional downloads_url_chains
ordered by chain_index, picks chain[0] or tab/site/referrer as URL, chooses target
or current path, converts start/end/last-access times and returns DOWNLOAD metadata.
Its broad exception handling can hide chain failures, and queries have no global
row/time/output bound. MR must not copy those behaviors unqualified.

Next implementation criteria:

1. Reuse current protected family/table bytes, existing case services, source-member
   extents, authority/member revalidation, cancellation and signed acceptance.
2. Inspect actual CREATE layouts for downloads and downloads_url_chains; preserve
   original IDs, raw chain_index, raw timestamps and numeric state/danger/interrupt
   codes. No assumption that every table uses INTEGER PRIMARY KEY id or that a
   composite index is a rowid table. Unsupported layouts must fail explicitly.
3. Treat recorded target/current paths as strings only: never resolve/open/write
   them. No assertion that a download exists, finished, is intact or is safe based
   on state/counters alone. Received/total bytes are source-reported metadata.
4. Bound table/chain/record/output sizes and total execution. Reject malformed chain
   references/ambiguous ordering and report unavailable URL fields rather than
   silently substituting a different field or swallowing a database error.
5. Preserve each URL chain member and the method used for any displayed URL.
   Start/end/last-access raw microseconds and normalized display precision remain
   separate; no derived causal/actor identity claim.
6. Use actual SQLite known-answer WAL/malformed/orphan/type/large-chain fixtures,
   independent joins, exact main/WAL byte slices, case isolation/revocation/cancel/
   restart/receipt tests and actual Arabic/English scaled controls before enablement.

History acceptance does not validate Downloads or close the broad Browser stage.
Sessions/profile metadata, all-filesystem recovery and original audit/descendant/
Qt-root blockers remain separate work.
