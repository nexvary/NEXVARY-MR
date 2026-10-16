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
& "$qtRoot/bin/windeployqt.exe" --release --qmldir (Join-Path $repo 'qml') --no-translations (Join-Path $bundle 'NEXVARY-MR.exe')
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }
# SQL plug-ins are runtime-loaded, so require the SQLite driver explicitly.
New-Item -ItemType Directory -Force (Join-Path $bundle 'sqldrivers') | Out-Null
Copy-Item "$qtRoot/plugins/sqldrivers/qsqlite.dll" (Join-Path $bundle 'sqldrivers/qsqlite.dll')
if (Test-Path "$qtRoot/plugins/imageformats") {
  New-Item -ItemType Directory -Force (Join-Path $bundle 'imageformats') | Out-Null
  Copy-Item "$qtRoot/plugins/imageformats/*" (Join-Path $bundle 'imageformats') -Recurse -Force
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
$manifest = Get-ChildItem $bundle -Recurse -File | Sort-Object FullName | ForEach-Object {
  $relative = [IO.Path]::GetRelativePath((Resolve-Path $bundle), $_.FullName)
  "$( (Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower() )  $relative"
}
$manifest | Set-Content (Join-Path $bundle 'SHA256SUMS.txt') -Encoding utf8
Compress-Archive -Path $bundle -DestinationPath (Join-Path $OutputDirectory 'NEXVARY-MR-Windows-x64-portable-unsigned.zip') -Force
