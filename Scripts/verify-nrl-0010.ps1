$ErrorActionPreference = "Stop"

function Step($message) {
    Write-Host ""
    Write-Host "==> $message" -ForegroundColor Cyan
}

Step "Apply Sprint NRL-0010"
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-nrl-0010.ps1

Step "Configure CMake"
cmake --preset windows-msvc-release

Step "Build Release"
cmake --build --preset release

Step "Run tests"
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Step "Generate visible research graph"
powershell -ExecutionPolicy Bypass -File .\Scripts\generate-research-graph.ps1

Write-Host ""
Write-Host "NRL-0010 completed. If the SVG opened, the visual graph exporter is working." -ForegroundColor Green
Write-Host "Commit:" -ForegroundColor Yellow
Write-Host "git add ."
Write-Host "git commit -m \"NRL-0010: Add Research Graph Exporter\""
Write-Host "git push"
