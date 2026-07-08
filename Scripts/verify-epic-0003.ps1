$ErrorActionPreference = "Stop"

Write-Host "== Nexiora EPIC-0003 verification ==" -ForegroundColor Cyan

cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "EPIC-0003 verification completed." -ForegroundColor Green
