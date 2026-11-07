# NEXVARY OSINT — 20-stage implementation plan

Status labels: IMPLEMENTED (code committed, CI pending); PLANNED (not implemented). A design document is not proof of working software.

| Stage | Deliverable | Status | Acceptance gate |
| --- | --- | --- | --- |
| 01 | Provider registry, disabled-by-default | IMPLEMENTED | registry unit tests |
| 02 | Case authorization and cost approval | IMPLEMENTED | fail-closed contract tests |
| 03 | Local mock providers | IMPLEMENTED | offline routing tests |
| 04 | Evidence provenance model | IMPLEMENTED | SHA-256 format and source tests |
| 05 | Scoped Subfinder text parser | IMPLEMENTED | malformed and out-of-scope fixture tests |
| 06 | Subfinder executable adapter | PLANNED | pinned binary digest, no shell, cancellation, timeout |
| 07 | ExifTool JSON parser | PLANNED | bounded local JSON fixtures |
| 08 | ExifTool process adapter | PLANNED | offline media fixture, no outbound network |
| 09 | Username source schema | PLANNED | canonical URL, ambiguity flags |
| 10 | Sherlock process adapter | PLANNED | fixture-based results, rate-limit safeguards |
| 11 | Maigret process adapter | PLANNED | report parsing and attribution |
| 12 | SpiderFoot optional service | PLANNED | local API discovery and isolation |
| 13 | PhoneInfoga normalization bridge | PLANNED | existing adapter compatibility tests |
| 14 | Unified async job queue | PLANNED | cancel, deadline, concurrency limits |
| 15 | Case-scoped encrypted evidence store | PLANNED | integrity, access and retention tests |
| 16 | Entity resolution and graph | PLANNED | no unverified identity merge |
| 17 | Qt OSINT workspace (AR/EN) | PLANNED | keyboard/RTL/accessibility tests |
| 18 | OSINT Industries licensed adapter | PLANNED | official API, OEM permission, mock-first |
| 19 | Lampyre licensed adapter | PLANNED | documented API/rights, mock-first |
| 20 | Signed Windows evaluation build | PLANNED | CI green, security and license review |

Integration rules: no credential collection or active intrusion; no paid API requests in CI; explicit operator authorization before external disclosure. Do not package third-party code without license review. Every finding remains an unverified lead until independently corroborated.

Next blocking work: stage 06 requires process trust/pinning; stages 07–08 require JSON fixtures and Qt process integration; stages 18–19 require vendor authorization. GitHub Actions success must be verified on exact commit before declaring any stage tested.
