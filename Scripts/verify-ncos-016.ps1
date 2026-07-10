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

Write-Host '== NCOS-016: applying =='
$apply = Join-Path $repo '.ncos_packages\NCOS-016_NexioraPackageManager\Scripts\apply-ncos-016.ps1'
if (!(Test-Path $apply)) { throw "No existe aplicador: $apply" }
powershell -ExecutionPolicy Bypass -File $apply

Write-Host '== NCOS-016: configure =='
cmake --preset windows-msvc-release

Write-Host '== NCOS-016: build =='
cmake --build --preset release

Write-Host '== NCOS-016: focused test =='
ctest --test-dir .\Build\windows-msvc-release -R NxPackageManagerTests --output-on-failure

Write-Host '== NCOS-016: full tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== NCOS-016: smoke package install =='
$exe = Join-Path $repo 'Build\windows-msvc-release\bin\nexiora_package.exe'
if (!(Test-Path $exe)) { throw 'No existe nexiora_package.exe' }
$pkg = Join-Path $repo '.ncos_packages\NCOS-016_NexioraPackageManager'
$out = & $exe install $pkg | Tee-Object -FilePath 'ncos-016-smoke.log'
if ($LASTEXITCODE -ne 0) { throw 'nexiora_package install fallo.' }
if (!(Test-Path '.\Knowledge\NCOS\Packages\ncos-016-nexiora-package-manager\registry.txt')) { throw 'No se genero registry.txt del paquete.' }
$registry = Get-Content '.\Knowledge\NCOS\Packages\ncos-016-nexiora-package-manager\registry.txt' -Raw
if (!$registry.Contains('status=installed')) { throw 'El registry no marco el paquete como instalado.' }
if (!$out -match 'Package Manager') { throw 'No se ejecuto Nexiora Package Manager.' }

Write-Host '== NCOS-016 complete =='
