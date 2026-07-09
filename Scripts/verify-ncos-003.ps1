$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $root

Write-Host "== NCOS-003 REPAIR: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-003 REPAIR: build =="
cmake --build --preset release

Write-Host "== NCOS-003 REPAIR: tests =="
ctest --test-dir .\Build\windows-msvc-release -R NxConceptRegistryTests --output-on-failure

$tool = ".\Build\windows-msvc-release\bin\nexiora_concept.exe"
if (!(Test-Path $tool)) { throw "No se encontro nexiora_concept.exe" }

Write-Host "== NCOS-003 REPAIR: smoke concept registry =="
& $tool upsert Genexus DataSelector "Un DataSelector define una consulta reutilizable sobre datos." "Sirve para evitar repetir filtros y reglas de seleccion." "Data Provider,Transaction,Procedure"
$outText = (& $tool show Genexus DataSelector) -join "`n"
$outText | Tee-Object -FilePath .\ncos-003-concept-smoke.log

if ($outText -notmatch "Concepto:\s*DataSelector") { throw "No se mostro el concepto DataSelector." }
if ($outText -notmatch "Definicion") { throw "La salida no tiene seccion Definicion." }
if ($outText -notmatch "Proposito") { throw "La salida no tiene seccion Proposito." }
if ($outText -notmatch "Relaciones") { throw "La salida no tiene seccion Relaciones." }
if ($outText -notmatch "Data Provider") { throw "La salida no contiene relaciones esperadas." }

$card = ".\Knowledge\NCOS\Concepts\genexus\dataselector.card"
if (!(Test-Path $card)) { throw "No se genero la tarjeta de concepto: $card" }

Write-Host "== NCOS-003 REPAIR complete =="
