$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
while ($root -and !(Test-Path (Join-Path $root "CMakePresets.json"))) {
    $parent = Split-Path -Parent $root
    if ($parent -eq $root) { break }
    $root = $parent
}
if (!(Test-Path (Join-Path $root "CMakePresets.json"))) { throw "No se encontro la raiz del repositorio." }
Set-Location $root
$exe = ".\Build\windows-msvc-release\bin\nexiora_package.exe"
$pkg = ".\.ncos_packages\NCOS-020_PackageHistoryRecovery"
& $exe verify $pkg
& $exe deps $pkg
& $exe install $pkg
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release -R NxPackageManagerTests --output-on-failure
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
& $exe history "NCOS-020 Package History Recovery"
