# Law Enforcement Workspace — implementation baseline

Status: **foundation only**, not a released or operational law-enforcement module.

## Product scope
Law Enforcement is a first-class workspace inside NEXVARY-MR, alongside Mobile Repair. It is not a separate product. The existing Digital Forensic Laboratory is preserved, with feature expansion temporarily deprioritized.

## Case intake
A case must record an internal ID, official case number, agency, investigator, seized device description, authority reference, issuing authority, and explicit scope. Prosecutor name, court name and UTC authorization-date endpoints are optional in the foundation model; no prosecutor, court or conviction requirement is introduced. Their nonempty values receive structural validation. Attachment metadata, seizure location/time, device identifiers, custody transfers and legal jurisdiction require dedicated UI/storage and validation.

The new header `cpp/include/nexvary/LawEnforcementCase.hpp` provides a dependency-free C++ case data model, bounded UTF-8 validation, status checks and optional UTC-date validation. **Completeness is not legal validity or permission to access a device.** The model does not change device state, access credentials, or decrypt evidence.

## Next engineering slices
1. Persist case records using an encrypted, migration-tested database with role-based access; never store case data in application logs.
2. Add Case Intake Qt/QML workspace with Arabic RTL, English translations, consistent cards, actual licensed colored icon assets, and responsive layouts.
3. Add append-only custody events, evidence SHA-256 manifests, case exports and audit trail.
4. Implement device-to-case binding, authorization scope review, and preflight of supported read-only collection paths.
5. Add research/training simulation on synthetic devices; distinguish verified device capabilities from hypotheses.
6. Validate Windows build, tests, screenshots from the running application, and legal/privacy review before any production use.

## Non-goals for this increment
No PIN removal, FRP bypass, cryptographic bypass, device exploitation, live evidence extraction, or claims of forensic certification.

## Validation contract

Limits are UTF-8 **bytes**, not displayed characters: case ID and case number
128 each; agency, investigator, authority reference, issuing authority,
prosecutor and court 256 each; device description 2048; scope 8192.
Malformed/overlong UTF-8, surrogate scalars, embedded NUL, prohibited controls,
bidirectional overrides/isolates and a BOM are rejected. Scope and device
description allow TAB/LF/CR; single-line identity fields do not. Required fields
must contain visible text; Unicode whitespace or formatting marks alone do not
satisfy completeness. Prosecutor and court may remain empty.

Optional UTC dates accept `YYYY-MM-DDTHH:MM:SSZ` or the same format with 1–9
fractional digits before `Z` (maximum 30 bytes). Gregorian dates and clock fields
must be valid; supplied endpoints must be in chronological order, including
fractional seconds. Either endpoint or both may be absent. No local timezone is
inferred and no comparison with the current clock grants permission or establishes
legal validity/expiry. CaseStatus values 0–3 remain stable and Archived is 4;
unknown CaseStatus/AccessOutcome enum values are rejected by their helpers.

## Test

`cpp/tests/law_enforcement_case_tests.cpp` now runs as
`mr_law_enforcement_case_tests` under `BUILD_TESTING`, including core-only builds
with `MR_BUILD_GUI=OFF`. Checks throw and return a failing exit status; they are
not C `assert` statements and remain active under `NDEBUG`.

Verified 2026-10-24 with GCC 13.3.0, C++20 and warnings as errors:

- Standalone Debug: **432 runtime checks passed**.
- Standalone `-DNDEBUG -O2`: **432 runtime checks passed**.
- Core-only CMake/CTest Debug: **3/3 passed**.
- Core-only CMake/CTest Release: **3/3 passed**.

Raw core CTest logs are retained in `evidence/law-foundation-2026-10-24/`.
**Windows CI for the resulting commit remains pending**; Linux checks are not
Windows/device acceptance. This foundation has no DB/UI integration or device
operation path. Successful validation means structural readiness for review,
never legal approval, current authorization or permission to access a device.
