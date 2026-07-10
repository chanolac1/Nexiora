$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw 'No se encontro CMakePresets.json.' }
        $dir = $parent
    }
}

$root = Find-RepoRoot
Set-Location $root

Write-Host '== NCOS-015 warning repair: applying =='
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$apply = Join-Path $scriptDir 'apply-ncos-015-warning-repair.ps1'
powershell -ExecutionPolicy Bypass -File $apply

Write-Host '== NCOS-015 warning repair: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-015 warning repair: build =='
$buildOutput = & cmake --build --preset release 2>&1 | Tee-Object -Variable buildLog
$buildText = $buildLog -join "`n"
if ($buildText -match 'NxPatchVerificationGate\.c:.*warning:') {
    throw 'Persisten warnings en NxPatchVerificationGate.c.'
}

Write-Host '== NCOS-015 warning repair: focused test =='
ctest --test-dir .\Build\windows-msvc-release -R NxPatchVerificationGateTests --output-on-failure

Write-Host '== NCOS-015 warning repair: full tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-015 warning repair: smoke verify =='
$out = & .\Build\windows-msvc-release\bin\nexiora_patch_verify.exe verify verify_demo 2>&1
$out
if (($out -join "`n") -notmatch 'Patch Verification Gate') { throw 'No se ejecuto Patch Verification Gate.' }

Write-Host '== NCOS-015 warning repair complete =='
