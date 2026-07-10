$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw 'No se pudo encontrar la raiz del repo con CMakePresets.json.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
Set-Location $repo

Write-Host '== NCOS-015 repair: applying =='
powershell -ExecutionPolicy Bypass -File (Join-Path $repo 'Scripts/apply-ncos-015.ps1')

Write-Host '== NCOS-015 repair: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-015 repair: build =='
cmake --build --preset release

Write-Host '== NCOS-015 repair: focused test =='
ctest --test-dir .\Build\windows-msvc-release -R NxPatchVerificationGateTests --output-on-failure

Write-Host '== NCOS-015 repair: full tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-015 repair: smoke verify =='
$exe = Join-Path $repo 'Build/windows-msvc-release/bin/nexiora_patch_verify.exe'
if (!(Test-Path $exe)) { throw 'No existe nexiora_patch_verify.exe' }
$out = & $exe verify verify_demo | Out-String
Write-Host $out
if ($out -notmatch 'PROMOTABLE|BLOCKED|Verification|verificacion|Verificacion|Gate|Estado') {
    throw 'No se pudo validar comportamiento del Patch Verification Gate.'
}

Write-Host '== NCOS-015 repair complete =='
