$ErrorActionPreference = "Stop"
Write-Host "== NCOS-001 repair2: applying =="
& (Join-Path $PSScriptRoot "apply-ncos-001-repair2.ps1")

Write-Host "== NCOS-001 repair2: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-001 repair2: build =="
cmake --build --preset release

Write-Host "== NCOS-001 repair2: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-001 repair2: smoke =="
.\Build\windows-msvc-release\bin\nexiora_session.exe start videojuego "Crear un videojuego simple"
.\Build\windows-msvc-release\bin\nexiora_session.exe note "Definir loop de juego"
.\Build\windows-msvc-release\bin\nexiora_session.exe status
.\Build\windows-msvc-release\bin\nexiora_session.exe close

Write-Host "== NCOS-001 repair2 complete =="
