$ErrorActionPreference = "Stop"

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($dir -and $dir.Length -gt 3) {
        if (Test-Path (Join-Path $dir "CMakePresets.json")) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir) { break }
        $dir = $parent
    }
    throw "No se encontro CMakePresets.json; ejecuta desde D:\Nexiora o dentro del repo."
}

$repo = Find-RepoRoot
Set-Location $repo
Write-Host "== NCOS-011 repair3: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-011-repair3.ps1

Write-Host "== NCOS-011 repair3: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-011 repair3: build =="
cmake --build --preset release

Write-Host "== NCOS-011 repair3: focused tests =="
ctest --test-dir .\Build\windows-msvc-release -R NxAutoFixProposalEngineTests --output-on-failure

Write-Host "== NCOS-011 repair3: full tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-011 repair3: smoke proposal =="
$sampleDir = ".\Knowledge\NCOS\BuildAnalysisSamples"
if (!(Test-Path $sampleDir)) { New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null }
$sample = Join-Path $sampleDir "sample-proposal-input.md"
@"
# Build analysis

D:/Nexiora/Source/demo.c:10:5: error: expected ';' before 'return'
D:/Nexiora/Source/demo.c:11:1: warning: unused variable 'x'
The following tests FAILED:
  12 - DemoTests (Failed)
"@ | Set-Content -Path $sample -Encoding UTF8

$exe = ".\Build\windows-msvc-release\bin\nexiora_fix_proposal.exe"
if (!(Test-Path $exe)) { throw "No existe nexiora_fix_proposal.exe" }
$out = & $exe propose repair3 $sample
$out | Write-Host

$proposal = ".\Knowledge\NCOS\FixProposals\repair3.proposal.md"
if (!(Test-Path $proposal)) { throw "No se genero propuesta: $proposal" }
$content = Get-Content $proposal -Raw
if ($content -notmatch "Suggested actions") { throw "La propuesta no contiene acciones sugeridas." }
if ($content -notmatch "syntax_error") { throw "La propuesta no detecto error de sintaxis." }
if ($content -notmatch "test_failure") { throw "La propuesta no detecto falla de pruebas." }

Write-Host "== NCOS-011 repair3 complete =="
