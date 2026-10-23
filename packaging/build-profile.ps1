# Windows GUI-subsystem applications do not reliably set PowerShell's
# $LASTEXITCODE. Own the process and its redirected handles explicitly.
function Get-NexvaryBuildProfile {
  [CmdletBinding()]
  param(
    [Parameter(Mandatory)][string]$FilePath,
    [ValidateRange(25,30000)][int]$TimeoutMilliseconds = 10000
  )
  $resolved = (Resolve-Path -LiteralPath $FilePath -ErrorAction Stop).Path
  $info = [System.Diagnostics.ProcessStartInfo]::new()
  $info.FileName = $resolved
  $info.WorkingDirectory = Split-Path $resolved -Parent
  $info.UseShellExecute = $false
  $info.CreateNoWindow = $true
  $info.RedirectStandardOutput = $true
  $info.RedirectStandardError = $true
  $info.ArgumentList.Add('--build-profile')
  $process = [System.Diagnostics.Process]::new()
  $process.StartInfo = $info
  $started = $false
  try {
    if (-not $process.Start()) { throw 'Compiled profile process did not start' }
    $started = $true
    # Drain both pipes concurrently, including when the child exits nonzero.
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
    if (-not $process.WaitForExit($TimeoutMilliseconds)) {
      $process.Kill($true)
      if (-not $process.WaitForExit(5000)) { throw 'Compiled profile process could not be stopped' }
      throw 'Compiled profile probe timed out'
    }
    $stdout = $stdoutTask.GetAwaiter().GetResult()
    $stderr = $stderrTask.GetAwaiter().GetResult()
    if ($process.ExitCode -ne 0) { throw "Compiled profile probe exited $($process.ExitCode)" }
    if ($stdout.Length -gt 4096 -or $stderr.Length -gt 4096) { throw 'Compiled profile response exceeds its limit' }
    if (-not [string]::IsNullOrWhiteSpace($stderr)) { throw 'Compiled profile probe wrote to stderr' }
    $profile = $stdout.Trim()
    if ($profile -cne 'production-read-only' -and $profile -cne 'laboratory-experimental') {
      throw 'Compiled profile probe returned an unidentified profile'
    }
    return $profile
  } finally {
    # Also stop an owned child if reading or validating its response throws.
    if ($started -and -not $process.HasExited) {
      $process.Kill($true)
      [void]$process.WaitForExit(5000)
    }
    $process.Dispose()
  }
}

function Assert-NexvaryProductionBinary {
  [CmdletBinding()]
  param([Parameter(Mandatory)][string]$FilePath)
  $profile = Get-NexvaryBuildProfile -FilePath $FilePath
  if ($profile -cne 'production-read-only') {
    throw 'Production packaging refuses a Laboratory or unidentified binary'
  }
  return $profile
}
