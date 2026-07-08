$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

Write-Host "== EPIC-0008: repair / prepare =="
powershell -ExecutionPolicy Bypass -File .\Scripts\repair-epic-0008.ps1

Write-Host "== EPIC-0008: configuring =="
cmake --preset windows-msvc-release

Write-Host "== EPIC-0008: building =="
cmake --build --preset release

Write-Host "== EPIC-0008: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== EPIC-0008: seed persistent memory =="
.\Build\windows-msvc-release\bin\nexiora.exe memory seed

Write-Host "== EPIC-0008: memory summary =="
.\Build\windows-msvc-release\bin\nexiora.exe memory summary

Write-Host "== EPIC-0008 complete =="
