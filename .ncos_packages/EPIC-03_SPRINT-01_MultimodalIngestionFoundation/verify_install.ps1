param([string]$Root = ".")
$ErrorActionPreference = "Stop"
$PackageDir = Join-Path $Root ".ncos_packages/EPIC-03_SPRINT-01_MultimodalIngestionFoundation"
$PackageExe = Join-Path $Root "Build/windows-msvc-release/bin/nexiora_package.exe"
& $PackageExe verify $PackageDir
if ($LASTEXITCODE -ne 0) { throw "verify failed" }
& $PackageExe deps $PackageDir
if ($LASTEXITCODE -ne 0) { throw "deps failed" }
& $PackageExe install $PackageDir
if ($LASTEXITCODE -ne 0) { throw "install failed" }
cmake --preset windows-msvc-release
if ($LASTEXITCODE -ne 0) { throw "configure failed" }
cmake --build --preset release --target nexiora_ingest NxMultimodalIngestionTests
if ($LASTEXITCODE -ne 0) { throw "focused build failed" }
$IngestExe = Join-Path $Root "Build/windows-msvc-release/bin/nexiora_ingest.exe"
if (-not (Test-Path $IngestExe)) { throw "nexiora_ingest.exe was not generated" }
ctest --test-dir (Join-Path $Root "Build/windows-msvc-release") -R NxMultimodalIngestionTests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "focused tests failed" }
cmake --build --preset release
if ($LASTEXITCODE -ne 0) { throw "full build failed" }
ctest --preset release-tests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "full tests failed" }
& $PackageExe history "EPIC-03 Sprint-01 Multimodal Ingestion Foundation"
if ($LASTEXITCODE -ne 0) { throw "history failed" }
Write-Host "EPIC-03 Sprint-01 installed and verified." -ForegroundColor Green
