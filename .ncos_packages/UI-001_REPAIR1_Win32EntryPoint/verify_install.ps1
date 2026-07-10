param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$packageDir = Join-Path $rootPath ".ncos_packages\UI-001_REPAIR1_Win32EntryPoint"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"

if (-not (Test-Path $pm)) {
    cmake --preset windows-msvc-release
    if ($LASTEXITCODE -ne 0) { throw "No fue posible configurar el bootstrap del Package Manager" }
    cmake --build --preset release --target nexiora_package
    if ($LASTEXITCODE -ne 0) { throw "No fue posible reconstruir el Package Manager" }
}

& $pm verify $packageDir
if ($LASTEXITCODE -ne 0) { throw "verify falló" }
& $pm deps $packageDir
if ($LASTEXITCODE -ne 0) { throw "deps falló" }
& $pm install $packageDir
if ($LASTEXITCODE -ne 0) { throw "install falló" }

$buildDir = Join-Path $rootPath "Build\control-center-release"
$outputDir = Join-Path $rootPath "Build\windows-msvc-release\bin"
$outputArgument = "-DNEXIORA_OUTPUT_DIR=$outputDir"

cmake -S (Join-Path $rootPath "Apps\ControlCenter") -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=Release $outputArgument
if ($LASTEXITCODE -ne 0) { throw "Configuración de Control Center falló" }

$log = Join-Path $buildDir "build.log"
cmake --build $buildDir --clean-first 2>&1 | Tee-Object -FilePath $log
if ($LASTEXITCODE -ne 0) { throw "Compilación de Control Center falló" }

$warnings = Select-String -Path $log -Pattern "warning:" -SimpleMatch
if ($warnings) { throw "Control Center emitió $($warnings.Count) warning(s)" }

ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Pruebas de Control Center fallaron" }

$exe = Join-Path $outputDir "nexiora_control_center.exe"
$testsExe = Join-Path $outputDir "NxControlCenterTests.exe"
if (-not (Test-Path $exe)) { throw "No se generó nexiora_control_center.exe en $outputDir" }
if (-not (Test-Path $testsExe)) { throw "No se generó NxControlCenterTests.exe en $outputDir" }

& $pm history "UI-001 Repair1 Win32 Entry Point"
Write-Host "UI-001 Repair1 verified: Unicode Win32 entry point fixed, output path fixed, 0 warnings and tests passed."
Write-Host "Launch: $exe"
