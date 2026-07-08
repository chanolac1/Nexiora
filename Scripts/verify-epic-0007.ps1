$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

Write-Host "== EPIC-0007 REPAIR2: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-epic-0007.ps1

Write-Host "== EPIC-0007 REPAIR2: clean configure =="
if (Test-Path .\Build\windows-msvc-release) {
    Remove-Item .\Build\windows-msvc-release -Recurse -Force
}
cmake --preset windows-msvc-release

Write-Host "== EPIC-0007 REPAIR2: building =="
cmake --build --preset release

Write-Host "== EPIC-0007 REPAIR2: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== EPIC-0007 REPAIR2: smoke conversation =="
$conversationInput = "ayuda`r`nestado`r`nque aprendiste`r`nrecomendaciones`r`nsorprendeme`r`nsalir`r`n"
$output = $conversationInput | .\Build\windows-msvc-release\bin\nexiora.exe
$output | Tee-Object -FilePath .\epic-0007-conversation-smoke.log

if ($output -notmatch "N E X I O R A") {
    throw "La consola conversacional no imprimio el encabezado NEXIORA. Revisa epic-0007-conversation-smoke.log"
}
if ($output -notmatch "Laboratorio Autonomo de Investigacion" -and $output -notmatch "Laboratorio Autónomo de Investigación") {
    throw "La consola conversacional no se inicio correctamente. Revisa epic-0007-conversation-smoke.log"
}
if ($output -notmatch "Comandos disponibles") {
    throw "La respuesta de ayuda no fue detectada. Revisa epic-0007-conversation-smoke.log"
}
if ($output -notmatch "Lo que he aprendido" -and $output -notmatch "aprendido") {
    throw "La respuesta 'que aprendiste' no fue detectada. Revisa epic-0007-conversation-smoke.log"
}

Write-Host "== EPIC-0007 REPAIR2 complete =="
Write-Host "Prueba manual: .\Build\windows-msvc-release\bin\nexiora.exe"
