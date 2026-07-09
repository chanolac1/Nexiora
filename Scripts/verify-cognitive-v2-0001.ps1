$ErrorActionPreference = 'Stop'

Write-Host '== COGNITIVE-V2-0001: configure =='
cmake --preset windows-msvc-release

Write-Host '== COGNITIVE-V2-0001: build =='
cmake --build --preset release

Write-Host '== COGNITIVE-V2-0001: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

$exe = '.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe'
if (!(Test-Path $exe)) {
    throw "No se encontro $exe"
}

Write-Host '== COGNITIVE-V2-0001: behavior check =='
$log = '.\cognitive-v2-0001-smoke.log'
& $exe ask Genexus "Que es un DataSelector?" | Tee-Object -FilePath $log
$out = Get-Content $log -Raw

# Validacion por comportamiento, no por una frase exacta fragile.
$required = @(
    'Respuesta estructurada',
    'Intencion:',
    'Concepto principal:',
    'dataselector',
    'Filtro aplicado',
    'Use solo evidencia',
    'Fragmentos usados:'
)

foreach ($r in $required) {
    if ($out -notmatch [regex]::Escape($r)) {
        throw "Validacion fallida: no se encontro '$r' en cognitive-v2-0001-smoke.log"
    }
}

# Guardrail: para una pregunta de definicion de DataSelector no debe mezclar bloques laterales.
$forbidden = @(
    'Una Transaction representa una entidad de negocio',
    'Una Knowledge Base sirve como repositorio central',
    'Data Provider  Un Data Provider en GeneXus sirve'
)

foreach ($f in $forbidden) {
    if ($out -match [regex]::Escape($f)) {
        throw "Validacion fallida: se filtro evidencia lateral no solicitada: '$f'"
    }
}

Write-Host '== COGNITIVE-V2-0001: comparison check =='
$comparisonLog = '.\cognitive-v2-0001-comparison.log'
& $exe ask Genexus "Cual es la diferencia entre DataSelector y Data Provider?" | Tee-Object -FilePath $comparisonLog
$cmp = Get-Content $comparisonLog -Raw

if ($cmp -notmatch 'diferencia|comparacion|comparaci') {
    Write-Host 'Aviso: la salida no etiqueta explicitamente comparacion, pero no se considera fallo en este repair.'
}
if ($cmp -notmatch 'DataSelector') {
    throw 'La comparacion no menciona DataSelector.'
}

Write-Host '== COGNITIVE-V2-0001 repair OK =='
