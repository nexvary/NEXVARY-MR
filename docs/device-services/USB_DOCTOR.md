# NEXVARY USB Doctor — observation-based connection diagnosis

Engineering increment 2026-10-24. No device write, driver installation, protection
bypass or external NEXVARY server is introduced. New forensic feature work is
paused; existing forensic modules remain covered by regression tests.

## Research → feasibility → implementation

Primary references:

- [Android ADB connection states](https://developer.android.com/tools/adb): a
  device row is a server observation, not proof that Android finished booting.
- [Android connection troubleshooting](https://developer.android.com/studio/run/device):
  compare data cables/ports and development settings before assuming a cause.
- [AOSP fastbootd](https://source.android.com/docs/core/architecture/bootloader/fastbootd):
  `is-userspace` distinguishes reported fastbootd from bootloader fastboot. The
  existing reviewed read-only capability probe supplies this value; detection
  alone does not infer it.
- [Microsoft Device Manager problem names](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/device-manager-error-messages)
  and [Code 28](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/cm-prob-failed-install):
  read reported problem codes instead of guessing from a missing ADB row.
- [Microsoft Code 43](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/cm-prob-failed-post-start):
  a reported device failure does not establish the physical root cause.

The engineering prototype is a pure C++ interpreter, `ConnectionDiagnosis`,
with no process/network/filesystem API. ToolBridge supplies timestamped results
from its existing bounded discovery runner and fixed Windows inventory script.

## User workflow

Open USB Diagnostics → NEXVARY USB Doctor. Discover ADB and/or Fastboot; select a
target through Device Manager. Windows inventory can now run even when no phone
is discoverable. Each source shows collection outcome, UTC time and freshness.
Findings explain the observation, its limits and the next non-destructive check.

`reported` means a protocol or Windows supplied the value; it is not cryptographic
attestation or an independently verified cause. `unknown` means unresolved.
The two-minute interpretation window is a local engineering policy, not a USB
standard or a claim that a connection remains stable for that period. Device
service execution retains its independent, stricter review/compatibility gates.

Implemented rules: missing tools, failed/cancelled scans, expired/invalid/future
observations, successful-empty scans, missing/ambiguous targets, authorized ADB,
unauthorized/offline/recovery/sideload/rescue/no-permissions states, reported
Fastboot subtypes, target correlation and Windows problem codes 10/22/28/31/43/52.
Other nonzero problem codes remain generic reported codes. Unknown/zero/missing
codes never certify health. Code 52 guidance keeps signature enforcement enabled.

USB attribution uses the exact instance serial and parent ancestry. Unrelated
nodes are never assigned to a selected phone when correlation fails. Without a
target, findings explicitly concern the host inventory only. VID/PID/MI are
instance-ID declarations, not interface-descriptor reads or certified bindings.
Node details are hidden until explicitly expanded and may contain identifiers.
No report exporter or persistent customer/device log is added by this increment.

## Acceptance and remaining scope

- Pure engine tests include state classification, boundaries at 120000 ms,
  future/invalid timestamps, wrong-target/stale profiles, multi-transport
  ambiguity, unrelated-phone PnP errors and malformed/bounded JSON inventory.
- Native bridge test covers completed discovery, selected authorization states,
  failed-scan invalidation and tool-configuration invalidation.
- Qt/QML tests navigate to the real panel, render AR/EN and execute the read-only
  host-inventory action. Existing shell tests cover all sections at three sizes
  and scales 1/1.25/1.5. These are synthetic/offscreen acceptance, not phone tests.
- Exact-commit Windows/Linux CI and generated packaging must be checked before
  promoting this source. Physical USB/cable/OEM/mode transitions, continuous
  disconnect history, USB descriptor acquisition and interface binding remain
  hardware-pending or unimplemented. No competitor superiority is claimed.
