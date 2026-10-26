function Invoke-NexvarySingleHostSmoke {
  param(
    [Parameter(Mandatory)][string]$BundleDirectory,
    [Parameter(Mandatory)][string]$TestRoot,
    [Parameter(Mandatory)][ValidatePattern('^[a-f0-9]{40}$')][string]$ExpectedSource,
    [Parameter(Mandatory)][ValidatePattern('^[a-f0-9]{64}$')][string]$ExpectedExecutableSha256,
    [Parameter(Mandatory)][ValidatePattern('^[a-f0-9]{64}$')][string]$ExpectedWorkerSha256,
    [ValidatePattern('^[a-z0-9-]+$')][string]$Label='windows10',
    [switch]$ObserveOwnedTcp
  )
  $ErrorActionPreference='Stop'
  $mrBundle=(Resolve-Path -LiteralPath $BundleDirectory).Path
  $mrRoot=(Resolve-Path -LiteralPath $TestRoot).Path
  $mrData=Join-Path $mrRoot ($Label+'-synthetic-data')
  if (Test-Path -LiteralPath $mrData) { throw 'Choose a new synthetic test label; existing data is preserved' }
  $mrOs=Get-CimInstance Win32_OperatingSystem
  $mrEvidence=Join-Path $mrRoot 'evidence'
  New-Item -ItemType Directory -Force -Path $mrEvidence | Out-Null
  $mrExe=Join-Path $mrBundle 'NEXVARY-MR.exe'
  $mrWorker=Join-Path $mrBundle 'mr_evidence_import_worker.exe'
  $mrProfile=Get-Content -LiteralPath (Join-Path $mrBundle 'BUILD_PROFILE.json') -Raw | ConvertFrom-Json
  if ($mrProfile.commit -cne $ExpectedSource -or $mrProfile.buildProfile -cne 'production-read-only') { throw 'Unexpected source/profile' }
  foreach ($mrPair in @(@($mrExe,$ExpectedExecutableSha256),@($mrWorker,$ExpectedWorkerSha256))) {
    if ((Get-FileHash -LiteralPath $mrPair[0] -Algorithm SHA256).Hash.ToLowerInvariant() -cne $mrPair[1]) { throw 'Executable digest mismatch' }
  }
  $mrSaved=@{}
  foreach ($mrName in @('PATH','QT_PLUGIN_PATH','QML_IMPORT_PATH','QML2_IMPORT_PATH','QT_QPA_FONTDIR','QT_QUICK_BACKEND')) { $mrSaved[$mrName]=[Environment]::GetEnvironmentVariable($mrName,'Process') }
  $mrProcess=New-Object System.Diagnostics.Process
  $mrTimer=[Diagnostics.Stopwatch]::StartNew()
  $mrSamples=0; $mrLoopback=$false; $mrExternal=$false; $mrObserveAvailable=[bool](Get-Command Get-NetTCPConnection -ErrorAction SilentlyContinue)
  try {
    $env:PATH="$mrBundle;$env:SystemRoot\System32;$env:SystemRoot"
    $env:QT_PLUGIN_PATH='';$env:QML_IMPORT_PATH='';$env:QML2_IMPORT_PATH='';$env:QT_QPA_FONTDIR='';$env:QT_QUICK_BACKEND='software'
    $mrStart=New-Object Diagnostics.ProcessStartInfo
    $mrStart.FileName=$mrExe
    $mrStart.Arguments='--smoke-test --data-dir "'+$mrData+'" --screenshot "'+(Join-Path $mrEvidence ($Label+'.png'))+'"'
    $mrStart.WorkingDirectory=$mrBundle
    $mrStart.UseShellExecute=$false
    $mrStart.RedirectStandardOutput=$true
    $mrStart.RedirectStandardError=$true
    $mrProcess.StartInfo=$mrStart
    if (-not $mrProcess.Start()) { throw 'Owned application could not start' }
    # Hold the actual process handle before waiting; do not infer an OS exit
    # from Start-Process wrappers or a native self-reported diagnostic.
    $mrHandle=$mrProcess.Handle
    $mrOut=$mrProcess.StandardOutput.ReadToEndAsync()
    $mrErr=$mrProcess.StandardError.ReadToEndAsync()
    while (-not $mrProcess.WaitForExit(200)) {
      if ($mrTimer.ElapsedMilliseconds -gt 60000) { $mrProcess.Kill();$null=$mrProcess.WaitForExit(5000);throw 'Owned synthetic process exceeded deadline' }
      if ($ObserveOwnedTcp -and $mrObserveAvailable) {
        try {
          $mrRows=@(Get-NetTCPConnection -OwningProcess $mrProcess.Id -ErrorAction Stop)
          $mrSamples++
          foreach ($mrRow in $mrRows) {
            if ($mrRow.LocalAddress -in @('127.0.0.1','::1')) { $mrLoopback=$true }
            $mrAddress=[Net.IPAddress]::Parse($mrRow.RemoteAddress)
            if ($mrAddress.IsIPv4MappedToIPv6) { $mrAddress=$mrAddress.MapToIPv4() }
            if (-not [Net.IPAddress]::IsLoopback($mrAddress) -and $mrAddress.ToString() -notin @('0.0.0.0','::')) { $mrExternal=$true }
          }
        } catch { if ($_.CategoryInfo.Category -eq [Management.Automation.ErrorCategory]::ObjectNotFound) { $mrSamples++ } else { $mrObserveAvailable=$false } }
      }
    }
    $mrExit=[int]$mrProcess.ExitCode
    if (-not $mrOut.Wait(5000) -or -not $mrErr.Wait(5000)) { throw 'Owned process output drain deadline' }
    [IO.File]::WriteAllText((Join-Path $mrEvidence ($Label+'.stdout.txt')),$mrOut.Result,[Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText((Join-Path $mrEvidence ($Label+'.stderr.txt')),$mrErr.Result,[Text.UTF8Encoding]::new($false))
    foreach ($mrPair in @(@($mrExe,$ExpectedExecutableSha256),@($mrWorker,$ExpectedWorkerSha256))) {
      if ((Get-FileHash -LiteralPath $mrPair[0] -Algorithm SHA256).Hash.ToLowerInvariant() -cne $mrPair[1]) { throw 'Executable changed during testing' }
    }
    $mrPolicyPath=Join-Path $mrEvidence ($Label+'-government-offline.json')
    $mrPolicy=$null
    if (Test-Path -LiteralPath $mrPolicyPath) { $mrPolicy=Get-Content -LiteralPath $mrPolicyPath -Raw | ConvertFrom-Json }
    $mrImage=Join-Path $mrEvidence ($Label+'-government-offline-ar.png')
    $mrRecord=[ordered]@{
      schema='nexvary-single-host-evaluation/v1';source=$ExpectedSource
      os=$mrOs.Caption;version=$mrOs.Version;build=$mrOs.BuildNumber;architecture=$mrOs.OSArchitecture;renderer='Qt Quick software'
      synthetic=$true;signed=$false;exitCode=$mrExit;durationMs=$mrTimer.ElapsedMilliseconds
      executableSha256=$ExpectedExecutableSha256;importWorkerSha256=$ExpectedWorkerSha256
      sdkExcludedFromPath=$true;defaultDeniedCategories=$mrPolicy.defaultDeniedCategories
      signedPolicyVerified=($mrPolicy.signedPolicyAudit -and $mrPolicy.policyVerification.valid)
      offlineAfterRevoke=$mrPolicy.offlineAfterRevoke
      screenshotProduced=(Test-Path -LiteralPath $mrImage)
      tcpSamplingRequested=[bool]$ObserveOwnedTcp;tcpSamplingAvailable=$mrObserveAvailable
      tcpSamples=$mrSamples;sampledLoopbackObserved=$mrLoopback;sampledExternalTcpObserved=$mrExternal
      scope='One existing Windows host; owned portable application only; synthetic app-only screenshots. TCP snapshots can miss transient traffic and do not cover DNS, UDP, proxies, descendants, shares, browsers or exports. No phone, installer, clean-machine or institutional acceptance.'
    }
    $mrRecord | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $mrEvidence ($Label+'-single-host-receipt.json')) -Encoding UTF8
    $mrRecord | ConvertTo-Json
    if ($mrExit -ne 0) { throw 'Owned application smoke failed; its output and receipt are preserved' }
    if (-not $mrRecord.signedPolicyVerified -or -not $mrRecord.offlineAfterRevoke -or -not $mrRecord.screenshotProduced) { throw 'Synthetic application receipts are incomplete' }
  } finally {
    foreach ($mrName in $mrSaved.Keys) { [Environment]::SetEnvironmentVariable($mrName,$mrSaved[$mrName],'Process') }
    $mrProcess.Dispose()
  }
}
