$ErrorActionPreference = "Stop"

Write-Host "== HIST-0001: configuring =="
cmake --preset windows-msvc-release

Write-Host "== HIST-0001: building =="
cmake --build --preset release

Write-Host "== HIST-0001: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== HIST-0001: investigate SQLite =="
$exe = ".\Build\windows-msvc-release\bin\nexiora.exe"
if (-not (Test-Path $exe)) {
    throw "No se encontro nexiora.exe"
}

& $exe investiga SQLite | Tee-Object -FilePath hist-0001-sqlite.log

$log = Get-Content .\hist-0001-sqlite.log -Raw
if ($log -notmatch "Investigacion observable") {
    throw "No se detecto la salida de investigacion observable."
}
if ($log -notmatch "Conceptos detectados") {
    throw "No se detectaron conceptos extraidos."
}
if ($log -notmatch "Confianza") {
    throw "No se detecto el resumen de confianza."
}

if (-not (Test-Path ".\Knowledge\Investigations\sqlite\report.md")) {
    throw "No se genero report.md para SQLite."
}
if (-not (Test-Path ".\Knowledge\Investigations\sqlite\memory.jsonl")) {
    throw "No se genero memory.jsonl para SQLite."
}

Write-Host "== HIST-0001 complete =="
