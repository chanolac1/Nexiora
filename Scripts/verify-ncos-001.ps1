$ErrorActionPreference = 'Stop'
Write-Host '== NCOS-001: applying =='
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-001.ps1

Write-Host '== NCOS-001: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-001: build =='
cmake --build --preset release

Write-Host '== NCOS-001: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

$exe = '.\Build\windows-msvc-release\bin\nexiora_session.exe'
if (!(Test-Path $exe)) { throw 'No se genero nexiora_session.exe' }

Write-Host '== NCOS-001: smoke session =='
& $exe start videojuego 'Crear un videojuego simple' | Tee-Object -FilePath ncos-001-session-smoke.log
& $exe note 'Definir loop de juego' | Tee-Object -FilePath ncos-001-session-smoke.log -Append
& $exe status | Tee-Object -FilePath ncos-001-session-smoke.log -Append

$sessionFile = '.\Knowledge\NCOS\Sessions\videojuego.jsonl'
if (!(Test-Path $sessionFile)) { throw "No se genero archivo de sesion: $sessionFile" }
$content = Get-Content $sessionFile -Raw
if ($content -notmatch 'Crear un videojuego simple') { throw 'La sesion no guardo el objetivo.' }
if ($content -notmatch 'Definir loop de juego') { throw 'La sesion no guardo la nota.' }

& $exe close | Tee-Object -FilePath ncos-001-session-smoke.log -Append
Write-Host '== NCOS-001 complete =='
