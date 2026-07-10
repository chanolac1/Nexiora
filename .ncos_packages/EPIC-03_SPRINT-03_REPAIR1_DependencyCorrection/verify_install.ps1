param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$packageDir = Join-Path $rootPath ".ncos_packages\EPIC-03_SPRINT-03_REPAIR1_DependencyCorrection"
$pm = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_package.exe"

function Invoke-Checked([scriptblock]$Command, [string]$Message) {
    & $Command
    if ($LASTEXITCODE -ne 0) { throw $Message }
}

if (-not (Test-Path $pm)) {
    Push-Location $rootPath
    try {
        Invoke-Checked { cmake --preset windows-msvc-release } "Configuración para bootstrap falló"
        Invoke-Checked { cmake --build --preset release --target nexiora_package } "Bootstrap del Package Manager falló"
    } finally { Pop-Location }
}
if (-not (Test-Path $pm)) { throw "Package Manager no pudo reconstruirse: $pm" }

Push-Location $rootPath
try {
    Invoke-Checked { & $pm verify $packageDir } "verify falló"
    Invoke-Checked { & $pm deps $packageDir } "deps falló"
    Invoke-Checked { & $pm install $packageDir } "install falló"
    Invoke-Checked { cmake --preset windows-msvc-release } "Configuración CMake falló"

    $focusedLog = Join-Path $rootPath "Build\epic03-sprint03-focused.log"
    & cmake --build --preset release --target nexiora_multimodal_cognitive NxMultimodalCognitiveIntegrationTests 2>&1 | Tee-Object -FilePath $focusedLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación enfocada falló" }
    $focusedWarnings = @(Select-String -Path $focusedLog -Pattern "warning:" -SimpleMatch)
    if ($focusedWarnings.Count -ne 0) { throw "Compilación enfocada produjo $($focusedWarnings.Count) warning(s)" }

    Invoke-Checked { ctest --test-dir .\Build\windows-msvc-release -R NxMultimodalCognitiveIntegrationTests --output-on-failure } "Prueba enfocada falló"

    $fullLog = Join-Path $rootPath "Build\epic03-sprint03-full.log"
    & cmake --build --preset release 2>&1 | Tee-Object -FilePath $fullLog
    if ($LASTEXITCODE -ne 0) { throw "Compilación completa falló" }
    $fullWarnings = @(Select-String -Path $fullLog -Pattern "warning:" -SimpleMatch)
    if ($fullWarnings.Count -ne 0) { throw "Compilación completa produjo $($fullWarnings.Count) warning(s)" }

    Invoke-Checked { ctest --test-dir .\Build\windows-msvc-release --output-on-failure } "Suite completa falló"

    $tool = Join-Path $rootPath "Build\windows-msvc-release\bin\nexiora_multimodal_cognitive.exe"
    if (-not (Test-Path $tool)) { throw "Ejecutable no generado: $tool" }
    Invoke-Checked { & $tool analyze $rootPath ".\Samples\Multimodal\memory_demo.nxknowledge" "memoria" } "Demostración cognitiva falló"
    Invoke-Checked { & $pm history "EPIC-03 Sprint-03 Repair1 Dependency Correction" } "history falló"
    Write-Host "EPIC-03 Sprint-03 Repair1 verified: dependency corrected, 0 warnings, all tests passed, grounded multimodal reasoning enabled."
} finally { Pop-Location }
