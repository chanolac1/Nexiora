$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

Write-Host "== COGNITIVE-0004: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-cognitive-0004.ps1

Write-Host "== COGNITIVE-0004: configure =="
cmake --preset windows-msvc-release

Write-Host "== COGNITIVE-0004: build =="
cmake --build --preset release

Write-Host "== COGNITIVE-0004: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== COGNITIVE-0004: smoke ranking =="
$batch = ".\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe"
if (!(Test-Path $batch)) { throw "No se encontro nexiora_cognitive_batch.exe" }
& $batch ingest-dir Genexus .\Samples\Cognitive\Batch --recursive | Out-Host
$out = & $batch ask Genexus "Para que sirve una Knowledge Base?"
$out | Out-Host
$text = $out -join "`n"
if ($text -notmatch "Evidencia usada") { throw "La respuesta no muestra evidencia usada." }
if ($text -notmatch "relevancia") { throw "La respuesta no muestra ranking de relevancia." }
if ($text -notmatch "Fragmentos unicos usados") { throw "La respuesta no reporta fragmentos unicos." }
Write-Host "== COGNITIVE-0004 complete =="
