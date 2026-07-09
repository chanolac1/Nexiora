$ErrorActionPreference = "Stop"

Write-Host "== NCOS-001 repair: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-001-repair.ps1

Write-Host "== NCOS-001 repair: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-001 repair: build =="
cmake --build --preset release

Write-Host "== NCOS-001 repair: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-001 repair: smoke session =="
$exe = ".\Build\windows-msvc-release\bin\nexiora_session.exe"
if (!(Test-Path $exe)) {
    throw "No se encontro nexiora_session.exe"
}

& $exe start videojuego "Crear un videojuego simple"
& $exe note "Definir loop de juego"
$status = & $exe status
$status | Out-Host
if ($status -notmatch "Sesion activa") {
    throw "La sesion no quedo activa correctamente."
}

$close = & $exe close
$close | Out-Host
if ($close -notmatch "Sesion cerrada") {
    throw "La sesion no cerro correctamente."
}

$sessionFile = ".\Knowledge\NCOS\Sessions\videojuego.jsonl"
if (!(Test-Path $sessionFile)) {
    throw "No se genero el archivo de sesion esperado: $sessionFile"
}

$content = Get-Content $sessionFile -Raw
if ($content -notmatch "Crear un videojuego simple") {
    throw "La sesion no guardo el objetivo."
}
if ($content -notmatch "Definir loop de juego") {
    throw "La sesion no guardo la nota."
}
if ($content -notmatch "closed") {
    throw "La sesion no registro cierre."
}

Write-Host "== NCOS-001 repair complete =="
