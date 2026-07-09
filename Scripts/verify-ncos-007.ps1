$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $root

Write-Host "== NCOS-007: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-007.ps1

Write-Host "== NCOS-007: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-007: build =="
cmake --build --preset release

Write-Host "== NCOS-007: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-007: smoke intent planning =="
$tool = ".\Build\windows-msvc-release\bin\nexiora_intent_plan.exe"
if (!(Test-Path $tool)) { throw "No se encontro nexiora_intent_plan.exe" }
$out = & $tool "crear un videojuego simple"
$out | Tee-Object -FilePath .\ncos-007-smoke.log

if ($out -notmatch "Plan creado desde intencion") { throw "No se creo plan desde intencion." }
if ($out -notmatch "build_project") { throw "No se detecto intencion build_project." }
if ($out -notmatch "Construir proyecto") { throw "No se normalizo el objetivo." }
if ($out -notmatch "Pasos") { throw "No se generaron pasos del plan." }

Write-Host "== NCOS-007 complete =="
