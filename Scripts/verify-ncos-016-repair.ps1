$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path -Parent $dir
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $dir) { throw 'No se encontro CMakePresets.json para ubicar la raiz del repo.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot -Start $PSScriptRoot
Set-Location $repo

Write-Host '== NCOS-016 repair: applying =='
powershell -ExecutionPolicy Bypass -File '.\.ncos_packages\NCOS-016_NexioraPackageManager_REPAIR\Scripts\apply-ncos-016-repair.ps1'

Write-Host '== NCOS-016 repair: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-016 repair: build =='
cmake --build --preset release

Write-Host '== NCOS-016 repair: focused test =='
ctest --test-dir '.\Build\windows-msvc-release' -R NxPackageManagerTests --output-on-failure

Write-Host '== NCOS-016 repair: full tests =='
ctest --test-dir '.\Build\windows-msvc-release' --output-on-failure

Write-Host '== NCOS-016 repair: smoke package install =='
$exe = '.\Build\windows-msvc-release\bin\nexiora_package.exe'
if (!(Test-Path $exe)) { throw 'No existe nexiora_package.exe' }
$out = & $exe install '.\.ncos_packages\NCOS-016_NexioraPackageManager'
$out
if ($LASTEXITCODE -ne 0) { throw 'nexiora_package install fallo.' }
if (($out -join "`n") -notmatch 'Nexiora Package Manager') { throw 'No se ejecuto el Package Manager.' }

Write-Host '== NCOS-016 repair complete =='
