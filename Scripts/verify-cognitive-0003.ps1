$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

Write-Host "== COGNITIVE-0003: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-cognitive-0003.ps1

Write-Host "== COGNITIVE-0003: configure =="
cmake --preset windows-msvc-release

Write-Host "== COGNITIVE-0003: build =="
cmake --build --preset release

Write-Host "== COGNITIVE-0003: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== COGNITIVE-0003: smoke dedup =="
$batch = ".\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe"
if (!(Test-Path $batch)) {
    Write-Host "nexiora_cognitive_batch.exe no encontrado; omitiendo smoke batch."
    exit 0
}
& $batch ingest-dir Genexus .\Samples\Cognitive\Batch --recursive | Out-Host
$out = & $batch ask Genexus "Para que sirve una Knowledge Base?"
$out | Out-Host
if (($out -join "`n") -notmatch "Fragmentos unicos") {
    throw "La respuesta no reporta fragmentos unicos; deduplicacion no aplicada."
}
Write-Host "== COGNITIVE-0003 complete =="
