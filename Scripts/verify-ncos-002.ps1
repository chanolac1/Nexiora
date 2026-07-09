$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $root

Write-Host "== NCOS-002: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-002.ps1

Write-Host "== NCOS-002: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-002: build =="
cmake --build --preset release

Write-Host "== NCOS-002: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

$exe = ".\Build\windows-msvc-release\bin\nexiora_plan.exe"
if (!(Test-Path $exe)) { throw "No se encontro nexiora_plan.exe" }

Write-Host "== NCOS-002: smoke plan =="
& $exe create "Crear un videojuego simple" | Tee-Object -FilePath ncos-002-plan-smoke.log
& $exe note "Definir loop de juego" | Tee-Object -FilePath ncos-002-plan-smoke.log -Append
& $exe done 1 | Tee-Object -FilePath ncos-002-plan-smoke.log -Append
& $exe status | Tee-Object -FilePath ncos-002-plan-smoke.log -Append

$log = Get-Content .\ncos-002-plan-smoke.log -Raw
if ($log -notmatch "Plan creado") { throw "No se creo el plan." }
if ($log -notmatch "Paso registrado") { throw "No se registro el paso completado." }
if ($log -notmatch "Progreso") { throw "No se mostro progreso del plan." }

$planFile = ".\Knowledge\NCOS\Plans\crear_un_videojuego_simple.jsonl"
if (!(Test-Path $planFile)) { throw "No se genero archivo de plan: $planFile" }

Write-Host "== NCOS-002 complete =="
