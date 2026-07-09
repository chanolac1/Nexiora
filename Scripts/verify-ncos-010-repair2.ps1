$ErrorActionPreference = 'Stop'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repo = Resolve-Path (Join-Path $scriptDir "..\..\..")
Push-Location $repo
try {
    Write-Host "== NCOS-010 repair2: applying =="
    powershell -ExecutionPolicy Bypass -File .\.nexiora_sprints\NCOS-010_BuildErrorAnalyzer\Scripts\apply-ncos-010-repair2.ps1
    Write-Host "== NCOS-010 repair2: configure =="
    cmake --preset windows-msvc-release
    Write-Host "== NCOS-010 repair2: build =="
    cmake --build --preset release
    Write-Host "== NCOS-010 repair2: focused test =="
    ctest --test-dir .\Build\windows-msvc-release -R NxBuildLogAnalyzerTests --output-on-failure
    Write-Host "== NCOS-010 repair2: smoke analyzer =="
    $sample = ".\Build\ncos010_repair2_sample.log"
    New-Item -ItemType Directory -Force -Path .\Build | Out-Null
    @("demo.c:1: warning: test warning", "demo.c:2: error: test error") | Set-Content -Encoding ASCII $sample
    $exe = ".\Build\windows-msvc-release\bin\nexiora_analyze_log.exe"
    if (!(Test-Path $exe)) { throw "No existe nexiora_analyze_log.exe" }
    $out = & $exe analyze repair2 $sample
    $out | Tee-Object -FilePath .\ncos-010-repair2-smoke.log
    if ($out -notmatch "Build Log Analyzer" -and $out -notmatch "Analyzer") { throw "No se ejecuto el analizador de logs." }
    if ($out -notmatch "Errores" -and $out -notmatch "error") { throw "No se detectaron errores en el log de prueba." }
    Write-Host "== NCOS-010 repair2 complete =="
}
finally { Pop-Location }
