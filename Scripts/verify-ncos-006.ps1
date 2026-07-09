$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $root

Write-Host "== NCOS-006: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-006.ps1

Write-Host "== NCOS-006: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-006: build =="
cmake --build --preset release

Write-Host "== NCOS-006: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-006: smoke answer composer =="
$concept = ".\Build\windows-msvc-release\bin\nexiora_concept.exe"
$graph = ".\Build\windows-msvc-release\bin\nexiora_graph.exe"
$answer = ".\Build\windows-msvc-release\bin\nexiora_answer.exe"

if (!(Test-Path $concept)) { throw "No se encontro nexiora_concept.exe" }
if (!(Test-Path $graph)) { throw "No se encontro nexiora_graph.exe" }
if (!(Test-Path $answer)) { throw "No se encontro nexiora_answer.exe" }

& $concept upsert Genexus DataSelector "Un DataSelector define una consulta reutilizable sobre datos." "Sirve para evitar repetir filtros y reglas de seleccion." "DataProvider,Procedure" | Out-Null
& $concept upsert Genexus DataProvider "Un Data Provider construye y devuelve estructuras de datos." "Sirve para preparar datos consumibles por otros objetos." "KnowledgeBase" | Out-Null
& $concept upsert Genexus KnowledgeBase "La Knowledge Base es el repositorio central del conocimiento." "Sirve para generar aplicaciones desde el modelo." "Transaction,Procedure" | Out-Null
& $graph link Genexus DataSelector related_to DataProvider | Out-Null
& $graph link Genexus DataProvider belongs_to KnowledgeBase | Out-Null

$out = & $answer explain Genexus DataSelector KnowledgeBase
$out | Tee-Object -FilePath .\ncos-006-smoke.log

if ($out -notmatch "Respuesta estructurada") { throw "No se genero respuesta estructurada." }
if ($out -notmatch "DataSelector") { throw "No se uso el concepto origen." }
if ($out -notmatch "KnowledgeBase") { throw "No se uso el concepto destino." }
if ($out -notmatch "Evidencia usada") { throw "No se mostro evidencia." }
if ($out -notmatch "Confianza") { throw "No se mostro confianza." }

Write-Host "== NCOS-006 complete =="
