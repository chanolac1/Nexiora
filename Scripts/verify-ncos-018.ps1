$ErrorActionPreference = "Stop"
function Find-RepoRoot {
    $dir=(Get-Location).Path
    while($true){
        if(Test-Path (Join-Path $dir "CMakePresets.json")){ return $dir }
        $parent=Split-Path $dir -Parent
        if($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)){ throw "No se encontro CMakePresets.json." }
        $dir=$parent
    }
}
$root=Find-RepoRoot
Set-Location $root
$pkg=Join-Path $root ".ncos_packages\NCOS-018_DependencyResolver"
$exe=Join-Path $root "Build\windows-msvc-release\bin\nexiora_package.exe"
if(!(Test-Path $exe)){ throw "NCOS-016 no esta compilado: falta nexiora_package.exe" }
Write-Host "== NCOS-018: dependency precheck =="
& $exe deps $pkg
if($LASTEXITCODE -ne 0){ throw "Faltan dependencias para NCOS-018." }
Write-Host "== NCOS-018: managed install =="
& $exe install $pkg
if($LASTEXITCODE -ne 0){ throw "Instalacion administrada fallo." }
cmake --preset windows-msvc-release
if($LASTEXITCODE -ne 0){ throw "Configure fallo." }
cmake --build --preset release
if($LASTEXITCODE -ne 0){ throw "Build fallo." }
ctest --test-dir .\Build\windows-msvc-release -R NxPackageManagerTests --output-on-failure
if($LASTEXITCODE -ne 0){ throw "NxPackageManagerTests fallo." }
Write-Host "== NCOS-018: verify installed resolver =="
& $exe deps $pkg | Tee-Object -FilePath "ncos-018-deps.log"
if($LASTEXITCODE -ne 0){ throw "Dependency resolver smoke fallo." }
$log=Get-Content "ncos-018-deps.log" -Raw
if($log -notmatch "Dependencias faltantes\s*:\s*0"){ throw "El resolver reporto dependencias faltantes." }
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
if($LASTEXITCODE -ne 0){ throw "Suite completa fallo." }
Write-Host "== NCOS-018 complete =="
