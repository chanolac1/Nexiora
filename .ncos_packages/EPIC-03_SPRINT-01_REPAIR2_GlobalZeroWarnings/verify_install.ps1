param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$pkg = Join-Path $rootPath ".ncos_packages\EPIC-03_SPRINT-01_REPAIR2_GlobalZeroWarnings"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"
if (-not (Test-Path $pm)) { throw "Package Manager no encontrado: $pm" }
& $pm verify $pkg
if ($LASTEXITCODE -ne 0) { throw "verify falló" }
& $pm deps $pkg
if ($LASTEXITCODE -ne 0) { throw "deps falló" }
& $pm install $pkg
if ($LASTEXITCODE -ne 0) { throw "install falló" }
Push-Location $rootPath
try {
    $logDir = Join-Path $rootPath "Build\verification"
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $configureLog = Join-Path $logDir "epic03_repair2_configure.log"
    $buildLog = Join-Path $logDir "epic03_repair2_build.log"
    cmake --preset windows-msvc-release 2>&1 | Tee-Object -FilePath $configureLog
    if ($LASTEXITCODE -ne 0) { throw "Configuración CMake falló" }
    cmake --build --preset release --clean-first 2>&1 | Tee-Object -FilePath $buildLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación completa falló" }
    $warnings = @(Select-String -Path $configureLog,$buildLog -Pattern '(?i)\bwarning\s*:' -ErrorAction SilentlyContinue)
    if ($warnings.Count -ne 0) {
        $warnings | ForEach-Object { Write-Host $_.Line }
        throw "La compilación emitió $($warnings.Count) warning(s); Nexiora exige cero warnings"
    }
    ctest --preset release-tests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Suite completa falló" }
    & $pm history "EPIC-03 Sprint-01 Repair2 Global Zero Warnings"
    if ($LASTEXITCODE -ne 0) { throw "history falló" }
    Write-Host "EPIC-03 Sprint-01 Repair2 verified: 0 errors, 0 warnings, all tests passed."
}
finally { Pop-Location }
