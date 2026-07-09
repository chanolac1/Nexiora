$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

Write-Host "== COGNITIVE-V2-0001 repair: applying =="

$src = Join-Path $root "Source\Cognitive\NxCognitiveCore.c"
if (!(Test-Path $src)) {
    throw "No existe Source\Cognitive\NxCognitiveCore.c. Descomprime el repair sobre D:\Nexiora."
}

# Force timestamp so Ninja rebuilds the cognitive object.
(Get-Item $src).LastWriteTime = Get-Date

# Ensure auto-research seed exists without copying a file onto itself.
$autoDir = Join-Path $root "Knowledge\Cognitive\AutoResearch\genexus"
New-Item -ItemType Directory -Force -Path $autoDir | Out-Null
$seed = Join-Path $autoDir "dataselector.txt"
if (!(Test-Path $seed)) {
@"
DataSelector en GeneXus
Un DataSelector en GeneXus es un objeto que permite definir una consulta reutilizable sobre datos.
A diferencia de un Data Provider, que construye y devuelve estructuras de datos, un DataSelector se enfoca en seleccionar registros o conjuntos de datos aplicando condiciones declarativas.
Un DataSelector ayuda a mantener consistencia porque evita repetir filtros y reglas de seleccion en distintos Procedures, Panels, Transactions u otros objetos.
"@ | Set-Content -Encoding UTF8 $seed
}

Write-Host "COGNITIVE-V2-0001 repair applied."
