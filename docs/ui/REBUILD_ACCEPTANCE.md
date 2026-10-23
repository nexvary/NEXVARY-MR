# Native workstation rebuild — 2026-10-23

This is an implemented Qt/QML increment on a development branch, not a Stage 60
or commercial-support certification. Native C++ service APIs and production
write-denial policy are retained. Competitive workflow inputs and the limits of
that research are in [competitive-ui-review.md](competitive-ui-review.md).

## Audit and implementation

| Area | Observed problem | Implemented change | Acceptance still required |
|---|---|---|---|
| Shell | Navigation, theme palette, dashboard and context mixed in Main.qml | AppShell, Sidebar, TopBar, OperationProgress and DeviceInspector; 12 stable sections; collapsible navigation; context-aware Back | Human Windows accessibility review |
| Design | Oversized decorative gloss, inconsistent headers, neon inputs | DesignTokens, shared Panel/ActionCard/StateBadge/EmptyState; restrained highlights; readable neutral inputs | Contrast/assistive-technology audit |
| Dashboard | Launch actions and status lacked a consistent hierarchy | Observed device/tool/order/case counts, bounded workflow strip, six operation cards, readiness and saved activity | Operator task-time comparison |
| Device Manager | Large empty discovery/results columns and scattered actions | Content-sized discovery; explicit target selection; three action cards before identity/details; eight human-readable properties; raw/log/USB/screen tabs | Physical USB/phone and scrcpy acceptance |
| Repair/USB | Diagnosis output/guidance competed for vertical space | Content-sized guidance/history, existing service/board/fault/security routes; dedicated USB route avoids unrelated tab bar | More OEM acceptance; full manual button audit |
| Firmware | Fixed-size form areas and weak selected-device context | Responsive package/identity/result hierarchy; existing AVB and ZIP analysis retained | OEM authenticity/full partition compatibility remain separate gaps |
| Forensics/recovery | Forensic landing repeated source assessment | Forensic case workspace opens directly; source assessment stays in Recovery; tools/evidence/timeline retained | Populated complex cases and external-tool acceptance |
| CRM | Unrelated bright-green input borders | Shared card headers and neutral inputs, existing orders/profiles/barcode/inventory preserved | Native printer and high-volume workflow review |
| Reports/Help | No dedicated route for reports or workstation guidance | Source-linked report cards; Help/About, scopes, sources and licensing | Editorial/translation and operator review |
| Settings/security | Tall filler and unclear engine choice | Responsive tool cards; grouped DB backup; explicit clamscan.exe picker; saved config before FreshClam | Live signature-CDN acceptance separate from synthetic tests |
| Icons | Existing set too small for coherent navigation | 24 original MIT Microsoft Fluent color SVG assets, pinned provenance and offline Qt resources | License/transitive distribution review |

## Reproducible evidence

- Linux Qt 6.4.2/GCC 13 Release, warnings as errors: 31/31 CTest suites passed.
- Final production UI regression: 37 QtTest results passed, zero QML warnings.
- Three logical window sizes: 1366×768, 1920×1080, 2560×1440, AR/EN.
- Separate Qt scale-factor 1.25 and 1.5 CTest runs exercise every section and
  capture real frames. They are offscreen rendering tests, not consumer Windows
  display acceptance or screen-reader certification.
- The child-process ADB fixture covers connected/identified, unauthorized,
  offline, failure, in-progress and cancellation states without real ADB/USB.
  Fixture identities and CRM records are synthetic; no customer input is copied.
- Before images use QML from local source commit `9ffcbd45500854d58c63ca043545359bbdd0d51f`
  with the same Qt runtime and 1366×768 window. After images are actual rendered
  application windows, never generated mockups. Some after images contain records
  created by the UI regression suite; they are labelled Synthetic.
- Selected PNGs and test logs: `evidence/ui-rebuild-2026-10-23/`. The full CI
  evidence artifact includes per-section PNGs from all three scale settings.

Windows build/packaging, clean Windows 10/11 with real display scaling, physical
phone tests, every nested dialog/button and accessibility acceptance are not
inferred from Linux screenshots. No licensed competitor benchmark was run.
Do not merge to main or publish a final release on the strength of these checks.

## Continue

1. Publish review branch and run exact-source Windows production/Laboratory CI;
   inspect actual failures and packaging evidence before offering binaries.
2. Review Windows screenshots and clean-host scaling; complete dense CRM and
   case/evidence/dialog ergonomics with populated authorized fixtures.
3. Verify physical device/OEM/scrcpy, printer, backup restore and operation results.
4. Continue independent firmware/backup parsing and privacy/update release gaps
   recorded in `docs/REQUIREMENTS_GAP_MATRIX.md` without inventing support claims.
