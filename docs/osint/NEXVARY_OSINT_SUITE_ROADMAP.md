# NEXVARY OSINT Intelligence Suite — integration roadmap

Status: researched design; **not** a claim of completed integrations. Target: NEXVARY-MR Government Edition, Qt/C++20, Windows-first, Arabic/English.

## Product boundary
An optional **NEXVARY OSINT** workspace within an authorized case. Keep provider adapters isolated from case management and from device forensic operations. No default bulk collection, account probing, or automated escalation to active scanning.

## First-wave candidate integrations
| Tool | Scope | License (verify at pinned revision) | Integration mode |
| --- | --- | --- | --- |
| Sherlock | Public username profile discovery | MIT | Isolated CLI process, JSON parsing |
| Maigret | Username dossier and evidence references | MIT | Isolated Python process, structured output |
| Subfinder | Passive domain/subdomain discovery | MIT | Pinned Go executable, JSONL |
| SpiderFoot | Multi-source enrichment and correlations | MIT | Separate local service, export/import; do not assume OSS REST API parity with HX |
| ExifTool | Local media/document metadata | Artistic/GPL dual-license terms: review before bundling | Local executable, JSON |
| PhoneInfoga | Phone OSINT | GPL-3.0 | Keep existing isolated process; review distribution obligations |
| theHarvester | Domain/email reconnaissance | GPL-2.0-only (check revision) | External process, no source-code copying |
| ArchiveBox | Capturing web references | GPL-3.0 (check revision) | Optional separately installed service |
| OSINT Industries | Paid enrichment | Commercial terms | Disabled until API contract and OEM rights verified |
| Lampyre | Paid enrichment | Commercial terms | Disabled until API/embedding rights verified |

## Product features to build
1. Case-scoped workspace: authorization ID, investigator, permitted selectors, disclosure consent, retention policy.
2. Unified search: domain, username, email, phone, URL, IP, file. Provider capability matrix prevents invalid dispatch.
3. Job manager: explicit start/cancel, timeouts, source rate limits, process isolation, no shell interpolation.
4. Normalized result model: entity, relation, raw source URL, retrieval timestamp, adapter/version, SHA-256, confidence state, evidence ID.
5. Link-analysis graph: show relationships as **leads**, never assume matching usernames identify the same person.
6. Evidence vault: immutable acquisition metadata, cryptographic hashes, case timeline, export with provenance.
7. Provider center: opt-in installation, health/version/license status, API-key storage in OS credential vault, cost preview and per-provider consent.
8. AR/EN RTL/LTR UI with tooltips, provenance view, clear blocked/rate-limited/unknown outcomes.

## Sequenced delivery gates
- G0: pin tool revisions, verify licenses, dependencies, network behavior and terms; prepare SBOM and attribution.
- G1: offline provider registry + fake results fixture + schema normalization unit tests.
- G2: isolated ExifTool and Subfinder adapters using local fixtures; integration tests on Windows/Linux.
- G3: Sherlock/Maigret adapters, explicit external-query disclosure, robust false-positive handling.
- G4: Qt tab, graph, evidence export, cancellation, audit logging.
- G5: signed Windows evaluation build, test reports, license notices, red-team privacy review.
- G6: commercial adapters only after contract/API rights and credentials; never run live queries in CI.

## Safety and legal controls
Respect service terms, robots/rate limits and jurisdictional privacy law; prohibit credential attacks, bypasses and hidden collection. Tests use synthetic identifiers only. GPL/AGPL licenses may impose obligations even for process integration depending on distribution and coupling; obtain license review before packaging. Do not embed paid provider endpoints without current official documentation.

## References (checked 2026-11-07)
- https://github.com/sherlock-project/sherlock
- https://github.com/soxoj/maigret
- https://github.com/projectdiscovery/subfinder
- https://github.com/smicallef/spiderfoot
- https://github.com/sundowndev/phoneinfoga
- https://github.com/laramies/theHarvester
- https://github.com/ArchiveBox/ArchiveBox
- https://exiftool.org/
