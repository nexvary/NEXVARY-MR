# Three-source forensic engine integration — audit gate

Tracking: #16. Base: `feature/government-operations`. **Status: architecture and source audit pending; no executable integration claimed.**

## Source candidates

| Repository | Documented implementation | Proposed MR module | Decision |
|---|---|---|---|
| nexvary/NEXVARY-Digital-Forensics-Workbench | Windows evidence handling, analysis and signed installer chain (README) | Evidence adapters, verified reports | AUDIT: compare against existing MR case/evidence/import/timeline/report; internal signing does not establish institutional trust |
| nexvary/NEXVARY-Browser-Forensics-Lab | Windows Python/Qt forensic workstation with packaged smoke tests (README) | Browser artifact read-only parser | AUDIT: inspect actual parser sources and formats; isolate Python process; hash copied evidence |
| nexvary/NEXVARY-DC | C++20/Qt6 disk inventory, imaging, filesystem recovery, destructive disk administration (README) | Read-only image/NTFS/exFAT/FAT32 analysis | AUDIT: strict allowlist of read-only methods; explicitly exclude formatting, partition mutation, repair, erase and USB installer functions |

## Required source audit (must complete before copying code)

1. Record exact upstream commit SHA, license of each source and third-party component, build manifest and test commands.
2. Locate real source modules and classify each as implemented/tested/experimental. A README is not code verification.
3. Map existing MR equivalent capabilities and choose reuse, adapter, or leave separate; no duplicate evidence stores.
4. Specify trust boundary and authorized case-scoped workflow, bounded file handling, immutable original hash, provenance, cancellation, process containment and no automatic network calls.
5. Build a minimal integration fixture with synthetic files; reject malformed data and out-of-scope writes.
6. Keep all upstream repositories operational; no upstream branch deletion and no automatic main merge.

## Proposed isolated interfaces

- `ForensicSourceDescriptor`: source repository, pinned SHA, license, module, supported format, input limits.
- `ReadOnlyEvidenceRequest`: case ID, evidence ID, verified working-copy path, expected SHA-256, explicit authorization.
- `ForensicFinding`: artifact type, offset/path, parser version, source digest, confidence, raw/derived distinction.
- `ForensicAdapterResult`: status, failure class, bounded findings, provenance, signed operation receipt reference.

These are interface design proposals, **not implemented C++ classes**.

## Order and acceptance

A. Workbench evidence capability diff and approved read-only adapter.
B. Browser forensic parser adapter.
C. Disk Care read-only image and filesystem adapter.
D. Case-to-finding-to-timeline-to-report end-to-end checks.

Each PR requires Windows/Linux CTest, sanitizer, Qt/QML RTL/DPI, security regression, malformed fixture, input limits, cancellation and SHA-256/provenance verification. Record source SHA, exact test logs, artifact hashes and remaining gaps. Coordinate with ongoing Government reliability work and PR #15; never claim accreditation or superiority from CI alone.
