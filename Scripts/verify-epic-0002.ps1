$ErrorActionPreference = "Stop"

Write-Host "== EPIC-0002: Knowledge Engine Foundation =="
Write-Host "Configuring release preset..."
cmake --preset windows-msvc-release

Write-Host "Building release preset..."
cmake --build --preset release

Write-Host "Running tests..."
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "EPIC-0002 verification completed successfully."
