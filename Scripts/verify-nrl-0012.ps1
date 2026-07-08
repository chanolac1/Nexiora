$ErrorActionPreference = "Stop"

function Step($message) {
    Write-Host ""
    Write-Host "==> $message" -ForegroundColor Cyan
}

Step "Apply Sprint NRL-0012"
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-nrl-0012.ps1

Step "Configure CMake"
cmake --preset windows-msvc-release

Step "Build Release"
cmake --build --preset release

Step "Run tests"
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Step "Run scheduler benchmark if available"
$bench = ".\Build\windows-msvc-release\bin\NxResearchSchedulerBench.exe"
if (Test-Path $bench) {
    & $bench
} else {
    Write-Host "Benchmark executable not found; skipping direct benchmark run." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "NRL-0012 completed successfully." -ForegroundColor Green
Write-Host ""
Write-Host "Commit:" -ForegroundColor Yellow
Write-Host "git add ."
Write-Host "git commit -m \"NRL-0012: Add Research Scheduler\""
Write-Host "git push"
