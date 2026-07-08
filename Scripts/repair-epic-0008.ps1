$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

Write-Host "== EPIC-0008 repair: normalizing timestamps =="
$Stamp = (Get-Date).AddMinutes(-5)

$PathsToNormalize = @(
    "CMakeLists.txt",
    "CMakePresets.json",
    "Include",
    "Source",
    "Tests",
    "Scripts"
)

foreach ($Path in $PathsToNormalize) {
    if (Test-Path $Path) {
        Get-ChildItem $Path -Recurse -Force -File -ErrorAction SilentlyContinue | ForEach-Object {
            $_.LastWriteTime = $Stamp
        }
        if ((Get-Item $Path) -is [System.IO.FileInfo]) {
            (Get-Item $Path).LastWriteTime = $Stamp
        }
    }
}

if (Test-Path "CMakeLists.txt") {
    (Get-Item "CMakeLists.txt").LastWriteTime = $Stamp
}
if (Test-Path "CMakePresets.json") {
    (Get-Item "CMakePresets.json").LastWriteTime = $Stamp
}

Write-Host "== EPIC-0008 repair: removing stale release build directory =="
if (Test-Path ".\Build\windows-msvc-release") {
    Remove-Item ".\Build\windows-msvc-release" -Recurse -Force
}

Write-Host "== EPIC-0008 repair complete =="
