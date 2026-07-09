$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$pkg = Join-Path $root ".nexiora_sprints\COGNITIVE-0005"
if (!(Test-Path $pkg)) { throw "No se encontro .nexiora_sprints\COGNITIVE-0005. Descomprime el ZIP en la raiz de Nexiora." }

$srcCore = Join-Path $pkg "Source\Cognitive\NxCognitiveCore.c"
$dstCore = Join-Path $root "Source\Cognitive\NxCognitiveCore.c"
if (!(Test-Path $srcCore)) { throw "No se encontro $srcCore" }
if ((Resolve-Path $srcCore).Path -ne (Resolve-Path $dstCore -ErrorAction SilentlyContinue).Path) {
    Copy-Item -Path $srcCore -Destination $dstCore -Force
}

$autoDst = Join-Path $root "Knowledge\Cognitive\AutoResearch\genexus"
New-Item -ItemType Directory -Force -Path $autoDst | Out-Null
$srcData = Join-Path $pkg "Knowledge\Cognitive\AutoResearch\genexus\dataselector.txt"
$dstData = Join-Path $autoDst "dataselector.txt"
if (!(Test-Path $srcData)) {
    # Fallback: if the first package already placed the file in the destination, keep it.
    if (!(Test-Path $dstData)) { throw "No se encontro fuente de auto-investigacion para DataSelector." }
} else {
    $srcResolved = (Resolve-Path $srcData).Path
    $dstResolved = if (Test-Path $dstData) { (Resolve-Path $dstData).Path } else { "" }
    if ($srcResolved -ne $dstResolved) {
        Copy-Item -Path $srcData -Destination $dstData -Force
    }
}
Write-Host "COGNITIVE-0005 repaired: apply idempotente y auto-investigacion DataSelector disponible."
