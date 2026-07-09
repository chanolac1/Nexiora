$ErrorActionPreference = "Stop"

Write-Host "== HIST-0002: configuring =="
cmake --preset windows-msvc-release

Write-Host "== HIST-0002: building =="
cmake --build --preset release

Write-Host "== HIST-0002: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== HIST-0002: live fixed-screen investigation smoke =="
$exe = ".\Build\windows-msvc-release\bin\nexiora.exe"
if (-not (Test-Path $exe)) {
    throw "No se encontro nexiora.exe"
}

# Ejecuta en modo no interactivo capturando la salida. Manualmente se vera en pantalla fija.
$log = & $exe investiga SQLite 2>&1 | Out-String
$log | Set-Content -Encoding UTF8 .\hist-0002-console-ui-smoke.log

if ($LASTEXITCODE -ne 0) {
    throw "nexiora.exe investiga SQLite fallo. Revisa hist-0002-console-ui-smoke.log"
}
if ($log -notmatch "Investigacion observable") {
    throw "No se encontro salida de investigacion observable. Revisa hist-0002-console-ui-smoke.log"
}
if ($log -notmatch "Artefactos") {
    throw "No se generaron artefactos segun la salida. Revisa hist-0002-console-ui-smoke.log"
}

Write-Host "== HIST-0002 complete =="
Write-Host "Para ver la pantalla fija en vivo ejecuta:"
Write-Host "  .\Build\windows-msvc-release\bin\nexiora.exe investiga SQLite"
