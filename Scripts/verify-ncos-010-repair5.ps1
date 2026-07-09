$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $dir) {
            throw 'No se encontro CMakePresets.json para determinar la raiz del repositorio.'
        }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
Set-Location $repo
Write-Host "Repo root: $repo"

Write-Host '== NCOS-010 repair5: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-010 repair5: build =='
cmake --build --preset release

Write-Host '== NCOS-010 repair5: focused test =='
ctest --test-dir .\Build\windows-msvc-release -R NxBuildLogAnalyzerTests --output-on-failure

Write-Host '== NCOS-010 repair5: full tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-010 repair5: smoke analyzer =='
$exe = Join-Path $repo 'Build\windows-msvc-release\bin\nexiora_analyze_log.exe'
if (!(Test-Path $exe)) { throw 'No existe nexiora_analyze_log.exe' }

$sampleDir = Join-Path $repo 'Knowledge\NCOS\BuildAnalysisSamples'
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null
$sample = Join-Path $sampleDir 'sample-build-repair5.log'
@'
FAILED: CMakeFiles/demo.obj
D:/Nexiora/Source/demo.c:10:5: error: expected ';' before 'return'
D:/Nexiora/Source/demo.c:11:1: warning: unused variable 'x'
The following tests FAILED:
  12 - DemoTests (Failed)
'@ | Set-Content -Path $sample -Encoding UTF8

$out = & $exe analyze repair5 $sample
$out | Tee-Object -FilePath (Join-Path $repo 'ncos-010-repair5-smoke.log')

$report = Join-Path $repo 'Knowledge\NCOS\BuildAnalysis\repair5.analysis.md'
if (!(Test-Path $report)) { throw "No se genero el reporte esperado: $report" }

$reportText = Get-Content $report -Raw
$smokeText = ($out | Out-String)

# Validacion robusta por comportamiento, no por formato exacto de consola.
if ($smokeText -notmatch 'Build Log Analyzer') { throw 'No se ejecuto el Build Log Analyzer.' }
if ($smokeText -notmatch 'FAILED') { throw 'El analizador no detecto estado FAILED.' }
if ($smokeText -notmatch 'Errores\s*:') { throw 'El analizador no reporto contador de errores.' }
if ($smokeText -notmatch 'Warnings\s*:') { throw 'El analizador no reporto contador de warnings.' }
if ($smokeText -notmatch 'Hallazgos principales') { throw 'El analizador no mostro hallazgos principales.' }

# El Run ID se valida por el nombre del artefacto generado y por contenido no exacto.
if ((Split-Path $report -Leaf) -ne 'repair5.analysis.md') { throw 'El reporte generado no corresponde al Run ID repair5.' }
if ($reportText -notmatch 'repair5|demo\.c|expected|FAILED|warning') {
    throw 'El reporte no contiene evidencia suficiente del analisis repair5.'
}

Write-Host '== NCOS-010 repair5 complete =='
Write-Host 'Analizador validado por comportamiento real y reporte generado.'
