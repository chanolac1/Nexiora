$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = Resolve-Path $Start
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $dir) {
            throw "No se encontro CMakePresets.json desde $Start"
        }
        $dir = $parent
    }
}

$root = Find-RepoRoot $PSScriptRoot
Set-Location $root

Write-Host '== NCOS-010 repair3: applying =='
& powershell -ExecutionPolicy Bypass -File (Join-Path $root 'Scripts\apply-ncos-010-repair3.ps1')

Write-Host '== NCOS-010 repair3: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-010 repair3: build =='
cmake --build --preset release

Write-Host '== NCOS-010 repair3: focused test =='
ctest --test-dir (Join-Path $root 'Build\windows-msvc-release') -R NxBuildLogAnalyzerTests --output-on-failure

Write-Host '== NCOS-010 repair3: full tests =='
ctest --test-dir (Join-Path $root 'Build\windows-msvc-release') --output-on-failure

Write-Host '== NCOS-010 repair3: smoke analyzer =='
$exe = Join-Path $root 'Build\windows-msvc-release\bin\nexiora_analyze_log.exe'
if (!(Test-Path $exe)) { throw "No existe nexiora_analyze_log.exe en $exe" }

$sampleDir = Join-Path $root 'Knowledge\NCOS\BuildAnalysisSamples'
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null
$sample = Join-Path $sampleDir 'sample-build.log'
@'
FAILED: CMakeFiles/demo.obj
D:/Nexiora/Source/demo.c:10:5: error: expected ';' before 'return'
D:/Nexiora/Source/demo.c:11:1: warning: unused variable 'x'
The following tests FAILED:
  12 - DemoTests (Failed)
'@ | Set-Content -Path $sample -Encoding UTF8

$out = & $exe analyze repair3 $sample
$out | Tee-Object -FilePath (Join-Path $root 'ncos-010-repair3-smoke.log')

if ($out -notmatch 'Build Log Analyzer' -and $out -notmatch 'Analizador') { throw 'No se ejecuto el analizador de logs.' }
if ($out -notmatch 'Errores' -and $out -notmatch 'errors') { throw 'No se reportaron errores.' }

Write-Host '== NCOS-010 repair3 complete =='
