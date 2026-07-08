$ErrorActionPreference = "Stop"

Write-Host "== EPIC-0005: applying package =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-epic-0005.ps1

Write-Host "== EPIC-0005: configuring =="
cmake --preset windows-msvc-release

Write-Host "== EPIC-0005: building =="
cmake --build --preset release

Write-Host "== EPIC-0005: testing =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== EPIC-0005: first autonomous execution =="
.\Build\windows-msvc-release\bin\nexiora.exe research run

$session = ".\Research\Sessions\first_autonomous_execution"
Write-Host "== EPIC-0005: generated artifacts =="
Get-ChildItem $session | Select-Object Name,Length

$svg = Join-Path $session "graph.svg"
if (Test-Path $svg) {
    Write-Host "Opening graph: $svg"
    Start-Process $svg
}
