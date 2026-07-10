param([string]$Root = ".")

$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
$packageExe = Join-Path $rootPath "Build/windows-msvc-release/bin/nexiora_package.exe"
$packageDir = Join-Path $rootPath ".ncos_packages/DOCS-002_REPAIR_DocumentationManagerBuildIntegration"
$buildDir = Join-Path $rootPath "Build/windows-msvc-release"
$docsExe = Join-Path $buildDir "bin/nexiora_docs.exe"

if (-not (Test-Path $packageExe -PathType Leaf)) {
    throw "No existe el Package Manager: $packageExe"
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

    cmake --build --preset windows-msvc-release --target nexiora_docs NxDocumentationManagerTests
    if ($LASTEXITCODE -ne 0) { throw "La compilación enfocada falló" }

    if (-not (Test-Path $docsExe -PathType Leaf)) {
        throw "La instalación se rechaza: nexiora_docs.exe no fue generado en $docsExe"
    }

    ctest --test-dir $buildDir -R "NxDocumentationManagerTests|NxDocumentationCliValidation" --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "Las pruebas enfocadas fallaron" }

    cmake --build --preset windows-msvc-release
    if ($LASTEXITCODE -ne 0) { throw "La compilación completa falló" }

    ctest --test-dir $buildDir --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "La suite completa falló" }

    & $docsExe validate $rootPath
    if ($LASTEXITCODE -ne 0) { throw "La documentación no está sincronizada" }

    & $packageExe history "DOCS-002-REPAIR Documentation Manager Build Integration"
    if ($LASTEXITCODE -ne 0) { throw "No fue posible consultar el historial" }
}
finally {
    Pop-Location
}
