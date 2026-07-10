param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$pkg = Join-Path $rootPath ".ncos_packages\EPIC-03_SPRINT-01_REPAIR3_BootstrapGlobalZeroWarnings"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"

Push-Location $rootPath
try {
    if (-not (Test-Path $pm)) {
        Write-Host "Package Manager no encontrado; reconstruyendo bootstrap oficial..."
        cmake --preset windows-msvc-release
        if ($LASTEXITCODE -ne 0) { throw "No fue posible configurar el bootstrap del Package Manager" }
        cmake --build --preset release --target nexiora_package
        if ($LASTEXITCODE -ne 0) { throw "No fue posible reconstruir nexiora_package.exe" }
        if (-not (Test-Path $pm)) { throw "El bootstrap terminó sin generar: $pm" }
    }

    & $pm verify $pkg
    if ($LASTEXITCODE -ne 0) { throw "verify falló" }
    & $pm deps $pkg
    if ($LASTEXITCODE -ne 0) { throw "deps falló" }
    & $pm install $pkg
    if ($LASTEXITCODE -ne 0) { throw "install falló" }

    $logDir = Join-Path $rootPath "Build\verification"
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $configureLog = Join-Path $logDir "epic03_repair3_configure.log"
    $buildLog = Join-Path $logDir "epic03_repair3_build.log"

    cmake --preset windows-msvc-release 2>&1 | Tee-Object -FilePath $configureLog
    if ($LASTEXITCODE -ne 0) { throw "Configuración CMake falló" }

    cmake --build --preset release --clean-first 2>&1 | Tee-Object -FilePath $buildLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación completa falló" }

    $warnings = @(Select-String -Path $configureLog,$buildLog -Pattern '(?i)\bwarning\s*:' -ErrorAction SilentlyContinue)
    if ($warnings.Count -ne 0) {
        $warnings | ForEach-Object { Write-Host $_.Line }
        throw "La compilación emitió $($warnings.Count) warning(s); Nexiora exige cero warnings"
    }

    if (-not (Test-Path $pm)) { throw "La compilación completa no regeneró nexiora_package.exe" }

    ctest --preset release-tests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Suite completa falló" }

    & $pm history "EPIC-03 Sprint-01 Repair3 Bootstrap Global Zero Warnings"
    if ($LASTEXITCODE -ne 0) { throw "history falló" }

    Write-Host "EPIC-03 Sprint-01 Repair3 verified: Package Manager restored, 0 errors, 0 warnings, all tests passed."
}
finally {
    Pop-Location
}
