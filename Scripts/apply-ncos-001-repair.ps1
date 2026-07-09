param(
    [string]$Root = (Get-Location).Path
)

$ErrorActionPreference = "Stop"

Write-Host "== NCOS-001 repair: cleaning session test state =="

$sessionRoot = Join-Path $Root "Knowledge\NCOS\Sessions"
if (Test-Path $sessionRoot) {
    Remove-Item -Path $sessionRoot -Recurse -Force
}

# Recreate expected runtime directory so the app can write fresh session state.
New-Item -ItemType Directory -Force -Path $sessionRoot | Out-Null

Write-Host "NCOS-001 repair applied: Knowledge\NCOS\Sessions reset."
