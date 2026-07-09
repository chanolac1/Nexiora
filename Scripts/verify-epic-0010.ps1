$ErrorActionPreference = "Stop"

powershell -ExecutionPolicy Bypass -File .\Scripts\apply-epic-0010.ps1

Write-Host "== EPIC-0010: configuring =="
cmake --preset windows-msvc-release

Write-Host "== EPIC-0010: building =="
cmake --build --preset release

Write-Host "== EPIC-0010: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

$demo = ".\Build\windows-msvc-release\bin\nexiora_progress_demo.exe"
if (!(Test-Path $demo)) {
    throw "No se encontro nexiora_progress_demo.exe"
}

Write-Host "== EPIC-0010: progress smoke test =="
& $demo SQLite | Tee-Object -FilePath .\epic-0010-progress-smoke.log

$log = Get-Content .\epic-0010-progress-smoke.log -Raw
if ($log -notmatch "Progreso de investigacion") {
    throw "El demo de progreso no imprimio el encabezado esperado."
}
if ($log -notmatch "100%") {
    throw "El demo de progreso no llego a 100%."
}
if ($log -notmatch "Investigacion completada") {
    throw "El demo de progreso no reporto finalizacion."
}

Write-Host "== EPIC-0010 complete =="
