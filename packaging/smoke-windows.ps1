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
  $arguments = "--smoke-test --data-dir `"$data`" --screenshot `"$picture`""
  $process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $runtimeDirectory -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
  if (-not $process.WaitForExit(60000)) { $process.Kill($true); [void]$process.WaitForExit(5000); throw "$Label smoke test timed out" }
  if ($process.ExitCode -ne 0) { throw "$Label smoke test exited $($process.ExitCode); see $stderr" }
  if (-not (Test-Path $picture)) { throw "$Label screenshot was not produced" }
  if ((Get-Item $picture).Length -lt 1000) { throw "$Label screenshot is empty or invalid" }
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
