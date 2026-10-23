# Color icon system

The rebuild uses actual Microsoft Fluent UI System Icons **color SVG assets**.
The existing ten navigation icons were already upstream artwork; they were not
emoji or hand-generated geometric placeholders. This increment extends that
same family rather than mixing unrelated styles or recoloring outline icons.

- Upstream: <https://github.com/microsoft/fluentui-system-icons>
- Pinned revision: `a347af95478f989038cccbf085978865264c048a`
- License: MIT; complete upstream copyright/permission notice is already shipped
  at `packaging/licenses/FLUENT-ICONS-MIT.txt`.
- Exact upstream path, Git blob ID and SHA-256 for every bundled upstream SVG:
  `assets/icons/PROVENANCE.json`.
- New SVG files are copied byte-for-byte from that revision. Download validation
  checks Git's blob digest as well as recording SHA-256. Files remain local Qt
  resources; no CDN or icon font is required at runtime.

| UI purpose | Asset |
| --- | --- |
| Dashboard | `home.svg` |
| Device Manager | `phone.svg` |
| Repair Studio | `toolbox.svg` / `wrench.svg` |
| Firmware Intelligence | `arrow_square_down.svg` / `board.svg` |
| USB Diagnostics | `phone_laptop.svg` (phone/workstation relationship) |
| Forensic Laboratory | `document_lock.svg` / `search_visual.svg` |
| Recovery & Backup | `history.svg` / `database.svg` |
| Workshop CRM | `clipboard_task.svg` |
| Reports | `document_text.svg` |
| Research & Compatibility | `book_open_lightbulb.svg` / `puzzle_piece.svg` |
| Settings | `settings.svg` |
| Help & About | `question_circle.svg` |
| Diagnostics / result status | `gauge.svg`, `shield_checkmark.svg`, `checkmark_circle.svg`, `error_circle.svg`, `warning.svg` |
| Refresh / workflow | `arrow_sync.svg`, `data_bar_vertical_ascending.svg` |

These semantic mappings are NEXVARY design choices; the image artwork is
unmodified upstream. The pinned family has no 24-color USB/port glyph among
the inspected USB entries, so the USB page uses the matching color phone/laptop
asset rather than inventing a USB drawing or silently tinting an outline.

The separate original `check_mark.svg` control asset retains its existing
provenance entry. It is not represented as Microsoft artwork. Qt SVG and its
runtime deployment/license obligations remain covered by `docs/THIRD_PARTY.md`.
The distributor must preserve notices when packaging the application.
