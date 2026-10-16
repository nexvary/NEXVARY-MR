param(
  [string]$BuildDirectory = 'build',
  [string]$Configuration = 'Release',
  [string]$OutputDirectory = 'dist'
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = Split-Path $PSScriptRoot -Parent
Set-Location $repo
$qtRoot = $env:QT_ROOT_DIR
if (-not $qtRoot) { throw 'QT_ROOT_DIR must identify the Qt 6.8.3 MSVC installation' }
$exe = Join-Path $BuildDirectory "$Configuration/nexvary_mr.exe"
if (-not (Test-Path $exe)) { $exe = Join-Path $BuildDirectory 'nexvary_mr.exe' }
if (-not (Test-Path $exe)) { throw 'Built nexvary_mr.exe was not found' }
$bundle = Join-Path $OutputDirectory 'NEXVARY-MR'
New-Item -ItemType Directory -Force $bundle | Out-Null
Copy-Item $exe (Join-Path $bundle 'NEXVARY-MR.exe')
& "$qtRoot/bin/windeployqt.exe" --release --qmldir (Join-Path $repo 'qml') --no-translations --no-compiler-runtime (Join-Path $bundle 'NEXVARY-MR.exe')
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }
# Official release CRT files only: preserve portable/per-user operation without
# running a machine-wide vc_redist installer. Never copy DLLs from System32,
# compiler bin directories or debug_nonredist.
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path $vswhere -PathType Leaf)) {
  throw 'Visual Studio vswhere.exe was not found; install VS2022 C++ tools with release redistributable files'
}
$visualStudio = & $vswhere -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if ($LASTEXITCODE -ne 0 -or -not $visualStudio) { throw 'A Visual Studio 2022 C++ installation was not found' }
$redistRoot = Join-Path (([string]$visualStudio).Trim()) 'VC/Redist/MSVC'
if (-not (Test-Path $redistRoot -PathType Container)) { throw "VS2022 release redistributable directory missing: $redistRoot" }
$crtDirectories = @(Get-ChildItem $redistRoot -Directory |
  Where-Object { $_.Name -match '^\d+(\.\d+){1,3}$' } |
  Sort-Object { [version]$_.Name } -Descending |
  ForEach-Object { Join-Path $_.FullName 'x64/Microsoft.VC143.CRT' } |
  Where-Object { Test-Path $_ -PathType Container })
if ($crtDirectories.Count -eq 0) {
  throw "No release x64 Microsoft.VC143.CRT directory exists below $redistRoot; incomplete portable packaging is refused"
}
$crtDirectory = $crtDirectories[0]
$crtDlls = @(Get-ChildItem $crtDirectory -Filter '*.dll' -File)
if ($crtDlls.Count -eq 0) { throw "Release CRT directory contains no DLLs: $crtDirectory" }
$crtEvidence = foreach ($file in $crtDlls) {
  Copy-Item $file.FullName (Join-Path $bundle $file.Name) -Force
  [ordered]@{
    file = $file.Name
    version = $file.VersionInfo.FileVersion
    sha256 = (Get-FileHash (Join-Path $bundle $file.Name) -Algorithm SHA256).Hash.ToLower()
  }
}
[ordered]@{
  source = $crtDirectory
  method = 'VS2022 official release x64 CRT app-local deployment'
  files = @($crtEvidence)
} | ConvertTo-Json -Depth 4 | Set-Content (Join-Path $bundle 'CRT_DEPLOYMENT.json') -Encoding utf8
# SQL plug-ins are runtime-loaded, so require the SQLite driver explicitly.
New-Item -ItemType Directory -Force (Join-Path $bundle 'sqldrivers') | Out-Null
Copy-Item "$qtRoot/plugins/sqldrivers/qsqlite.dll" (Join-Path $bundle 'sqldrivers/qsqlite.dll')
# Board attachments accept PNG (built into QtGui) and JPEG only. Do not copy
# every SDK plug-in: that can introduce debug DLLs or qsvg without Qt6Svg.
# Keep all plug-ins and dependency DLLs already selected by windeployqt.
New-Item -ItemType Directory -Force (Join-Path $bundle 'imageformats') | Out-Null
Copy-Item "$qtRoot/plugins/imageformats/qjpeg.dll" (Join-Path $bundle 'imageformats/qjpeg.dll')
$requiredRuntime = @(
  'NEXVARY-MR.exe', 'msvcp140.dll', 'vcruntime140.dll', 'vcruntime140_1.dll',
  'Qt6Core.dll', 'Qt6Gui.dll', 'Qt6Qml.dll',
  'Qt6Quick.dll', 'Qt6Sql.dll',
  'platforms/qwindows.dll', 'sqldrivers/qsqlite.dll', 'imageformats/qjpeg.dll'
)
foreach ($relative in $requiredRuntime) {
  $file = Join-Path $bundle $relative
  if (-not (Test-Path $file -PathType Leaf) -or (Get-Item $file).Length -eq 0) {
    throw "Missing required deployment file: $relative"
  }
}
@('[Paths]', 'Prefix=.', 'Plugins=.', 'QmlImports=qml') | Set-Content (Join-Path $bundle 'qt.conf') -Encoding utf8
Copy-Item (Join-Path $repo 'docs/USER_GUIDE.md') $bundle
Copy-Item (Join-Path $repo 'docs/THIRD_PARTY.md') $bundle
Copy-Item (Join-Path $repo 'docs/INTEGRATIONS.md') $bundle
if (Test-Path (Join-Path $repo 'CHANGELOG.md')) { Copy-Item (Join-Path $repo 'CHANGELOG.md') $bundle }
New-Item -ItemType Directory -Force (Join-Path $bundle 'licenses') | Out-Null
Copy-Item (Join-Path $PSScriptRoot 'licenses/*') (Join-Path $bundle 'licenses') -Recurse -Force
# Preserve Qt's actual third-party notices when present in the SDK.
if (Test-Path "$qtRoot/licenses") { Copy-Item "$qtRoot/licenses" (Join-Path $bundle 'licenses/qt-sdk') -Recurse -Force }
$manifest = Get-ChildItem $bundle -Recurse -File | Where-Object { $_.Name -ne 'SHA256SUMS.txt' } | Sort-Object FullName | ForEach-Object {
  $relative = [IO.Path]::GetRelativePath((Resolve-Path $bundle), $_.FullName)
  "$( (Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower() )  $relative"
}
$manifest | Set-Content (Join-Path $bundle 'SHA256SUMS.txt') -Encoding utf8
Compress-Archive -Path $bundle -DestinationPath (Join-Path $OutputDirectory 'NEXVARY-MR-Windows-x64-portable-unsigned.zip') -Force
