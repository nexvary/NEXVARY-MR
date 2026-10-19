# Selected-device Windows USB correlation

This increment adds a separate read-only collector and a conservative correlation layer. Existing PnPUtil inventory remains available.

## Behavior and scope

Select one discovered device, then use Diagnostics > Official drivers > Read selected phone drivers. A bundled fixed PowerShell script enumerates present USB instances, parent IDs, service/problem code and Win32_PnPSignedDriver metadata. The application launches the system Windows PowerShell executable with no profile or user-supplied expressions. It does not install drivers, elevate, alter execution policy, disable signing or change the device.

Correlation requires exactly one physical `USB\VID_xxxx&PID_xxxx\serial` instance whose serial matches in Windows' case-insensitive form. Child nodes are included only through explicit parent ancestry. Duplicate physical serials are ambiguous. Network serials, OS-generated IDs that differ from the Android serial, absent data and incomplete inventory are not guessed. Cycles and traversal depth are bounded. The collector caps inventory at 256 nodes and the process runner caps output at 512 KiB and runtime at 30 seconds.

A serial-correlated node tree does **not** independently establish which particular interface ADB or Fastboot opened. `interfaceBindingVerified` therefore stays false. Driver signature is Windows-reported metadata, not an independent cryptographic verification. Permission failures in optional signature retrieval leave signature status unknown. This is a timestamped observation, cleared on device selection changes and discovery refresh; it is not continuous monitoring.

The UI shows associated INF, provider, version, service and problem code in Arabic/English. JSON reports add `usbDriverCorrelation` without changing existing field types. An empty object means no retained correlation snapshot. No operation is enabled by a driver match.

## Evidence

Microsoft documentation reviewed 2026-10-19:
- https://learn.microsoft.com/en-us/powershell/module/pnpdevice/get-pnpdeviceproperty
- https://learn.microsoft.com/en-us/windows-hardware/drivers/install/devpkey-device-parent
- https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa394354(v=vs.85)

No third-party code or driver binaries were copied. Windows-provided PowerShell/CIM/PnP APIs are runtime dependencies for this optional read-only action.

## Verification

Synthetic C++ tests cover exact matching, case normalization, composite parent ancestry, unrelated phones, substring rejection, network serial exclusion, duplicate/ambiguous instances, malformed JSON and oversized output. Windows CI executes the actual collector and validates its schema without claiming a phone match. Existing Qt navigation/packaging tests continue. Local Python tests: 4 passed. Local CMake/Qt unavailable; native results belong to the PR Actions run.

Physical phone-to-driver acceptance: **Pending Hardware Validation**. Current firmware/backup/driver-install support is unchanged.

The same delivery fixes recovery assessment messaging: missing consent, internal storage after reset, and unreadable card have separate bilingual explanations. UI tests check each reason and the localized status label; the underlying assessment gates remain unchanged.
