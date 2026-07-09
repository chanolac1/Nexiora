$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $root

Write-Host "== NCOS-005 repair2: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-ncos-005-repair2.ps1

Write-Host "== NCOS-005 repair2: cleaning stale concept test roots =="
$stale = @(
    ".\.nexiora_test_ncos003_registry",
    ".\Build\windows-msvc-release\.nexiora_test_ncos003_registry"
)
foreach ($p in $stale) {
    if (Test-Path $p) { Remove-Item -Path $p -Recurse -Force }
}

Write-Host "== NCOS-005 repair2: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-005 repair2: build =="
cmake --build --preset release

Write-Host "== NCOS-005 repair2: focused concept registry test =="
ctest --test-dir .\Build\windows-msvc-release -R NxConceptRegistryTests --output-on-failure

Write-Host "== NCOS-005 repair2: full test suite =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-005 repair2 complete =="
