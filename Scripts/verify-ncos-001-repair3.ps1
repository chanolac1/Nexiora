$ErrorActionPreference = "Stop"

Write-Host "== NCOS-001 repair3: no self-copy apply =="
Write-Host "Este repair evita Copy-Item sobre el mismo archivo."

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $root

# Limpieza defensiva del estado de prueba que puede afectar NxSessionEngineTests.
$testRoot = Join-Path $root "Build\windows-msvc-release\Testing\Temporary\ncos_session_test"
if (Test-Path $testRoot) {
    Remove-Item $testRoot -Recurse -Force
}

# También limpia sesión activa de demo si existe, sin borrar sesiones reales del usuario.
# No eliminamos Knowledge\NCOS\Sessions completo para no perder historial.

Write-Host "== NCOS-001: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-001: build =="
cmake --build --preset release

Write-Host "== NCOS-001: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-001: smoke session =="
$exe = ".\Build\windows-msvc-release\bin\nexiora_session.exe"
if (!(Test-Path $exe)) {
    throw "No se encontro nexiora_session.exe"
}

& $exe start videojuego "Crear un videojuego simple"
& $exe note "Definir loop de juego"
& $exe status
& $exe close

Write-Host "== NCOS-001 repair3 complete =="
