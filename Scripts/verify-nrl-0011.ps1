$ErrorActionPreference = "Stop"

function Step($message) {
    Write-Host ""
    Write-Host "==> $message" -ForegroundColor Cyan
}

Step "Apply Sprint NRL-0011"
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-nrl-0011.ps1

Step "Configure CMake"
cmake --preset windows-msvc-release

Step "Build Release"
cmake --build --preset release

Step "Run tests"
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host ""
Write-Host "NRL-0011 completed successfully." -ForegroundColor Green
Write-Host ""
Write-Host "Commit:" -ForegroundColor Yellow
Write-Host "git add ."
Write-Host "git commit -m \"NRL-0011: Add Research Graph Query Engine\""
Write-Host "git push"
