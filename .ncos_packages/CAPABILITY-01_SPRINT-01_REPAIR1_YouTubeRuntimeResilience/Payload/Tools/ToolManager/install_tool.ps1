param([Parameter(Mandatory=$true)][string]$Root,[Parameter(Mandatory=$true)][string]$ToolId)
$ErrorActionPreference = 'Stop'
$rootPath = (Resolve-Path $Root).Path
$catalogPath = Join-Path $rootPath 'Config\Tools\catalog.psd1'
if (-not (Test-Path $catalogPath)) { throw "Tool catalog not found: $catalogPath" }
$catalog = Import-PowerShellDataFile $catalogPath
if (-not $catalog.ContainsKey($ToolId)) { throw "Unsupported tool: $ToolId" }
$entry = $catalog[$ToolId]
$allowed = @(
  'https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe',
  'https://github.com/denoland/deno/releases/latest/download/deno-x86_64-pc-windows-msvc.zip'
)
if ($allowed -notcontains $entry.Url) { throw 'Source URL is not allowlisted.' }
$toolDir = Join-Path $rootPath ("Tools\Managed\" + $ToolId)
$registryDir = Join-Path $rootPath 'Tools\Registry'
New-Item -ItemType Directory -Force -Path $toolDir,$registryDir | Out-Null
$final = Join-Path $toolDir $entry.Executable
$download = Join-Path $toolDir ("$ToolId.download")
$extract = Join-Path $toolDir '.extract'
$backup = "$final.rollback"
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $download,$extract
Write-Host "[NEXIORA][TOOL][10%] Downloading $ToolId"
Invoke-WebRequest -UseBasicParsing -Uri $entry.Url -OutFile $download
if ((Get-Item $download).Length -lt 100000) { Remove-Item -Force $download; throw 'Downloaded artifact is unexpectedly small.' }
if ($entry.PackageType -eq 'zip') {
    New-Item -ItemType Directory -Force -Path $extract | Out-Null
    Write-Host "[NEXIORA][TOOL][55%] Extracting $ToolId"
    Expand-Archive -Force -Path $download -DestinationPath $extract
    $candidate = Get-ChildItem -Path $extract -Recurse -File -Filter $entry.InnerExecutable | Select-Object -First 1
    if ($null -eq $candidate) { throw "Executable not found in archive: $($entry.InnerExecutable)" }
    if (Test-Path $final) { Copy-Item -Force $final $backup }
    Copy-Item -Force $candidate.FullName $final
    Remove-Item -Recurse -Force $extract,$download
} else {
    if (Test-Path $final) { Copy-Item -Force $final $backup }
    Move-Item -Force $download $final
}
$hash = (Get-FileHash -Algorithm SHA256 $final).Hash.ToLowerInvariant()
$version = (& $final --version 2>$null | Select-Object -First 1)
if ([string]::IsNullOrWhiteSpace($version)) {
    if (Test-Path $backup) { Move-Item -Force $backup $final }
    throw 'Installed tool did not report a version.'
}
$registry = Join-Path $registryDir "$ToolId.registry"
$registryTemp = "$registry.tmp"
@("id=$ToolId","version=$version","executable=$final","sha256=$hash","source_url=$($entry.Url)") | Set-Content -Encoding ascii $registryTemp
Move-Item -Force $registryTemp $registry
Write-Host "[NEXIORA][TOOL][100%] Installed $ToolId"
Write-Host 'status=INSTALLED'
Write-Host "id=$ToolId"
Write-Host "version=$version"
Write-Host "executable=$final"
Write-Host "sha256=$hash"
