# Native board data and technician sessions

The native viewer renders **component geometry and net membership** from an explicitly licensed JSON file. It does not parse proprietary boardview formats, display a circuit PDF, read voltages from USB, or provide manufacturer diagnostic authority. OpenBoardView is a separate integration candidate; its code and third-party board data are not included here.

`samples/training-board.json` is original, fictional CC0-1.0 training data. It does not represent an actual phone. Its example voltage references must never be used on a real board. The in-app demo must keep this warning visible.

## Board schema: `nexvary.board.v1`

```json
{
  "format": "nexvary.board.v1",
  "title": "Authorized board title",
  "author": "Rights holder / source author",
  "license": "SPDX identifier or explicit permission reference",
  "description": "Provenance, usage limitations, and reference conditions",
  "width": 1000,
  "height": 650,
  "nets": [
    {"name": "DEMO_RAIL", "referenceVoltage": 3.3, "tolerance": 0.1},
    {"name": "DEMO_SIGNAL"}
  ],
  "components": [
    {"reference": "U_DEMO1", "name": "Fictional controller", "x": 100, "y": 100,
     "width": 150, "height": 100, "nets": ["DEMO_RAIL", "DEMO_SIGNAL"]}
  ]
}
```

The `license` field is metadata supplied by the user; the program cannot independently establish ownership. `importBoard(path, rightsConfirmed)` refuses an import until permission is affirmatively confirmed. UI callers must obtain that confirmation for external files. Bundled original training data may be loaded with `true` and identified as training data.

Validation is transactional: a failed import leaves the current board and notes intact. Files are limited to 8 MiB; there must be 1–10000 components and at most 10000 nets. Required title/author/license strings are nonblank and at most 160 characters. Optional description is at most 4000 characters. Width/height are finite, positive, and at most 100000 coordinate units. Every component rectangle must fit inside those bounds. Components have unique references and a nonblank name (160 characters maximum). Net names and component references match `[A-Za-z][A-Za-z0-9_.:-]{0,63}`. Each component's net list must contain distinct, declared net names. Unknown metadata is ignored and never executed.

`referenceVoltage` and `tolerance` must appear together. Voltage has an absolute limit of 1000 V; tolerance is an absolute, nonnegative number of volts, at most 1000 V. A measurement is tagged `within_reference`, `outside_reference`, or `no_reference`. This is an arithmetic comparison with user-supplied reference data, not a conclusion that a component is good/bad. Instrument uncertainty and operating conditions are not automatically modeled. No measurement hardware is connected.

## QObject API

Properties: `loaded`, `title`, `author`, `license`, `description`, `sourcePath`, `boardWidth`, `boardHeight`, `components`, `nets`, `measurements`, `annotations`, `dirty`, `error`. `dirty` becomes true when a measurement or annotation is added and clears after successful save, restore, or board import; the UI uses this to confirm session replacement.

- `importBoard(path, rightsConfirmed = false) -> bool`: accepts a local path, `file://` URL, or bundled `:/`/`qrc:/` resource. Remote URLs are rejected.
- `search(query) -> QVariantList`: case-insensitive substring search across component reference, name and attached net names; blank query returns all components.
- `addMeasurement(net, value, note = "") -> bool`: records a **manually entered** finite voltage, a known net, note (up to 2000 characters), UTC timestamp and computed comparison. ±1000 V limit; 10000 records maximum.
- `addAnnotation(reference, text, x, y) -> bool`: component-bound text annotation with an in-bounds point, UTC timestamp and 4000-character limit; 10000 records maximum.
- `saveSession(localPathOrFileUrl) -> bool`: saves an atomic JSON sidecar with `QSaveFile`; refuses to overwrite the board source. Session contents are plain text, so use OS account/file permissions for sensitive notes.
- `loadSession(localPathOrFileUrl) -> bool`: transactionally restores a validated session only when its SHA-256 board fingerprint matches the exact source bytes. Comparison results are recomputed, not trusted from the sidecar.

`boardChanged`, `sessionChanged`, and `errorChanged` notify QML. Failed operations set `error`; successful mutations clear it. The bridge has no Qt Quick dependency; Qt Core and Qt Test suffice for its test target. QML should use `Text.PlainText` when rendering user-supplied text.

Sessions use `nexvary.board-session.v1`, with `boardSha256`, `measurements` and `annotations` arrays. Reimporting a valid board starts a new empty session; save current work before switching files. Board photos, pin geometry, electrical traces, PDFs and binary formats are currently unsupported. There are no destructive device commands in this component.
