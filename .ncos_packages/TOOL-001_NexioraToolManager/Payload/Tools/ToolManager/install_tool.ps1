param([Parameter(Mandatory=$true)][string]$Root,[Parameter(Mandatory=$true)][string]$ToolId)
$ErrorActionPreference = 'Stop'
$rootPath = (Resolve-Path $Root).Path
$catalogPath = Join-Path $rootPath 'Config\Tools\catalog.psd1'
if (-not (Test-Path $catalogPath)) { throw "Tool catalog not found: $catalogPath" }
$catalog = Import-PowerShellDataFile $catalogPath
if (-not $catalog.ContainsKey($ToolId)) { throw "Unsupported tool: $ToolId" }
$entry = $catalog[$ToolId]
if ($entry.Url -notmatch '^https://github\.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp\.exe$') { throw 'Source URL is not allowlisted.' }
$toolDir = Join-Path $rootPath ("Tools\Managed\" + $ToolId)
$registryDir = Join-Path $rootPath 'Tools\Registry'
New-Item -ItemType Directory -Force -Path $toolDir,$registryDir | Out-Null
$final = Join-Path $toolDir $entry.Executable
$temp = "$final.download"
$backup = "$final.rollback"
Remove-Item -Force -ErrorAction SilentlyContinue $temp
Invoke-WebRequest -UseBasicParsing -Uri $entry.Url -OutFile $temp
if ((Get-Item $temp).Length -lt 100000) { Remove-Item -Force $temp; throw 'Downloaded artifact is unexpectedly small.' }
$hash = (Get-FileHash -Algorithm SHA256 $temp).Hash.ToLowerInvariant()
if (Test-Path $final) { Copy-Item -Force $final $backup }
Move-Item -Force $temp $final
$version = (& $final --version 2>$null | Select-Object -First 1)
if ([string]::IsNullOrWhiteSpace($version)) {
    if (Test-Path $backup) { Move-Item -Force $backup $final }
    throw 'Installed tool did not report a version.'
}
$registry = Join-Path $registryDir "$ToolId.registry"
$registryTemp = "$registry.tmp"
@("id=$ToolId","version=$version","executable=$final","sha256=$hash","source_url=$($entry.Url)") | Set-Content -Encoding ascii $registryTemp
Move-Item -Force $registryTemp $registry
Write-Host "status=INSTALLED"
Write-Host "id=$ToolId"
Write-Host "version=$version"
Write-Host "executable=$final"
Write-Host "sha256=$hash"
