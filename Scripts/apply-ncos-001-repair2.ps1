$ErrorActionPreference = "Stop"
$root = (Get-Location).Path

Copy-Item -Path (Join-Path $PSScriptRoot "..\Source\NCOS\NxSessionEngine.c") -Destination (Join-Path $root "Source\NCOS\NxSessionEngine.c") -Force
Copy-Item -Path (Join-Path $PSScriptRoot "..\Tests\Unit\NxSessionEngineTests.c") -Destination (Join-Path $root "Tests\Unit\NxSessionEngineTests.c") -Force

# Clear only the isolated test directory if it exists.
$testRoot = Join-Path $root ".nexiora_test_session_engine_isolated"
if (Test-Path $testRoot) { Remove-Item -Recurse -Force $testRoot }

Write-Host "NCOS-001 repair2 applied: session engine now creates fresh root directories."
