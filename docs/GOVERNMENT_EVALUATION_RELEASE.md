# Government Technical Evaluation Release scope freeze

Decision date: 2026-11-02. Publisher: Nexvary Inc. Candidate only; NOT RELEASE READY.
This document supersedes open-ended feature expansion for this evaluation release.
PR15/17 remain draft and unmerged. It does not grant accreditation, partnership,
legal authority or permission to use real evidence. Guardian is excluded.

## Frozen evaluation path and feature matrix

Verified means first-party synthetic/native/hosted evidence at the **pinned source**
recorded in DEVELOPMENT_STATE.md, never automatic acceptance of a newer head.
All current-candidate Windows package checks remain separate release gates.

| Function | Classification | Boundary and required evaluation evidence |
|---|---|---|
| Local identity, roles, case ownership/grants and case creation | Implemented and verified on earlier pinned sources | No institutional SSO or proof of independent people; identity/native/workspace suites |
| Authorized local evidence registration and protected retained copy | Implemented and verified on earlier pinned sources | Windows current-user DPAPI; SHA256 proves byte consistency, not authenticity; 16 MiB cap |
| Bounded text/XML/CSV/JSON/SQLite/log/ADB-v5 inventory imports | Implemented and verified on earlier pinned sources | See AUTHORIZED_IMPORT.md/actual adapter limits; no full application/device extraction |
| Classic PCAP and JPEG EXIF case analysis | Implemented and verified on earlier pinned sources | 8 MiB; exact source/report provenance and prepared intents; no PCAPNG or encrypted traffic decryption claim |
| Registered raw-image TSK mmls partitions | Implemented and verified on earlier pinned sources | Optional user-installed external dependency and reviewed exception; 16 MiB; no automatic network isolation |
| Findings, literal AR/EN search and provenance graph | Implemented and verified on earlier pinned sources | Bounded retained-report projection; no semantic/person/social inference |
| Timestamp timeline | Implemented and verified on earlier pinned sources | Explicit offsets and unresolved timestamps; no timezone inference or complete activity semantics |
| PDF, audit export, independent verifier and custody review | Implemented and verified on earlier pinned sources | Exported material may be unencrypted; external anchor/key/time custody required |
| Restart reconciliation and cancellation acceptance | Implemented, current candidate not fully verified | Both journal-to-result directions, unknown remains unknown; no automatic external-effect replay |
| Owned worker cancellation/owner death containment | Implemented and verified at a23c9d8 and integration5a4efc9 | Windows Jobs/Linux supervisor; preserve missing-helper and hostile same-user negatives; not a sandbox |
| Current import-cancel correction and permission-transition fixture | Implemented, fresh candidate verification required | Neither success nor rejection retained after explicit cancel; no stale-live-owner lock deletion |
| Full Windows10/11 delivery/UI/scale and independent laboratory acceptance | Implemented delivery paths, field verification open | CI Server is distinct from physical desktop; unsigned development packages |
| Professional repair, device writes, cloud, browser/network tools | Not included in government evaluation | Existing native default-deny policy/roles remain; workspace navigation grants no authority |
| NTFS internal engine | Future / not product scope | No binding/UI enablement in this release; preserve integration tests |
| Locked phone extraction, bypass, iOS acquisition, enterprise identity | Future / not product scope | No claim, demo dependency or acceptance condition for current release |

## Finite release blockers

| ID | Severity | Closure condition | Required evidence | Planning estimate and confidence |
|---|---|---|---|---|
| R1 | Critical | Original135/139 audit lag root identified or risk demonstrably bounded; no acknowledged loss/invalid accepted state | Original artifacts if recoverable; diagnostic/fault/concurrent/permission traces; independent review of exact write/readback path | Cannot estimate root investigation credibly from unavailable original bytes; review after2 focused engineering days; low confidence |
| R2 | High | Correct permission-transition test, preserve fail-closed lock policy; cancel fix accepted on government then integration | Separate Push and PR native/sanitizer results on each SHA, retaining previous failures | 0.5–1 engineering day plus hosted queue; medium |
| R3 | High | Frozen end-to-end evaluator procedure repeats on delivered source | Clean case/import/hash/search/timeline/PDF/audit/restart traces and negative outcomes; AR/EN UI at100/125/150% | 1–2 engineering days after R2; medium; historical DPI root stays explicit until evidence bounds it |
| R4 | High | Exact Installer/Portable source, manifests, hashes, install/run/uninstall verified | Provider digest, internal manifest/SPDX checks and installed byte equality; Windows10 and11 field receipts | 1 engineering day after CI plus external host access; medium/low |
| R5 | High | Submission dossier matches actual delivered bytes and limitations | Editable/PDF render QA, reference sample truth/hash validation, actual screenshots; no external sending | 0.5–1 engineering day after candidate pin; medium |
| R6 | High | Evaluation operating safeguards accepted for limited synthetic pilot | Authorized environment, local identity/key/anchor/time/backup/export custody and license/security review | External scheduling unknown; no guaranteed date |

These estimates describe remaining effort, not a promised calendar release date.
Work overlaps only where independent. R1 has no defensible upper bound today.
No date is committed until R1 and source-specific R2 gates close. A candidate may
be reviewed internally while blocked; it must not be presented as release-ready.

## Acceptance protocol

1. Verify supplied SHA256 and candidate source/PR tree before installation.
2. Start with approved synthetic data only. Establish separate local requester and
reviewer accounts; verify unauthorized role, self-review and expired-grant refusals.
3. Create case EVAL-2026-001, register evaluation JSON with explicit authority and
verify its retained source hash. Run JSON import; verify source/report paths.
4. Review findings, literal Arabic/English search and explicit-offset timeline;
compare against independent ground truth, documenting unsupported fields.
5. Export case PDF and independently verify audit export against separately kept
anchor. Do not place private keys or protected state in the submission/public Git.
6. Cancel an active import, logout/revoke authority, and confirm no late retained
result. Switch case and restart; compare complete retained state and signed history.
7. Repeat real UI AR/EN100/125/150%, resize, report clearing, case switches and worker
transitions. Record actual captures, source and environment; keep timeouts/assertions.
8. Interrupted write recovery must not rerun an external effect or convert unknown
into success. Preserve original divergent files, refuse silently fixing them.

## Deferred work and comparison discipline

NTFS product linkage, new engines, phone bypass, large-case indexing, semantic graph,
enterprise identity and network sandboxing move to a later release. They do not
expand the current evaluation. No competitor executable is available in this
workspace: ground-truth results are first-party tests, never comparative parity.
Official vendor pages dated by access date are claims, not direct measurements.
An authorized same-sample protocol must score correctness/misses/false positives,
source provenance, elapsed time/RSS, search/timeline/reporting and AR/local use.
Unknown competitor behavior is unknown, never absent. No purchase or sensitive
collection is authorized implicitly.
