param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$packageDir = Join-Path $rootPath ".ncos_packages\UI-001_NexioraControlCenter"
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
cmake -S (Join-Path $rootPath "Apps\ControlCenter") -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=Release -DNEXIORA_OUTPUT_DIR=$outputDir
if ($LASTEXITCODE -ne 0) { throw "Configuración de Control Center falló" }
$log = Join-Path $buildDir "build.log"
cmake --build $buildDir --clean-first 2>&1 | Tee-Object -FilePath $log
if ($LASTEXITCODE -ne 0) { throw "Compilación de Control Center falló" }
$warnings = Select-String -Path $log -Pattern "warning:" -SimpleMatch
if ($warnings) { throw "Control Center emitió $($warnings.Count) warning(s)" }
ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Pruebas de Control Center fallaron" }
$exe = Join-Path $outputDir "nexiora_control_center.exe"
if (-not (Test-Path $exe)) { throw "No se generó nexiora_control_center.exe" }
$docs = Join-Path $outputDir "nexiora_docs.exe"
$descriptor = Join-Path $rootPath "Docs\NCOS\UI-001_RELEASE_DESCRIPTOR.nxdoc"
if (Test-Path $docs) {
    & $docs finalize-sprint $rootPath $descriptor
    if ($LASTEXITCODE -ne 0) { throw "Actualización documental automática falló" }
}
& $pm history "UI-001 Nexiora Control Center"
Write-Host "UI-001 verified: native Windows UI, observable progress, 0 warnings and tests passed."
Write-Host "Launch: $exe"
