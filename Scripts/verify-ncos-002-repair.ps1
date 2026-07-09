$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Push-Location $root
try {
    Write-Host "== NCOS-002 repair: applying =="
    powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-002-repair.ps1

    Write-Host "== NCOS-002 repair: configure =="
    cmake --preset windows-msvc-release

    Write-Host "== NCOS-002 repair: build =="
    cmake --build --preset release

    Write-Host "== NCOS-002 repair: tests =="
    ctest --test-dir .\Build\windows-msvc-release --output-on-failure

    Write-Host "== NCOS-002 repair: smoke plan =="
    .\Build\windows-msvc-release\bin\nexiora_plan.exe create "Crear un videojuego simple"
    .\Build\windows-msvc-release\bin\nexiora_plan.exe note "Definir loop de juego"
    .\Build\windows-msvc-release\bin\nexiora_plan.exe done 1
    $status = .\Build\windows-msvc-release\bin\nexiora_plan.exe status | Out-String
    Write-Host $status
    if ($status -notmatch "videojuego") { throw "El estado del plan no conserva el objetivo." }
    if ($status -notmatch "Completados") { throw "El estado del plan no muestra progreso de pasos." }

    Write-Host "== NCOS-002 repair complete =="
}
finally { Pop-Location }
