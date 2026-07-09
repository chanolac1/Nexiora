$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
Write-Host "== COGNITIVE-0005 REPAIR: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-cognitive-0005.ps1
Write-Host "== COGNITIVE-0005 REPAIR: configure =="
cmake --preset windows-msvc-release
Write-Host "== COGNITIVE-0005 REPAIR: build =="
cmake --build --preset release
Write-Host "== COGNITIVE-0005 REPAIR: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
Write-Host "== COGNITIVE-0005 REPAIR: ask DataSelector =="
$out = .\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Que es un DataSelector?"
$out | Write-Host
if ($out -notmatch "DataSelector") { throw "La respuesta no menciona DataSelector." }
if ($out -notmatch "consulta reutilizable") { throw "No se incorporo la fuente local de auto-investigacion." }
if ($out -match "Transaction representa") { throw "La respuesta sigue usando evidencia lateral de Transaction." }
Write-Host "== COGNITIVE-0005 REPAIR complete =="
