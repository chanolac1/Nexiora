$ErrorActionPreference = "Stop"

Write-Host "== NCOS-004: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-004.ps1

Write-Host "== NCOS-004: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-004: build =="
cmake --build --preset release

Write-Host "== NCOS-004: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure -R NxConceptGraphTests

Write-Host "== NCOS-004: smoke concept graph =="
$exe = ".\Build\windows-msvc-release\bin\nexiora_graph.exe"
if (!(Test-Path $exe)) { throw "No se encontro nexiora_graph.exe" }

& $exe link Genexus DataSelector related_to DataProvider | Tee-Object -FilePath ncos-004-link.log
& $exe link Genexus DataSelector used_by Procedure | Tee-Object -FilePath ncos-004-link2.log
$out = & $exe show Genexus DataSelector
$out | Tee-Object -FilePath ncos-004-show.log

if ($out -notmatch "dataselector") { throw "El grafo no mostro el concepto dataselector." }
if ($out -notmatch "dataprovider") { throw "El grafo no mostro la relacion hacia dataprovider." }
if ($out -notmatch "procedure") { throw "El grafo no mostro la relacion hacia procedure." }

Write-Host "== NCOS-004 complete =="
