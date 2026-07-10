$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkg = Join-Path $repo '.ncos_packages\NCOS-019_TransactionalInstallRollback_REPAIR'
$exe = Join-Path $repo 'Build\windows-msvc-release\bin\nexiora_package.exe'

Write-Host '== NCOS-019 repair: verify package =='
& $exe verify $pkg
if ($LASTEXITCODE -ne 0) { throw 'Package verification failed.' }

Write-Host '== NCOS-019 repair: dependencies =='
& $exe deps $pkg
if ($LASTEXITCODE -ne 0) { throw 'Package dependencies are not satisfied.' }

Write-Host '== NCOS-019 repair: install =='
& $exe install $pkg
if ($LASTEXITCODE -ne 0) { throw 'Package installation failed.' }

Write-Host '== NCOS-019 repair: configure/build =='
cmake --preset windows-msvc-release
if ($LASTEXITCODE -ne 0) { throw 'Configure failed.' }
cmake --build --preset release
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }

Write-Host '== NCOS-019 repair: focused test =='
ctest --test-dir .\Build\windows-msvc-release -R NxPackageManagerTests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'NxPackageManagerTests failed.' }

Write-Host '== NCOS-019 repair complete =='
