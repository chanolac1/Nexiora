$ErrorActionPreference = "Stop"

function Find-RepoRoot([string]$start) {
    $dir = (Resolve-Path $start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir "CMakePresets.json")) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) {
            throw "No se encontro la raiz del repositorio Nexiora desde $start"
        }
        $dir = $parent
    }
}

$root = Find-RepoRoot $PSScriptRoot
Set-Location $root

Write-Host "== NCOS-008: applying =="
powershell -ExecutionPolicy Bypass -File .\.nexiora_sprints\NCOS-008_TaskExecutionEngine\Scripts\apply-ncos-008.ps1

Write-Host "== NCOS-008: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-008: build =="
cmake --build --preset release

Write-Host "== NCOS-008: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-008: smoke task execution =="
$exe = ".\Build\windows-msvc-release\bin\nexiora_task.exe"
if (!(Test-Path $exe)) { throw "No se encontro nexiora_task.exe en $exe" }

# Limpieza del caso de smoke para que sea repetible.
$taskFile = ".\Knowledge\NCOS\Tasks\videojuego_demo.tasks"
$logFile = ".\Knowledge\NCOS\Tasks\videojuego_demo.action.log"
Remove-Item $taskFile -Force -ErrorAction SilentlyContinue
Remove-Item $logFile -Force -ErrorAction SilentlyContinue

& $exe create videojuego_demo "Crear un videojuego simple" | Tee-Object -FilePath ncos-008-create.log
if ($LASTEXITCODE -ne 0) { throw "nexiora_task create fallo." }
& $exe run-next videojuego_demo | Tee-Object -FilePath ncos-008-run-next.log
if ($LASTEXITCODE -ne 0) { throw "nexiora_task run-next fallo." }
& $exe run-all videojuego_demo | Tee-Object -FilePath ncos-008-run-all.log
if ($LASTEXITCODE -ne 0) { throw "nexiora_task run-all fallo." }
$status = & $exe status videojuego_demo
if ($LASTEXITCODE -ne 0) { throw "nexiora_task status fallo." }
$status | Tee-Object -FilePath ncos-008-status.log

if ($status -notmatch "Completadas") { throw "No se mostro resumen de tareas completadas." }
if ($status -notmatch "Total") { throw "No se mostro total de tareas." }
if (!(Test-Path $taskFile)) { throw "No se genero archivo de tareas: $taskFile" }
if (!(Test-Path $logFile)) { throw "No se genero action log: $logFile" }

Write-Host "== NCOS-008 complete =="
