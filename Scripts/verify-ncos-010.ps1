$ErrorActionPreference = 'Stop'
function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $p = Split-Path $dir -Parent
        if ($p -eq $dir) { throw 'No se encontro raiz del repo.' }
        $dir = $p
    }
}
$repo = Find-RepoRoot $PSScriptRoot
Set-Location $repo

Write-Host '== NCOS-010: applying =='
powershell -ExecutionPolicy Bypass -File .\.nexiora_sprints\NCOS-010_BuildErrorAnalyzer\Scripts\apply-ncos-010.ps1

Write-Host '== NCOS-010: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-010: build =='
cmake --build --preset release

Write-Host '== NCOS-010: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-010: smoke analyzer =='
$log = '.\Knowledge\NCOS\Compiler\tests.log'
if (!(Test-Path $log)) {
    New-Item -ItemType Directory -Force -Path '.\Knowledge\NCOS\Compiler' | Out-Null
    Set-Content -Path $log -Value "demo.c:10: error: simulated compiler error`ndemo.c:11: warning: simulated warning" -Encoding UTF8
}
$exe = '.\Build\windows-msvc-release\bin\nexiora_analyze_log.exe'
if (!(Test-Path $exe)) { throw "No se genero $exe" }
$out = & $exe analyze tests $log | Tee-Object -FilePath .\ncos-010-smoke.log
if ($out -notmatch 'Build Log Analyzer') { throw 'No se ejecuto el analizador de logs.' }
if ($out -notmatch 'Hallazgos') { throw 'No se reportaron hallazgos.' }
if (!(Test-Path '.\Knowledge\NCOS\BuildAnalysis\tests.analysis.md')) { throw 'No se genero el reporte de analisis.' }
Write-Host '== NCOS-010 complete =='
