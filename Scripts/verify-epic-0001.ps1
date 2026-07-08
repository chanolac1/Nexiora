$ErrorActionPreference = "Stop"

Write-Host "[EPIC-0001] Verifying Nexiora Autonomous Laboratory Foundation..."

if (!(Test-Path ".\CMakeLists.txt")) {
    throw "Run this script from the Nexiora repository root."
}

if (!(Test-Path ".\Research\Reports")) {
    New-Item -ItemType Directory -Force -Path ".\Research\Reports" | Out-Null
}

cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "[EPIC-0001] Verification completed."
Write-Host "Commit with: git add .; git commit -m \"EPIC-0001: Autonomous Laboratory Foundation\"; git push"
