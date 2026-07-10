param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$pkg = Join-Path $rootPath ".ncos_packages\EPIC-03_SPRINT-01_REPAIR4_HermeticMultimodalTests"
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
    $configureLog = Join-Path $logDir "epic03_repair4_configure.log"
    $focusedBuildLog = Join-Path $logDir "epic03_repair4_focused_build.log"
    $fullBuildLog = Join-Path $logDir "epic03_repair4_full_build.log"

    cmake --preset windows-msvc-release 2>&1 | Tee-Object -FilePath $configureLog
    if ($LASTEXITCODE -ne 0) { throw "Configuración CMake falló" }

    cmake --build --preset release --target NxMultimodalIngestionTests 2>&1 | Tee-Object -FilePath $focusedBuildLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación enfocada falló" }

    ctest --test-dir .\Build\windows-msvc-release -R '^NxMultimodalIngestionTests$' --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Prueba enfocada falló" }

    cmake --build --preset release 2>&1 | Tee-Object -FilePath $fullBuildLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación completa falló" }

    $warnings = @(Select-String -Path $configureLog,$focusedBuildLog,$fullBuildLog -Pattern '(?i)\bwarning\s*:' -ErrorAction SilentlyContinue)
    if ($warnings.Count -ne 0) {
        $warnings | ForEach-Object { Write-Host $_.Line }
        throw "La compilación emitió $($warnings.Count) warning(s); Nexiora exige cero warnings"
    }

    ctest --preset release-tests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Suite completa falló" }

    & $pm history "EPIC-03 Sprint-01 Repair4 Hermetic Multimodal Tests"
    if ($LASTEXITCODE -ne 0) { throw "history falló" }

    Write-Host "EPIC-03 Sprint-01 Repair4 verified: hermetic multimodal tests, 0 warnings, all tests passed."
}
finally {
    Pop-Location
}
