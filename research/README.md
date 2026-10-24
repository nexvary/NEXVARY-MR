# NEXVARY Advanced Mobile Technology Research Program

**Status:** Research foundation; no claim of verified device support.

This is the official research and development program of NEXVARY-MR. Research must translate into measured, independently reproducible improvements to the repair, device intelligence, firmware, USB diagnostics, and authorized evidence workflows.

## Three parallel workstreams

1. **Commercial product now:** reliability, diagnostics, supported OEM workflows, compatibility data, user experience, Windows QA.
2. **Research laboratory:** Mobile Security Architecture Atlas, repeatable experiments on owned/authorized devices, security architecture, firmware and USB observations.
3. **Institutional capabilities:** law-enforcement case intake, chain of custody, audit, evidence integrity, training, documentation, privacy and authorization gates.

## Rules of evidence
Every assertion must be tagged `VERIFIED`, `REPORTED`, `HYPOTHESIS`, or `REFUTED`.
- VERIFIED: reproduced in a documented authorized test with model/build/patch, environment, raw evidence and reviewer.
- REPORTED: attributed to an external publication, not reproduced in-house.
- HYPOTHESIS: research proposition without supporting repeatable evidence.
- REFUTED: tested and disproved under recorded conditions.

No entry may use `VERIFIED` based solely on marketing, a leaked file, a blog, or an unrepeatable demonstration. Never generalize one model/patch result to an entire brand.

## Research artifacts
- `atlas/`: architecture and trust-boundary notes
- `compatibility/`: machine-readable coverage and evidence status
- `experiments/`: protocols, results, controlled synthetic datasets
- `sources/`: bibliographic metadata and legal provenance
- `docs/`: engineering gates and integration plan

## Source policy
Use lawfully accessible vendor documentation, public research, academic literature and permitted open-source projects. Do not store leaked proprietary source, malware binaries, stolen credentials, or exploit kits in this repository.

## Product integration
Expose reviewed knowledge as **read-only, evidence-linked** compatibility and security architecture insights in Device Intelligence. Research records must not trigger device-write operations. No PIN/FRP bypass or cryptographic circumvention is implemented by this program.

## Executable integration

`research/program/catalog.json` is the source-backed Qt Research Center catalog. Native `ResearchBridge` owns bounded result/compatibility intake and validation; the older package files remain attributed import references, not an alternate product capability database. `research/advanced-mobile/knowledge.json` remains the shared native capability-provenance catalog. See `docs/UNIFIED_WORKSPACES.md` for persistence, tests and outstanding acceptance gates.
