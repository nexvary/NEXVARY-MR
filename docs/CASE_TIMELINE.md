# Government case timestamp timeline

This is an original NEXVARY read-only projection of **observed timestamp literals**
from the existing protected case's authorized-import reports. It is not a claim
that a quoted date describes a real event, or a full mobile application timeline.

Open an authorized case, import a supported registered source, then open Forensic
Analysis > Observed timestamp timeline > Build timeline. Literal search covers
time, reference, source name/identifiers and provenance. Twelve observations are
shown per page; Previous/Next remain reachable. Arabic and English controls share
the native engine. Closing the case, changing its revision or signing out clears
the view. Global cancel and exit cancel include timeline work.

## Time interpretation

Only whole values matching four-digit calendar year, month/day, T, hour/minute/
second, optional 1–3 fractional digits and Z or explicit numeric offset are
normalized. Independent fixed UTC expectations are in timeline_tests.cpp.
Calendar/time validation is strict. Offset range is +/-14:00 maximum. -00:00 stays
unknown. Local dates, ambiguous formats, Android threadtime without year/zone,
shortened values, leap seconds and submillisecond precision stay unresolved with a
reason. The original observed representation and explicit offset are retained.
No workstation timezone, current year, case location or daylight-saving rule is
guessed. No precision is silently rounded. Dates outside years 0001–9999 after UTC
conversion remain unresolved.

Normalized observations sort by UTC, then stable analysis-ID/finding ordinal.
Unresolved observations follow, sorted by the same stable ID. Repeated imports
remain distinct observations with distinct analysis provenance; they are not
independent corroboration or separate real-world events. Non-date strings and
numeric epochs are outside this first projection. Timestamp-looking text may be
a quotation; the investigator must inspect its source.

## Evidence and authorization

The main thread obtains the existing protected, audit-verified case snapshot
through LawCaseStore. A QtConcurrent task validates each retained import report's
exact-byte SHA-256 and its source hash/evidence membership before projection.
Every output row retains case, device, evidence, analysis, finding reference and
both source/report hashes. Missing/mismatched evidence, altered reports, duplicate
analysis IDs and oversized inputs fail closed without partial results.

Native AnalyzeEvidence permission, active identity, case grant, actor and exact
case revision are rechecked before accepting a completed background result.
Cancellation is cooperative through a shared atomic flag. Data and closures
passed to the worker have no QObject/database pointer, file/network access or
write callback. The case is never mutated by building/searching the view.
Existing protected backups retain source reports; restoring them permits
reconstruction without a new timeline store.

A report hash establishes consistency with retained protected case history; the
projection does **not** reread/reverify current original-file bytes. Use existing
Verify Evidence for that separate check. Local accounts/clocks/hashes do not prove
institutional identity, legal authority or trusted external time.

## Bounds and validation

At most 100 existing analysis runs, 128 findings per import, 64 KiB per report,
256 search characters and 2,048 retained matching observations. Counts disclose
additional omitted matches and reports whose source import was already truncated.
Selection applies the hard cap in retained run/finding order **before** UTC
sorting; it is not the earliest 2,048 observations in the whole case. Rebuilding
with a literal filter can expose different matching subsets. Existing protected
case limits remain stricter where applicable.

Native tests cover explicit-offset fixed answers, calendar/precision ambiguity,
sorting, Arabic literal search, tampering, bounds/cancellation, role/grant checks,
parallel revisions and backup reconstruction. A 12,800-observation synthetic
benchmark records elapsed milliseconds; it is not a competitor benchmark or
large-evidence acceptance test. UI captures use synthetic data on multiple
sizes/directions/DPI settings. See DEVELOPMENT_STATE.md for actual CI status.

Not yet implemented: semantic event inference, timestamps embedded in arbitrary
sentences, numeric epochs, PCAP/EXIF integration, named timezone review, person
relationship analysis, retained signed timeline review, timeline PDF/export,
institutional identity and clean Windows 10/11/device acceptance.
