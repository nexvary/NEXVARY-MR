# Uploaded Lampyre Windows client: static inspection

Inspected 2026-11-11. Input: user-supplied `Lampyre-1.10.4.exe`, 313,464,328 bytes.
SHA-256: `0b0851e5b9c1816be0e83d8779a255355afa2b1f632c5915961ae37ce6d33da4`.

## Observed, not inferred from a marketing page

- Installer is a Windows PE32 bootstrapper, Inno Setup 6.4.3 Unicode.
- PE version resource: ProductVersion 1.10.4; FileVersion 1.10.423.212;
  company Data Tower Trade and Services L.L.C-FZ. Bootstrapper architecture is
  not a claim about the installed application's architecture.
- Extracted 550 files (522,490,917 bytes) to a temporary analysis directory,
  without executing the installer or client. The installer contains duplicate
  conditional paths; this extracted set is not an acceptance of every install
  profile or Windows dependency.
- Contains Lampyre.exe/Lampyre.dll, managed .NET assemblies and desktop runtime
  libraries. DevExpress v24.1 XtraBars/XtraGrid/XtraLayout and Windows Forms
  dependencies are present; WPF and WebView2 libraries are also bundled.
  Bundling alone does not prove which path is used for a particular screen.
- Product modules separate core analysis/search/data from Card, Table, Scene
  graph, GIS and Report presentation modules.
- Manifest resource names in Lampyre.dll include ExplorerDock, FileImportDock,
  InstancesDock, InstrumentsDock, LampyriseDock, LayersDock, LinkerDock,
  NotesDock, NotificationsDock, OriginDock, ProceduresDock and PropertiesDock.
- UI metadata references docking/tabbed workspaces. Core analysis resources
  reference criteria trees, request lists and input parameters. These are
  structural observations; they do not prove a feature works at runtime.
- Bundled lighthouse.py, lighthouse_dispatcher.py and lighthouse_ontology.py
  concern the local extension framework. They are not treated as permission,
  schema or authentication documentation for the account's remote API/MCP.
- No standalone PDF/CHM/HTML API documentation was present in the extracted
  file inventory. Account-specific access/capabilities remain unverified.

## Extraction method and limits

Released innoextract 1.9 and upstream master both failed to parse the new header.
Built the open-source extraction tool with the narrowly scoped upstream PR #197
format-support patch for Inno Setup 6.4.3. Header listing and full extraction then
completed successfully. Source: https://github.com/dscharrer/innoextract/pull/197

Examined assembly resource names and metadata, not application source code.
Did not execute Lampyre, activate a license, authenticate, bypass access controls,
consume Photons or copy vendor UI resources into NEXVARY. Installer has a
certificate container naming the vendor; no full Authenticode trust/revocation
validation is claimed.

## Independent design derived from observed organization

The useful pattern is a working analysis desk, not a provider-card landing page:
request/session list; central sortable/filterable table and relationship canvas;
selected-item inspector with origin/provenance. Keep separate views for map data
only when real coordinates are available. Do not add a decorative map.

Implemented an independent `OsintAnalysisPane.qml`, using existing NEXVARY icons
and theme. A fixed local example exposes three synthetic entities, table filtering
and sorting, selected-row inspection, selectable relationship nodes, zoom/reset
and scroll. The entire dataset and links are explicitly synthetic, cost zero,
and never enter the case evidence store. Result changes clear the inspector.
Errors/cancellation expose no fixture records. No vendor code or extracted
images are included. A graph association is not proof of identity or ownership.

Native bridge and Qt UI coverage were extended for fixture records, filtering,
selection, bilingual table/graph screenshots and error-state clearing. Local
source-QML rendering uses a synthetic Python context bridge and is a component
preview, not a screenshot of the running Lampyre client or full Windows MR.
Full Windows/Qt CI and physical acceptance remain separate validation gates.
