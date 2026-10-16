# NEXVARY MR — Mobile Repair Workspace

Offline-first Windows desktop foundation for authorized mobile phone software and hardware repair.

## Features (foundation)
- Arabic/English desktop UI (PySide6)
- Windows readiness checks and Android tool discovery
- Read-only USB/ADB/Fastboot discovery
- Device catalog with searchable brands and models
- Guided diagnostic rules and repair case notes stored locally in SQLite
- No flashing, unlocking, FRP bypass, or data-erasing commands in this foundation

## Start (Windows 10/11, Python 3.11+)
1. Install Python and run `pip install -r requirements.txt`
2. Launch `python -m nexvary_mr`
3. For device discovery, install official Google Android Platform Tools.

## Test
`python -m unittest discover -s tests -v`

## Roadmap
1. Read-only diagnostics and usability tests
2. Verified OEM driver registry with signed downloads and user consent
3. Authorized firmware workflows with exact model matching and backup checks
4. Licensed boardview support, measurements and workshop workflows

Third-party projects are research references only; their code is not included. No warranty of device compatibility.
