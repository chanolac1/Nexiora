$ErrorActionPreference="Stop"
function Find-RepoRoot { $d=(Get-Location).Path; while($true){ if(Test-Path (Join-Path $d "CMakePresets.json")){return $d}; $p=Split-Path $d -Parent; if($p -eq $d){throw "No se encontro repo"}; $d=$p } }
$root=Find-RepoRoot; Set-Location $root
$pkg=Join-Path $root ".ncos_packages\NCOS-019_TransactionalInstallRollback"
$exe=Join-Path $root "Build\windows-msvc-release\bin\nexiora_package.exe"
& $exe verify $pkg; if($LASTEXITCODE -ne 0){throw "verify fallo"}
& $exe deps $pkg; if($LASTEXITCODE -ne 0){throw "deps fallo"}
& $exe install $pkg; if($LASTEXITCODE -ne 0){throw "install fallo"}
cmake --preset windows-msvc-release; if($LASTEXITCODE -ne 0){throw "configure fallo"}
cmake --build --preset release; if($LASTEXITCODE -ne 0){throw "build fallo"}
ctest --test-dir .\Build\windows-msvc-release -R NxPackageManagerTests --output-on-failure; if($LASTEXITCODE -ne 0){throw "focused test fallo"}
ctest --test-dir .\Build\windows-msvc-release --output-on-failure; if($LASTEXITCODE -ne 0){throw "suite fallo"}
Write-Host "== NCOS-019 complete =="
