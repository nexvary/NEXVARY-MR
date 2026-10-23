$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
# Fixed read-only inventory. No serial, path or user expression is interpolated.
$signed = @{}
$signatureInventoryAvailable = $true
try {
    Get-CimInstance Win32_PnPSignedDriver | ForEach-Object {
        if ($_.DeviceID) { $signed[$_.DeviceID] = $_ }
    }
} catch { $signatureInventoryAvailable = $false }
$nodes = @(Get-PnpDevice -PresentOnly | Where-Object { $_.InstanceId.StartsWith('USB\', [StringComparison]::OrdinalIgnoreCase) })
if ($nodes.Count -gt 256) { throw 'USB inventory exceeds the supported bound' }
$items = @($nodes | ForEach-Object {
    $node = $_
    $props = @{}
    Get-PnpDeviceProperty -InstanceId $node.InstanceId -KeyName DEVPKEY_Device_Parent,DEVPKEY_Device_Service,DEVPKEY_Device_ProblemCode -ErrorAction SilentlyContinue | ForEach-Object { $props[$_.KeyName] = $_.Data }
    $driver = $signed[$node.InstanceId]
    [ordered]@{
        instanceId = $node.InstanceId
        parentId = $props['DEVPKEY_Device_Parent']
        name = $node.FriendlyName
        service = $props['DEVPKEY_Device_Service']
        problemCode = $props['DEVPKEY_Device_ProblemCode']
        infName = $(if ($driver) { $driver.InfName } else { $null })
        provider = $(if ($driver) { $driver.DriverProviderName } else { $null })
        version = $(if ($driver) { $driver.DriverVersion } else { $null })
        signed = $(if ($driver) { $driver.IsSigned } else { $null })
    }
})
[ordered]@{ schemaVersion = 1; signatureInventoryAvailable = $signatureInventoryAvailable; devices = $items } | ConvertTo-Json -Depth 5 -Compress
