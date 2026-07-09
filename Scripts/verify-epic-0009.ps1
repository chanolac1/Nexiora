$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $Root

& (Join-Path $PSScriptRoot "apply-epic-0009.ps1")

Write-Host "== EPIC-0009: configuring =="
cmake --preset windows-msvc-release
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

Write-Host "== EPIC-0009: building =="
cmake --build --preset release
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

Write-Host "== EPIC-0009: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Tests failed" }

$tool = Join-Path $Root "Build/windows-msvc-release/bin/nexiora_acquire.exe"
if (-not (Test-Path $tool)) {
    throw "No se encontro nexiora_acquire.exe"
}

Write-Host "== EPIC-0009: smoke acquisition tool =="
& $tool "SQLite" | Tee-Object -FilePath "epic-0009-acquisition-smoke.log"
if ($LASTEXITCODE -ne 0) { throw "nexiora_acquire SQLite failed" }

$log = Get-Content "epic-0009-acquisition-smoke.log" -Raw
if ($log -notmatch "SQLite") { throw "El smoke test no imprimio el tema SQLite" }
if ($log -notmatch "Plan") { throw "El smoke test no imprimio un plan de adquisicion" }

Write-Host "== EPIC-0009 complete =="
Write-Host "Tool: $tool"
