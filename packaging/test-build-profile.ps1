param(
  [Parameter(Mandatory)][string]$FixturePath,
  [Parameter(Mandatory)][string]$ApplicationPath,
  [Parameter(Mandatory)][ValidateSet('production','laboratory')][string]$ExpectedProfile,
  [string]$EvidenceDirectory = 'dist/evidence'
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
. (Join-Path $PSScriptRoot 'build-profile.ps1')
$fixture = (Resolve-Path -LiteralPath $FixturePath).Path
$application = (Resolve-Path -LiteralPath $ApplicationPath).Path
$root = Join-Path ([IO.Path]::GetTempPath()) "NEXVARY profile probes $([guid]::NewGuid().ToString('N'))"
New-Item -ItemType Directory $root | Out-Null
$results = [System.Collections.Generic.List[object]]::new()
function Record-Result([string]$Name) {
  $results.Add([ordered]@{name=$Name; passed=$true})
  Write-Host "PASS: $Name"
}
function Expect-Rejection([string]$Name, [scriptblock]$Action, [string]$ExpectedMessage) {
  $message = ''
  try { & $Action | Out-Null } catch { $message = $_.Exception.Message }
  if ($message -notlike "*$ExpectedMessage*") {
    throw "$Name did not fail for the expected reason: $message"
  }
  Record-Result $Name
}
try {
  foreach ($mode in @('production','laboratory','failure','empty','unknown','timeout','oversized','stderr')) {
    Copy-Item -LiteralPath $fixture -Destination (Join-Path $root "mr-profile-$mode.exe")
  }
  $production = Join-Path $root 'mr-profile-production.exe'
  $laboratory = Join-Path $root 'mr-profile-laboratory.exe'
  # Neither test relies on a native exit-code variable in the caller's scope.
  Remove-Variable LASTEXITCODE -ErrorAction SilentlyContinue
  if ((Assert-NexvaryProductionBinary $production) -cne 'production-read-only') { throw 'Production response mismatch' }
  Record-Result 'GUI child in path with spaces succeeds with unset LASTEXITCODE'
  if ((Get-NexvaryBuildProfile $laboratory) -cne 'laboratory-experimental') { throw 'Laboratory response mismatch' }
  Record-Result 'Laboratory profile is read accurately'
  Expect-Rejection 'Laboratory cannot enter production package' { Assert-NexvaryProductionBinary $laboratory } 'refuses a Laboratory'
  Expect-Rejection 'Nonzero GUI exit cannot claim production' { Get-NexvaryBuildProfile (Join-Path $root 'mr-profile-failure.exe') } 'exited 17'
  Expect-Rejection 'Empty profile rejected' { Get-NexvaryBuildProfile (Join-Path $root 'mr-profile-empty.exe') } 'unidentified'
  Expect-Rejection 'Multiple profiles rejected' { Get-NexvaryBuildProfile (Join-Path $root 'mr-profile-unknown.exe') } 'unidentified'
  Expect-Rejection 'Oversized response rejected' { Get-NexvaryBuildProfile (Join-Path $root 'mr-profile-oversized.exe') } 'exceeds its limit'
  Expect-Rejection 'Error stream rejected' { Get-NexvaryBuildProfile (Join-Path $root 'mr-profile-stderr.exe') } 'stderr'
  Expect-Rejection 'Hung child terminated' { Get-NexvaryBuildProfile (Join-Path $root 'mr-profile-timeout.exe') -TimeoutMilliseconds 100 } 'timed out'
  if (Get-Process -Name 'mr-profile-timeout' -ErrorAction SilentlyContinue) { throw 'Timed-out child survived' }
  Record-Result 'No timed-out profile child remains'
  Expect-Rejection 'Missing executable rejected' { Get-NexvaryBuildProfile (Join-Path $root 'absent.exe') } 'does not exist'
  $actual = Get-NexvaryBuildProfile $application
  $expected = if ($ExpectedProfile -eq 'laboratory') { 'laboratory-experimental' } else { 'production-read-only' }
  if ($actual -cne $expected) { throw "Actual application profile mismatch: $actual" }
  Record-Result "Actual $ExpectedProfile application profile verified"
  if ($ExpectedProfile -eq 'laboratory') {
    Expect-Rejection 'Actual Laboratory application cannot be packaged' { Assert-NexvaryProductionBinary $application } 'refuses a Laboratory'
  } else {
    if ((Assert-NexvaryProductionBinary $application) -cne $expected) { throw 'Actual production packaging check failed' }
    Record-Result 'Actual production application passes packaging profile gate'
  }
} finally {
  New-Item -ItemType Directory -Force $EvidenceDirectory | Out-Null
  [ordered]@{schemaVersion=1; expectedProfile=$ExpectedProfile; results=@($results.ToArray())} |
    ConvertTo-Json -Depth 6 | Set-Content (Join-Path $EvidenceDirectory 'build-profile-regressions.json')
  Remove-Item -LiteralPath $root -Recurse -Force
}
