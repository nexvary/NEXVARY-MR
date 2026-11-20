param(
  [string]$BundleDirectory = 'dist/NEXVARY-MR',
  [string]$EvidenceDirectory = 'dist/evidence',
  [string]$Label = 'portable'
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$bundle = (Resolve-Path $BundleDirectory).Path
New-Item -ItemType Directory -Force $EvidenceDirectory | Out-Null
$evidence = (Resolve-Path $EvidenceDirectory).Path
$temporaryRoot = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { [IO.Path]::GetTempPath() }
$isolated = Join-Path $temporaryRoot "NEXVARY MR isolated $Label $([guid]::NewGuid().ToString('N'))"
New-Item -ItemType Directory $isolated | Out-Null
Copy-Item "$bundle/*" $isolated -Recurse -Force
$exe = Join-Path $isolated 'NEXVARY-MR.exe'
$data = Join-Path $isolated 'test data'
$picture = Join-Path $evidence "$Label.png"
$stdout = Join-Path $evidence "$Label.stdout.txt"
$stderr = Join-Path $evidence "$Label.stderr.txt"
$savedPath = $env:PATH
$savedQtPlugin = $env:QT_PLUGIN_PATH
$savedQml = $env:QML_IMPORT_PATH
$savedQml2 = $env:QML2_IMPORT_PATH
try {
  # Exclude Qt SDK and developer tools: load libraries only from the bundle/OS.
  $env:PATH = "$isolated;$env:SystemRoot/System32;$env:SystemRoot"
  $env:QT_PLUGIN_PATH = ''
  $env:QML_IMPORT_PATH = ''
  $env:QML2_IMPORT_PATH = ''
  $env:QT_QUICK_BACKEND = 'software'
  $arguments = "--smoke-test --data-dir `"$data`" --screenshot `"$picture`""
  $process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $isolated -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
  if (-not $process.WaitForExit(60000)) { $process.Kill(); throw "$Label smoke test timed out" }
  if ($process.ExitCode -ne 0) { throw "$Label smoke test exited $($process.ExitCode); see $stderr" }
  if (-not (Test-Path $picture)) { throw "$Label screenshot was not produced" }
  if ((Get-Item $picture).Length -lt 1000) { throw "$Label screenshot is empty or invalid" }
  "$Label GUI smoke exit=0; isolated deployment; screenshot=$picture" | Set-Content (Join-Path $evidence "$Label.result.txt")
} finally {
  $env:PATH = $savedPath
  $env:QT_PLUGIN_PATH = $savedQtPlugin
  $env:QML_IMPORT_PATH = $savedQml
  $env:QML2_IMPORT_PATH = $savedQml2
}
