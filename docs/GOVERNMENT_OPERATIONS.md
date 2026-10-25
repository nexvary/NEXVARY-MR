# Government operations increment — 2026-10-25

## Product direction and branch review

Government/Law Enforcement is the first workspace, Research second, and Professional
Repair remains a shared-engine workspace in the same executable. The navigation
has twelve government sections and a separate repair navigation switch. Legacy
page APIs and tests remain available. This is a development build for evaluation.

Base: `feature/law-enforcement-case-foundation` at
`8fb9b38f771da91c60f3b85a2084868b16fe95d4`, PR #11. Its executable predecessor
`03da82d981e4e192b20dcf12af4aeddac16d71e0` has seven successful native CI jobs in
40098038007, independently rechecked through the GitHub jobs endpoint this session.
The baseline already contains the Research Program, case management, protected
case/evidence storage, local identities, roles/grants, signed research reviews,
audit anchors, PDF reporting and synthetic training. Do not rebuild those systems.

`evidence/government-2026-10-25/remote-audit.json` pins all requested branch/PR heads.
Research PR #12 is already ancestral to the case branch. USB/Device Intelligence
and workstation changes are also present there. No main merge or blanket merge
of the NPSE branch is performed. The earlier local uncommitted workstation files
were left intact; this increment starts from the actual published case branch.

## Working-copy export and independent verification

Evidence & custody now exports a selected evidence item to a **new** local package
directory after explicit unencrypted-export consent and an export purpose. Native
`ExportEvidence` policy permits investigator/reviewer; auditors can inspect audit
without exporting file bytes. Case ownership/grants and session checks still apply.

The protected original is read, decrypted by the existing provider and checked
against its recorded size/SHA-256 before output. Evidence from a different case,
stale revisions, corruption, existing output directories and missing purpose are
rejected. The package contains exactly `evidence.bin` and `manifest.json`. Source
filenames are metadata and never output paths. The exporter requests owner-only file/directory permission bits. On Windows, Qt
permission bits are not an independent NTFS ACL policy: exports inherit the chosen
destination security context. Use an institution-approved, access-controlled volume;
these explicitly unencrypted exports are not protected by the original DPAPI vault. The bounded 16 MiB evidence and 100-export-per-case limits apply.

The manifest pins case/device/evidence UUIDs, source revision and audit head,
export actor/time/purpose, synthetic profile, size and file hash. Its exact-byte
SHA-256 is stored in the case's `workingCopies` history, covered by the linked audit
and displayed for separate retention. Closed/archived case content remains immutable;
working-copy exports and analysis can append audit/history records. Suspended cases block export.
The package references the pre-export revision; the next audit event records its hash.

```sh
python scripts/verify_evidence_package.py /path/to/new-package --manifest-sha256 <separately-retained-64-hex-digest>
```

This Python standard-library tool works offline without Qt, Windows, account keys
or NEXVARY credentials. It rejects altered manifests/files, duplicate JSON fields,
unsupported schemas, extra files, traversal paths, oversized/nonregular files and
leaf symlinks. It neither parses nor executes evidence. A digest copied only from
the same untrusted package is not an independent trust reference. Successful checks
establish byte integrity relative to the retained digest, not legal entitlement,
original-device authenticity, physical custody or trusted external time. Existing
signed audit anchors can bind the subsequently retained case history; the manifest
itself is not an institutional digital signature.

File creation and SQLite commit do not form a crash-atomic filesystem transaction.
Normal failures remove files created by this operation and roll back database changes.
A process/power failure may leave an incomplete/orphan package. Reject incomplete
packages, compare the retained digest with the case export history, and choose a
fresh directory before retrying. Never delete or overwrite a protected source to retry.
OS-account compromise, mounted network filesystem provenance and parent-directory
replacement by a privileged actor remain outside the local-account storage boundary.

## Device assessment

The government assessment projects manufacturer/model, SoC/hardware, firmware,
Android/iOS, patch, bootloader, Verified Boot, encryption and TEE/Secure Enclave.
Fresh existing Android observations retain their source/time and are labelled
**device-reported**, never physically tested or attested. Unavailable/expired fields
remain unknown. No iOS version, enclave state or supported acquisition path is
inferred from a brand/model. Exact-device acquisition risk stays review-required;
the assessment does not execute commands or grant a capability.

## Project boundaries and remaining work

Guardian is a separate child/family-safety product. MR neither launches nor depends
on its package, and this increment changes no Guardian repository or Android agent.
The NPSE branch (PR #14) contains reusable engine code and a distinct MR Android
companion. Shared library provenance does not make Guardian an MR agent. Its Android
changes remain outside this case-based branch until an explicit, tested integration
is needed. Do not require installing Guardian to operate MR.

Government analysis now uses the existing classic-PCAP and JPEG-EXIF parsers directly
on hash-verified case evidence in memory, behind a native AnalyzeEvidence permission.
Investigator/reviewer sessions need a current case grant. Reports retain source hash,
parser/version, exact report JSON/digest, actor and local time in protected case state
and append audit. Repeating analysis of the same source produces the same report
bytes/digest. The original remains unchanged. Malformed/unrecognized input, sources
over 8 MiB, unrelated evidence and unauthorized roles fail closed. No new parsing
features were added to the frozen legacy forensic lab. The government navigation
uses this case-bound page rather than opening old repair/recovery case collections.
PDF case reports include analysis and working-copy history.

Generic Android backup parsing, SQLite case analysis, parser-to-case indexing/timeline
integration, document-specific attachments,
actual disposal execution, searchable case artifact integration and a full document-management
subsystem remain implementation gaps. Protected backup/restore and retention governance
are implemented; see PROTECTED_CASE_BACKUP.md and EVIDENCE_RETENTION.md. Existing evidence import can preserve an
explicitly selected document but is not a document-management subsystem.

Institution SSO, independently verified personnel, external key/time custody,
trusted-publisher signing, actual owned-phone/exact-firmware acceptance and clean
Windows 10/11 validation require their actual environments/materials. Existing
hosted Windows Server packaging is not a Windows 10/11 field acceptance test.
