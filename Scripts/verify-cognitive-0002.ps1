$ErrorActionPreference = "Stop"

Write-Host "== COGNITIVE-0002: applying =="
if (Test-Path .\Scripts\apply-cognitive-0002.ps1) {
    powershell -ExecutionPolicy Bypass -File .\Scripts\apply-cognitive-0002.ps1
}

Write-Host "== COGNITIVE-0002: configure =="
cmake --preset windows-msvc-release

Write-Host "== COGNITIVE-0002: build =="
cmake --build --preset release

Write-Host "== COGNITIVE-0002: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

$tool = ".\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe"
if (!(Test-Path $tool)) { throw "No se encontro nexiora_cognitive_batch.exe" }

Write-Host "== COGNITIVE-0002: ingest directory =="
& $tool ingest-dir Genexus .\Samples\Cognitive\Batch --recursive

Write-Host "== COGNITIVE-0002: ask =="
& $tool ask Genexus "Para que sirve una Knowledge Base?"

# El Cognitive Core guarda por diseño en Knowledge\Cognitive\Topics\<tema>.
$memory = ".\Knowledge\Cognitive\Topics\genexus\memory.jsonl"
$concepts = ".\Knowledge\Cognitive\Topics\genexus\concepts.txt"
$chunks = ".\Knowledge\Cognitive\Topics\genexus\chunks.txt"
if (!(Test-Path $memory)) { throw "No se genero memoria cognitiva: $memory" }
if (!(Test-Path $concepts)) { throw "No se genero conceptos cognitivos: $concepts" }
if (!(Test-Path $chunks)) { throw "No se genero chunks cognitivos: $chunks" }

Write-Host "== COGNITIVE-0002 complete =="
Write-Host "Memoria: $memory"
Write-Host "Conceptos: $concepts"
Write-Host "Fragmentos: $chunks"
