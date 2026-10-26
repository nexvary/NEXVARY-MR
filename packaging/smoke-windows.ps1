param(
  [string]$BundleDirectory = 'dist/NEXVARY-MR',
  [string]$EvidenceDirectory = 'dist/evidence',
  [string]$Label = 'portable',
  [switch]$RunInPlace
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$bundle = (Resolve-Path $BundleDirectory).Path
New-Item -ItemType Directory -Force $EvidenceDirectory | Out-Null
$evidence = (Resolve-Path $EvidenceDirectory).Path
$temporaryRoot = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [IO.Path]::GetTempPath() }
$isolated = Join-Path $temporaryRoot "NEXVARY MR isolated $Label $([guid]::NewGuid().ToString('N'))"
New-Item -ItemType Directory $isolated | Out-Null
if (-not $RunInPlace) { Copy-Item "$bundle/*" $isolated -Recurse -Force }
$runtimeDirectory = if ($RunInPlace) { $bundle } else { $isolated }
$exe = Join-Path $runtimeDirectory 'NEXVARY-MR.exe'
$data = Join-Path $isolated 'test data'
$picture = Join-Path $evidence "$Label.png"
$stdout = Join-Path $evidence "$Label.stdout.txt"
$stderr = Join-Path $evidence "$Label.stderr.txt"
# Application lookup can return multiple PATH matches. Capture the first resolved
# executable, rather than passing an array (joined into one invalid command path).
$verifierPython = (Get-Command python -CommandType Application -ErrorAction Stop | Select-Object -First 1).Source
if (-not (Test-Path -LiteralPath $verifierPython -PathType Leaf)) { throw 'Policy audit verifier Python executable was not found' }
$savedPath = $env:PATH
$savedQtPlugin = $env:QT_PLUGIN_PATH
$savedQml = $env:QML_IMPORT_PATH
$savedQml2 = $env:QML2_IMPORT_PATH
$savedFontDirectory = $env:QT_QPA_FONTDIR
$savedQuickBackend = $env:QT_QUICK_BACKEND
$process = $null
try {
  # Exclude Qt SDK and developer tools: load libraries only from the bundle/OS.
  $env:PATH = "$runtimeDirectory;$env:SystemRoot/System32;$env:SystemRoot"
  $env:QT_PLUGIN_PATH = ''
  $env:QML_IMPORT_PATH = ''
  $env:QML2_IMPORT_PATH = ''
  $env:QT_QPA_FONTDIR = ''
  $env:QT_QUICK_BACKEND = 'software'
  # Exercise the actual deployed child and Qt runtime using synthetic bytes only.
  $importInput = Join-Path $isolated 'synthetic import.json'
  [IO.File]::WriteAllText($importInput, '{"synthetic":"deployed import worker"}', [Text.UTF8Encoding]::new($false))
  $importHash = (Get-FileHash -LiteralPath $importInput -Algorithm SHA256).Hash.ToLower()
  $importOutput = Join-Path $evidence "$Label.authorized-import.json"
  $importError = Join-Path $evidence "$Label.authorized-import.stderr.txt"
  $importArguments = @('--input', ('"' + $importInput + '"'), '--sha256', $importHash, '--adapter', 'json')
  $import = Start-Process -FilePath (Join-Path $runtimeDirectory 'mr_evidence_import_worker.exe') -ArgumentList $importArguments -WorkingDirectory $runtimeDirectory -PassThru -RedirectStandardOutput $importOutput -RedirectStandardError $importError
  if (-not $import.WaitForExit(10000)) { $import.Kill($true); throw 'Deployed import worker timed out' }
  if ($import.ExitCode -ne 0) { throw 'Deployed import worker failed' }
  $importReport = Get-Content -LiteralPath $importOutput -Raw | ConvertFrom-Json
  if ($importReport.schema -ne 'nexvary-authorized-import/v1' -or $importReport.status -ne 'imported' -or $importReport.sourceSha256 -ne $importHash -or $importReport.findings[0].value -ne 'deployed import worker') { throw 'Deployed import worker report did not verify' }
  if ((Get-FileHash -LiteralPath $importInput -Algorithm SHA256).Hash.ToLower() -ne $importHash) { throw 'Deployed importer changed its input' }
  $adbReference = Get-Content -LiteralPath (Join-Path $PSScriptRoot '../samples/authorized-import/adb-v5-reference.json') -Raw | ConvertFrom-Json
  $adbInput = Join-Path $isolated 'synthetic backup.ab'
  [IO.File]::WriteAllBytes($adbInput, [Convert]::FromBase64String($adbReference.backupBase64))
  $adbHash = (Get-FileHash -LiteralPath $adbInput -Algorithm SHA256).Hash.ToLower()
  if ($adbHash -ne $adbReference.sourceSha256) { throw 'Synthetic ADB reference hash changed' }
  $adbOutput = Join-Path $evidence "$Label.adb-backup.json"
  $adbError = Join-Path $evidence "$Label.adb-backup.stderr.txt"
  $adbArguments = @('--input', ('"' + $adbInput + '"'), '--sha256', $adbHash, '--adapter', 'adb-backup-v5')
  $adb = Start-Process -FilePath (Join-Path $runtimeDirectory 'mr_evidence_import_worker.exe') -ArgumentList $adbArguments -WorkingDirectory $runtimeDirectory -PassThru -RedirectStandardOutput $adbOutput -RedirectStandardError $adbError
  if (-not $adb.WaitForExit(10000)) { $adb.Kill($true); throw 'Deployed ADB inventory timed out' }
  if ($adb.ExitCode -ne 0) { throw 'Deployed ADB inventory failed' }
  $adbReport = Get-Content -LiteralPath $adbOutput -Raw | ConvertFrom-Json
  if ($adbReport.status -ne 'imported' -or $adbReport.sourceSha256 -ne $adbHash -or -not $adbReport.backup.inventoryOnly -or $adbReport.backup.membersScanned -ne 4 -or $adbReport.findings.Count -ne 4) { throw 'Deployed ADB inventory report did not verify' }
  for ($i = 0; $i -lt 4; $i++) {
    $actual = $adbReport.findings[$i]; $expected = $adbReference.expected[$i]
    if ($actual.reference -ne $expected.reference -or $actual.memberSha256 -ne $expected.memberSha256 -or $actual.memberSize -ne $expected.memberSize -or $actual.sourceByteOffset -ne $expected.sourceByteOffset) { throw 'Deployed ADB member provenance differs from independent fixture' }
  }
  if ((Get-FileHash -LiteralPath $adbInput -Algorithm SHA256).Hash.ToLower() -ne $adbHash) { throw 'Deployed ADB inventory changed its source' }
  # Exercise the deployed app's real resolution with SDK/PATH tools excluded.
  $probeOutput = Join-Path $evidence "$Label.screen-tools.json"
  $probeError = Join-Path $evidence "$Label.screen-tools.stderr.txt"
  $probe = Start-Process -FilePath $exe -ArgumentList '--screen-tools-probe' -WorkingDirectory $runtimeDirectory -PassThru -RedirectStandardOutput $probeOutput -RedirectStandardError $probeError
  if (-not $probe.WaitForExit(10000)) { $probe.Kill($true); throw 'Screen tool resolution probe timed out' }
  if ($probe.ExitCode -ne 0) { throw 'Screen tool resolution probe failed' }
  $resolved = Get-Content -LiteralPath $probeOutput -Raw | ConvertFrom-Json
  if (-not $resolved.screenViewAvailable) { throw 'Consented screen view is unexpectedly unavailable in production' }
  $toolDirectory = Join-Path $runtimeDirectory 'tools/scrcpy'
  foreach ($toolName in @('adb', 'scrcpy')) {
    $entry = @($resolved.tools | Where-Object { $_.name -eq $toolName })
    $expected = [IO.Path]::GetFullPath((Join-Path $toolDirectory "$toolName.exe"))
    if ($entry.Count -ne 1 -or -not $entry[0].available -or [IO.Path]::GetFullPath($entry[0].path) -ne $expected) { throw "Bundled $toolName did not resolve independently of PATH" }
  }
  $receipt = Get-Content -LiteralPath (Join-Path $toolDirectory 'SOURCE_PROVENANCE.json') -Raw | ConvertFrom-Json
  foreach ($entry in $receipt.files.PSObject.Properties) {
    if ((Get-FileHash -LiteralPath (Join-Path $toolDirectory $entry.Name) -Algorithm SHA256).Hash.ToLower() -ne $entry.Value) { throw "Bundled screen-tool digest changed: $($entry.Name)" }
  }
  $versionOutput = Join-Path $evidence "$Label.scrcpy-version.txt"
  $versionError = Join-Path $evidence "$Label.scrcpy-version.stderr.txt"
  $version = Start-Process -FilePath (Join-Path $toolDirectory 'scrcpy.exe') -ArgumentList '--version' -WorkingDirectory $toolDirectory -PassThru -RedirectStandardOutput $versionOutput -RedirectStandardError $versionError
  if (-not $version.WaitForExit(10000)) { $version.Kill($true); throw 'Official scrcpy version probe timed out' }
  if ($version.ExitCode -ne 0 -or (Get-Content -LiteralPath $versionOutput -Raw) -notmatch '^scrcpy 5\.0\.1(?: |\r?\n)') { throw 'Official scrcpy runtime could not start in deployed isolation' }
  $arguments = "--smoke-test --data-dir `"$data`" --screenshot `"$picture`""
  $process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $runtimeDirectory -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
  if (-not $process.WaitForExit(60000)) { $process.Kill($true); [void]$process.WaitForExit(5000); throw "$Label smoke test timed out" }
  if ($process.ExitCode -ne 0) { throw "$Label smoke test exited $($process.ExitCode); see $stderr" }
  if (-not (Test-Path $picture)) { throw "$Label screenshot was not produced" }
  if ((Get-Item $picture).Length -lt 1000) { throw "$Label screenshot is empty or invalid" }
  $policyReceipt = Get-Content -LiteralPath (Join-Path $evidence "$Label-government-offline.json") -Raw | ConvertFrom-Json
  if ($policyReceipt.schema -ne 'nexvary-deployed-operation-policy/v1' -or -not $policyReceipt.synthetic -or $policyReceipt.defaultDeniedCategories -ne 6 -or -not $policyReceipt.offlineAfterRevoke -or -not $policyReceipt.startupBoundaryDenials -or -not $policyReceipt.explicitSyntheticPortalGrant -or -not $policyReceipt.portalStoppedAfterRevoke -or @($policyReceipt.permissions).Count -ne 6 -or @($policyReceipt.permissions | Where-Object { $_.allowed }).Count -ne 0) { throw 'Deployed Government operation policy receipt failed' }
  if (-not $policyReceipt.reviewedExceptionProtocol -or -not $policyReceipt.signedPolicyAudit -or -not $policyReceipt.policyVerification.valid) { throw 'Deployed signed policy audit/review protocol receipt failed' }
  $auditFile = Join-Path $evidence "$Label-government-policy-audit.json"
  $auditAnchor = Join-Path $evidence "$Label-government-policy-anchor.json"
  $auditVerifier = Join-Path $PSScriptRoot '../scripts/verify_policy_audit.py'
  & $verifierPython $auditVerifier $auditFile --anchor $auditAnchor | Set-Content (Join-Path $evidence "$Label-government-policy-independent-verification.json")
  if ($LASTEXITCODE -ne 0) { throw 'Independent deployed policy audit verification failed' }
  foreach ($language in @('ar', 'en')) {
    if ((Get-Item -LiteralPath (Join-Path $evidence "$Label-government-offline-$language.png")).Length -lt 1000) { throw 'Deployed Government policy capture is missing' }
  }
  $indexReceipt = Join-Path $evidence "$Label-government-artifacts.json"
  $index = Get-Content -LiteralPath $indexReceipt -Raw | ConvertFrom-Json
  if ($index.schema -ne 'nexvary-deployed-artifact-index/v1' -or -not $index.synthetic -or $index.findingCount -ne 1 -or $index.provenancePathNodes -ne 5 -or $index.sourceSha256 -ne '38593cb1d8a2a40cd2efa222256edfad201f615fdba4c65770bd8d130fc2e851' -or $index.nodes -ne 6 -or $index.edges -ne 5 -or $index.reportSha256 -notmatch '^[0-9a-f]{64}$') { throw 'Deployed Government finding index did not verify' }
  foreach ($language in @('ar', 'en')) {
    if ((Get-Item -LiteralPath (Join-Path $evidence "$Label-government-artifacts-$language.png")).Length -lt 1000) { throw 'Deployed Government index capture is missing' }
  }
  $mode = if ($RunInPlace) { 'installed-in-place' } else { 'isolated-copy' }
  [ordered]@{
    schemaVersion=1; label=$Label; mode=$mode; executable=$exe
    exitCode=$process.ExitCode; screenshot=$picture; dataDirectory=$data
    qtSdkExcludedFromPath=$true; testFontOverrideCleared=$true
  } | ConvertTo-Json | Set-Content (Join-Path $evidence "$Label.result.json")
  "$Label GUI smoke exit=0; mode=$mode; executable=$exe; screenshot=$picture" |
    Set-Content (Join-Path $evidence "$Label.result.txt")
  Write-Host "$Label GUI smoke passed: mode=$mode; exit=0"
} finally {
  $env:PATH = $savedPath
  $env:QT_PLUGIN_PATH = $savedQtPlugin
  $env:QML_IMPORT_PATH = $savedQml
  $env:QML2_IMPORT_PATH = $savedQml2
  $env:QT_QPA_FONTDIR = $savedFontDirectory
  $env:QT_QUICK_BACKEND = $savedQuickBackend
  if ($null -ne $process) { $process.Dispose() }
}
