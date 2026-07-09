$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { break }
        $dir = $parent
    }
    throw 'No se encontro CMakePresets.json; ejecuta desde la raiz del repositorio Nexiora.'
}

$root = Find-RepoRoot
Set-Location $root
Write-Host "Repo root: $root"

Write-Host '== NCOS-010 repair4: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-010 repair4: build =='
cmake --build --preset release

Write-Host '== NCOS-010 repair4: focused test =='
ctest --test-dir .\Build\windows-msvc-release -R NxBuildLogAnalyzerTests --output-on-failure

Write-Host '== NCOS-010 repair4: full tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-010 repair4: smoke analyzer =='
$exe = Join-Path $root 'Build\windows-msvc-release\bin\nexiora_analyze_log.exe'
if (!(Test-Path $exe)) { throw 'No existe nexiora_analyze_log.exe' }

$sampleDir = Join-Path $root 'Knowledge\NCOS\BuildAnalysisSamples'
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null
$sampleLog = Join-Path $sampleDir 'sample-build.log'
@'
FAILED: CMakeFiles/demo.obj
D:/Nexiora/Source/demo.c:10:5: error: expected ';' before 'return'
D:/Nexiora/Source/demo.c:11:1: warning: unused variable 'x'
The following tests FAILED:
  12 - DemoTests (Failed)
'@ | Set-Content -Encoding UTF8 $sampleLog

$out = & $exe analyze repair4 $sampleLog
$out | Tee-Object -FilePath 'ncos-010-repair4-smoke.log'

$report = Join-Path $root 'Knowledge\NCOS\BuildAnalysis\repair4.analysis.md'
if (!(Test-Path $report)) { throw 'No se genero el reporte de analisis.' }

# Behavior checks, not fragile title checks.
if ($out -notmatch 'Run ID\s*: repair4') { throw 'El analizador no proceso el Run ID esperado.' }
if ($out -notmatch 'Errores\s*:') { throw 'El analizador no reporto conteo de errores.' }
if ($out -notmatch 'Warnings\s*:') { throw 'El analizador no reporto conteo de warnings.' }
if ($out -notmatch 'Reporte\s*:') { throw 'El analizador no informo la ruta del reporte.' }
if ($out -notmatch 'Hallazgos principales') { throw 'El analizador no emitio hallazgos principales.' }

$reportContent = Get-Content $report -Raw
if ($reportContent -notmatch 'expected') { throw 'El reporte no contiene el error esperado del log.' }
if ($reportContent -notmatch 'unused variable') { throw 'El reporte no contiene el warning esperado del log.' }

Write-Host '== NCOS-010 repair4 complete =='
Write-Host 'Build Log Analyzer verificado por comportamiento real.'
