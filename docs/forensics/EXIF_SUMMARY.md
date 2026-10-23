# Local JPEG Exif summary

Implemented in `ExifSummary.cpp`, exposed through `RecoveryBridge::analyzeExif`
and Forensic Lab → Tools → Photo EXIF. This is a bounded metadata reader, not
camera authentication, location extraction or a complete Exif implementation.

## Research → feasibility → prototype → test → integration

- Research: primary [CIPA standards index](https://www.cipa.jp/e/std/std-sec.html),
  reviewed 2026-10-23, lists Exif 3.1 dated 2026-01-30. The
  [public Exif 2.3 specification](https://www.cipa.jp/std/documents/e/DC-008-2012_E_C.pdf)
  defines the classic JPEG APP1/TIFF structures and existing tag IDs used here;
  [CIPA's metadata mapping](https://www.cipa.jp/std/documents/e/DC-X010-2017.pdf)
  distinguishes raw dates and UTC offsets. No complete 3.1 conformance is claimed.
- Feasibility: existing Qt Core byte readers and case engine suffice; no new
  runtime library, external process or copied third-party parser is needed.
- Prototype: one pre-scan APP1 Exif segment, classic II/MM TIFF, IFD0 and one Exif
  sub-IFD. At most 8 MiB per source, 4096 markers, 256 entries per directory,
  and 256 bytes per selected ASCII field. Parsing does not decode pixels.
- Tests: both byte orders; independently generated Pillow reference; offsets,
  types, counts, repeated tags, cycle, string, marker, truncation, duplicate APP1,
  privacy and cancellation cases; 12000 deterministic mutations with sanitizer
  target. Case tests exercise consent/operator requirements, hash verification,
  save/reopen/export and cancellation. QML tests click analysis and result actions
  in AR/EN and dark/light at 1180×760 and capture real runtime windows.
- Integration: asynchronous local analysis using an immutable byte copy. The
  source is read again and SHA-256 must match before adding `exif-summary.json`
  as `native-analysis` evidence. Existing case-size, audit, custody and export
  controls apply. Windows cases continue to require DPAPI; Linux uses synthetic
  test cases only and does not silently create plaintext production cases.

## Output and limits

Allowlisted fields: manufacturer, model, software, orientation, raw DateTime,
DateTimeOriginal, DateTimeDigitized, the corresponding raw offset strings,
and declared pixel dimensions. These are editable declarations, not independently
verified facts. Invalid selected field types/encodings are rejected explicitly.

GPS directory declaration can be reported as a boolean; GPS values, device/camera
serials, owner names, comments, MakerNotes, thumbnails, unknown tags and pixel
payloads are not included. GPS and next-IFD contents are not parsed or validated.
Summary payloads contain a source hash, not the source pathname; the encrypted
case separately retains local source provenance. Make/model/software/dates and
hashes can still be identifying. Plaintext export requires explicit consent.

Dates and offsets remain raw. Unknown, blank or inconsistent date/offset values
are not interpreted as UTC or promoted into capture events in the case timeline.
Acquisition/audit time remains distinct from metadata-declared capture time.

Headers are scanned only to the first SOS or EOI. Later scans and entropy-coded
data are outside scope. A successful summary does not prove a valid complete
JPEG, authentic image, correct clock, ownership or physical device support.
Standalone TIFF, BigTIFF, HEIC, XMP and extended Exif are unsupported.

Validation results and exact publication limits are in
`docs/release/EXIF_INCREMENT_2026-10-23.md`. Current GitHub CI/artifacts at
`7c928b0` predate this increment and must not be presented as its Windows proof.
