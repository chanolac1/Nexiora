$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $root

Write-Host "== NCOS-005: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-005.ps1

Write-Host "== NCOS-005: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-005: build =="
cmake --build --preset release

Write-Host "== NCOS-005: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-005: smoke graph reasoning =="
.\Build\windows-msvc-release\bin\nexiora_graph.exe link Genexus DataSelector related_to DataProvider | Out-Host
.\Build\windows-msvc-release\bin\nexiora_graph.exe link Genexus DataProvider belongs_to KnowledgeBase | Out-Host
$out = .\Build\windows-msvc-release\bin\nexiora_reason.exe why Genexus DataSelector KnowledgeBase
$out | Out-Host

if ($out -notmatch "Razonamiento sobre grafo") { throw "No se ejecuto el razonamiento sobre grafo." }
if ($out -notmatch "dataselector") { throw "La evidencia no menciona dataselector." }
if ($out -notmatch "dataprovider") { throw "La evidencia no incluye concepto intermedio dataprovider." }
if ($out -notmatch "knowledgebase") { throw "La evidencia no llega a knowledgebase." }
if ($out -notmatch "Confianza") { throw "No se reporto confianza." }

Write-Host "== NCOS-005 complete =="
