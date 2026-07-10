$ErrorActionPreference = "Stop"

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($true) {
        if (Test-Path (Join-Path $dir "CMakePresets.json")) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw "No se encontro CMakePresets.json." }
        $dir = $parent
    }
}

$root = Find-RepoRoot
Set-Location $root
$pkg = Join-Path $root ".ncos_packages\NCOS-017_PackageVerificationWorkflow"
$exe = Join-Path $root "Build\windows-msvc-release\bin\nexiora_package.exe"

Write-Host "== NCOS-017: package install =="
if (!(Test-Path $pkg)) { throw "No existe el paquete: $pkg" }
if (!(Test-Path $exe)) { throw "No existe nexiora_package.exe. Instala/compila NCOS-016 primero." }

& $exe install $pkg | Tee-Object -FilePath "ncos-017-install.log"
if ($LASTEXITCODE -ne 0) { throw "Package install fallo." }

Write-Host "== NCOS-017: configure =="
cmake --preset windows-msvc-release
if ($LASTEXITCODE -ne 0) { throw "CMake configure fallo." }

Write-Host "== NCOS-017: build =="
cmake --build --preset release
if ($LASTEXITCODE -ne 0) { throw "Build fallo." }

Write-Host "== NCOS-017: focused test =="
ctest --test-dir .\Build\windows-msvc-release -R NxPackageManagerTests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "NxPackageManagerTests fallo." }

Write-Host "== NCOS-017: package verify =="
& $exe verify $pkg | Tee-Object -FilePath "ncos-017-verify.log"
if ($LASTEXITCODE -ne 0) { throw "Package verify fallo." }
$verifyLog = Get-Content "ncos-017-verify.log" -Raw
if ($verifyLog -notmatch "Estado\s*: VALID") { throw "El paquete no se marco como VALID." }
if ($verifyLog -notmatch "Payload faltantes\s*: 0") { throw "El paquete reporto payload faltante." }

Write-Host "== NCOS-017: sample package verify =="
$samplePkg = Join-Path $root "Samples\NCOS\PackageWorkflow"
& $exe verify $samplePkg | Tee-Object -FilePath "ncos-017-sample-verify.log"
if ($LASTEXITCODE -ne 0) { throw "Sample package verify fallo." }

Write-Host "== NCOS-017: full tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Suite completa fallo." }

Write-Host "== NCOS-017 complete =="
