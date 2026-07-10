param([string]$Root = ".")
$ErrorActionPreference = "Stop"

$resolvedRoot = (Resolve-Path $Root).Path
Set-Location $resolvedRoot

$exe = Join-Path $resolvedRoot "Build/windows-msvc-release/bin/nexiora_package.exe"
$package = Join-Path $resolvedRoot ".ncos_packages/DOCS-001_REPAIR_DocumentationGovernance"

if (-not (Test-Path $exe)) {
    throw "No se encontro nexiora_package.exe en: $exe"
}
if (-not (Test-Path (Join-Path $package "manifest.npkg"))) {
    throw "No se encontro manifest.npkg en: $package"
}

# La CLI NCOS-020 recibe solo <package_dir> para verify, deps e install.
& $exe verify $package
if ($LASTEXITCODE -ne 0) { throw "verify fallo con codigo $LASTEXITCODE" }

& $exe deps $package
if ($LASTEXITCODE -ne 0) { throw "deps fallo con codigo $LASTEXITCODE" }

& $exe install $package
if ($LASTEXITCODE -ne 0) { throw "install fallo con codigo $LASTEXITCODE" }

cmake --preset windows-msvc-release
if ($LASTEXITCODE -ne 0) { throw "cmake configure fallo" }

cmake --build --preset windows-msvc-release
if ($LASTEXITCODE -ne 0) { throw "build fallo" }

ctest --test-dir (Join-Path $resolvedRoot "Build/windows-msvc-release") -R NxDocumentationGovernanceTests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "prueba enfocada fallo" }

ctest --test-dir (Join-Path $resolvedRoot "Build/windows-msvc-release") --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "suite completa fallo" }

& (Join-Path $resolvedRoot "Build/windows-msvc-release/bin/nexiora_docs.exe") validate $resolvedRoot
if ($LASTEXITCODE -ne 0) { throw "validacion documental fallo" }

& $exe history "DOCS-001-REPAIR Documentation Governance Repair"
if ($LASTEXITCODE -ne 0) { throw "consulta de historial fallo" }

Write-Host "DOCS-001-REPAIR instalado y validado correctamente."
