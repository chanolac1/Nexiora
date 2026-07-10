param([string]$Root = ".")

$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$packageExe = Join-Path $rootPath "Build/windows-msvc-release/bin/nexiora_package.exe"
$packageDir = Join-Path $rootPath ".ncos_packages/DOCS-002_REPAIR2_DocumentationManagerPresetIntegration"
$buildDir = Join-Path $rootPath "Build/windows-msvc-release"
$docsExe = Join-Path $buildDir "bin/nexiora_docs.exe"
$presetsFile = Join-Path $rootPath "CMakePresets.json"

if (-not (Test-Path $packageExe -PathType Leaf)) {
    throw "No existe el Package Manager: $packageExe"
}
if (-not (Test-Path $presetsFile -PathType Leaf)) {
    throw "No existe CMakePresets.json: $presetsFile"
}

$presets = Get-Content $presetsFile -Raw | ConvertFrom-Json
$configureNames = @($presets.configurePresets | ForEach-Object { $_.name })
$buildNames = @($presets.buildPresets | ForEach-Object { $_.name })
$testNames = @($presets.testPresets | ForEach-Object { $_.name })

if ($configureNames -notcontains "windows-msvc-release") {
    throw "Falta el configure preset requerido: windows-msvc-release"
}
if ($buildNames -notcontains "release") {
    throw "Falta el build preset requerido: release"
}
if ($testNames -notcontains "release-tests") {
    throw "Falta el test preset requerido: release-tests"
}

& $packageExe verify $packageDir
if ($LASTEXITCODE -ne 0) { throw "verify falló" }

& $packageExe deps $packageDir
if ($LASTEXITCODE -ne 0) { throw "deps falló" }

& $packageExe install $packageDir
if ($LASTEXITCODE -ne 0) { throw "install falló" }

Push-Location $rootPath
try {
    cmake --preset windows-msvc-release
    if ($LASTEXITCODE -ne 0) { throw "La configuración CMake falló" }

    cmake --build --preset release --target nexiora_docs NxDocumentationManagerTests
    if ($LASTEXITCODE -ne 0) { throw "La compilación enfocada falló" }

    if (-not (Test-Path $docsExe -PathType Leaf)) {
        throw "La instalación se rechaza: nexiora_docs.exe no fue generado en $docsExe"
    }

    ctest --preset release-tests -R "NxDocumentationManagerTests|NxDocumentationCliValidation" --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Las pruebas enfocadas fallaron" }

    cmake --build --preset release
    if ($LASTEXITCODE -ne 0) { throw "La compilación completa falló" }

    ctest --preset release-tests --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "La suite completa falló" }

    & $docsExe validate $rootPath
    if ($LASTEXITCODE -ne 0) { throw "La documentación no está sincronizada" }

    & $packageExe history "DOCS-002-REPAIR2 Documentation Manager Preset Integration"
    if ($LASTEXITCODE -ne 0) { throw "No fue posible consultar el historial" }
}
finally {
    Pop-Location
}
