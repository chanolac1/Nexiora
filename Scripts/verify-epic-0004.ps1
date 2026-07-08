$ErrorActionPreference = "Stop"

Write-Host "== Nexiora EPIC-0004 verification =="

cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== EPIC-0004 verification completed =="
