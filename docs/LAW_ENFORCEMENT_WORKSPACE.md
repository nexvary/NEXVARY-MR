# Law Enforcement Workspace — implementation baseline

Status: **foundation only**, not a released or operational law-enforcement module.

## Product scope
Law Enforcement is a first-class workspace inside NEXVARY-MR, alongside Mobile Repair. It is not a separate product. The existing Digital Forensic Laboratory is preserved, with feature expansion temporarily deprioritized.

## Case intake
A case must record an internal ID, official case number, agency, investigator, seized device description, authority reference, issuing authority, and explicit scope. Prosecutor name, court, authorization dates, attachment metadata, seizure location/time, device identifiers, custody transfers, and legal jurisdiction require dedicated UI/storage and validation.

The new header `cpp/include/nexvary/LawEnforcementCase.hpp` provides a dependency-free C++ case data model and minimum completeness checks. **Completeness is not legal validity or permission to access a device.** The model does not change device state, access credentials, or decrypt evidence.

## Next engineering slices
1. Persist case records using an encrypted, migration-tested database with role-based access; never store case data in application logs.
2. Add Case Intake Qt/QML workspace with Arabic RTL, English translations, consistent cards, actual licensed colored icon assets, and responsive layouts.
3. Add append-only custody events, evidence SHA-256 manifests, case exports and audit trail.
4. Implement device-to-case binding, authorization scope review, and preflight of supported read-only collection paths.
5. Add research/training simulation on synthetic devices; distinguish verified device capabilities from hypotheses.
6. Validate Windows build, tests, screenshots from the running application, and legal/privacy review before any production use.

## Non-goals for this increment
No PIN removal, FRP bypass, cryptographic bypass, device exploitation, live evidence extraction, or claims of forensic certification.

## Test
Compile `cpp/tests/law_enforcement_case_tests.cpp` with C++20 and include path `cpp/include`. This test currently is not wired into the root CMake test suite.
