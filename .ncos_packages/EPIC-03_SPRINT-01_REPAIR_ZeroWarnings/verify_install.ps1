param([string]$Root = ".")

$ErrorActionPreference = "Stop"
$PackageDir = Join-Path $Root ".ncos_packages/EPIC-03_SPRINT-01_REPAIR_ZeroWarnings"
$PackageExe = Join-Path $Root "Build/windows-msvc-release/bin/nexiora_package.exe"
$BuildDir = Join-Path $Root "Build/windows-msvc-release"

function Invoke-CapturedCommand {
    param(
        [string]$Name,
        [scriptblock]$Command,
        [bool]$RejectWarnings
    )

    $output = & $Command 2>&1
    $exitCode = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }

    if ($exitCode -ne 0) {
        throw "$Name failed with exit code $exitCode"
    }

    if ($RejectWarnings) {
        $warningLines = @($output | Select-String -Pattern '(?i)(^|\s)warning(?:\s+[A-Z]+[0-9]+)?\s*:')
        if ($warningLines.Count -gt 0) {
            $warningLines | ForEach-Object { Write-Host $_ -ForegroundColor Red }
            throw "$Name emitted $($warningLines.Count) warning(s); Nexiora requires zero warnings"
        }
    }
}

& $PackageExe verify $PackageDir
if ($LASTEXITCODE -ne 0) { throw "verify failed" }

& $PackageExe deps $PackageDir
if ($LASTEXITCODE -ne 0) { throw "deps failed" }

& $PackageExe install $PackageDir
if ($LASTEXITCODE -ne 0) { throw "install failed" }

Invoke-CapturedCommand -Name "CMake configure" -RejectWarnings $true -Command {
    cmake --preset windows-msvc-release
}

Invoke-CapturedCommand -Name "Focused multimodal build" -RejectWarnings $true -Command {
    cmake --build --preset release --target nexiora_ingest NxMultimodalIngestionTests --clean-first
}

$IngestExe = Join-Path $BuildDir "bin/nexiora_ingest.exe"
if (-not (Test-Path $IngestExe)) {
    throw "nexiora_ingest.exe was not generated"
}

ctest --test-dir $BuildDir -R NxMultimodalIngestionTests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "focused tests failed" }

Invoke-CapturedCommand -Name "Full build" -RejectWarnings $true -Command {
    cmake --build --preset release
}

ctest --preset release-tests --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "full tests failed" }

& $PackageExe history "EPIC-03 Sprint-01 Repair Zero Warnings"
if ($LASTEXITCODE -ne 0) { throw "history failed" }

Write-Host "EPIC-03 Sprint-01 Repair installed and verified with zero warnings." -ForegroundColor Green
