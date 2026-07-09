$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$pkg = Join-Path $root ".nexiora_sprints\COGNITIVE-0003"

if (!(Test-Path $pkg)) {
    throw "No se encontro el paquete .nexiora_sprints\COGNITIVE-0003. Descomprime el ZIP en la raiz de Nexiora."
}

$src = Join-Path $pkg "Source\Cognitive\NxCognitiveCore.c"
$dst = Join-Path $root "Source\Cognitive\NxCognitiveCore.c"
if (!(Test-Path $src)) { throw "No se encontro $src" }
Copy-Item -Path $src -Destination $dst -Force

Write-Host "COGNITIVE-0003 applied: deduplicated cognitive answers."
