param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$pkg = Join-Path $rootPath ".ncos_packages\EPIC-03_SPRINT-02_DecoderOCRAdapters"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"

Push-Location $rootPath
try {
    if (-not (Test-Path $pm)) {
        Write-Host "Package Manager no encontrado; reconstruyendo bootstrap oficial..."
        cmake --preset windows-msvc-release
        if ($LASTEXITCODE -ne 0) { throw "No fue posible configurar el bootstrap" }
        cmake --build --preset release --target nexiora_package
        if ($LASTEXITCODE -ne 0) { throw "No fue posible reconstruir nexiora_package.exe" }
        if (-not (Test-Path $pm)) { throw "Bootstrap sin Package Manager: $pm" }
    }

    & $pm verify $pkg
    if ($LASTEXITCODE -ne 0) { throw "verify falló" }
    & $pm deps $pkg
    if ($LASTEXITCODE -ne 0) { throw "deps falló" }
    & $pm install $pkg
    if ($LASTEXITCODE -ne 0) { throw "install falló" }

    $logDir = Join-Path $rootPath "Build\verification"
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $configureLog = Join-Path $logDir "epic03_sprint02_configure.log"
    $focusedLog = Join-Path $logDir "epic03_sprint02_focused.log"
    $fullLog = Join-Path $logDir "epic03_sprint02_full.log"

    cmake --preset windows-msvc-release 2>&1 | Tee-Object -FilePath $configureLog
    if ($LASTEXITCODE -ne 0) { throw "Configuración CMake falló" }

    cmake --build --preset release --target nexiora_decode NxDecoderAdaptersTests 2>&1 | Tee-Object -FilePath $focusedLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación enfocada falló" }
    if (-not (Test-Path (Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_decode.exe"))) { throw "nexiora_decode.exe no fue generado" }

    ctest --test-dir .\Build\windows-msvc-release -R '^NxDecoderAdaptersTests$' --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Prueba enfocada falló" }

    cmake --build --preset release 2>&1 | Tee-Object -FilePath $fullLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación completa falló" }

    $warnings = @(Select-String -Path $configureLog,$focusedLog,$fullLog -Pattern '(?i)\bwarning\s*:' -ErrorAction SilentlyContinue)
    if ($warnings.Count -ne 0) {
        $warnings | ForEach-Object { Write-Host $_.Line }
        throw "La entrega emitió $($warnings.Count) warning(s); Nexiora exige cero warnings"
    }

    ctest --preset release-tests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Suite completa falló" }

    & (Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_decode.exe") status
    if ($LASTEXITCODE -ne 0) { throw "La CLI de adaptadores no respondió" }

    & $pm history "EPIC-03 Sprint-02 Decoder and OCR Adapters"
    if ($LASTEXITCODE -ne 0) { throw "history falló" }

    Write-Host "EPIC-03 Sprint-02 verified: decoder adapters installed, 0 warnings, all tests passed."
}
finally {
    Pop-Location
}
