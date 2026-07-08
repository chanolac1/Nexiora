$ErrorActionPreference = "Stop"

powershell -ExecutionPolicy Bypass -File .\Scripts\apply-epic-0006.ps1

Write-Host "== EPIC-0006: configuring =="
cmake --preset windows-msvc-release

Write-Host "== EPIC-0006: building =="
cmake --build --preset release

Write-Host "== EPIC-0006: testing =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== EPIC-0006: generating research session =="
.\Build\windows-msvc-release\bin\nexiora.exe research run

Write-Host "== EPIC-0006: generating dashboard =="
.\Build\windows-msvc-release\bin\nexiora.exe research dashboard

$dashboard = Join-Path (Get-Location) "Research\Sessions\first_autonomous_execution\dashboard.html"
if (Test-Path $dashboard) {
    Write-Host "Dashboard generated: $dashboard"
    Start-Process $dashboard
} else {
    throw "dashboard.html was not generated"
}
