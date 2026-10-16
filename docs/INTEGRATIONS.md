# External tool integration register

## Implemented adapters (C++ / Qt)
- Android platform-tools: locate `adb` and `fastboot` in PATH; list devices using only `adb devices -l` and `fastboot devices`. No auto-installation, flashing, unlocking or destructive actions.
- scrcpy: detect the official executable in PATH and start it in a separate process **only on explicit user click**. This is external-process integration, not vendored source, embedded video or an automatic installation. Android debugging and device authorization are required for ordinary USB mirroring.

## Licensing and distribution
- scrcpy official repository: https://github.com/Genymobile/scrcpy (Apache-2.0). Obtain binaries only from official releases and verify license/notices before redistribution.
- Android platform-tools: obtain from Google official SDK distribution; redistribution and terms need separate review.
- No third-party binaries or firmware are committed or distributed by this repository.

## Pending evaluation
OpenBoardView, TRACE Boardviewer, PixelFlasher, FixTrack, ERPNext Repairs and device catalogs require verification of repository identity, licenses, data rights and Windows integration before any source code is incorporated.

## Safety boundary
These adapters do not bypass locks, FRP, consent prompts or authentication, and do not modify device identity.
