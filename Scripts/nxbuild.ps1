param(
    [Parameter(Position=0)]
    [ValidateSet("doctor", "configure", "build", "test", "benchmark", "approve", "all")]
    [string]$Command = "all",

    [string]$Preset = "windows-msvc-debug",
    [string]$BuildPreset = "debug",
    [string]$TestPreset = "debug-tests",
    [int]$Iterations = 100000
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Bin = Join-Path $Root "Build\windows-msvc-debug\bin"
$BenchExe = Join-Path $Bin "nexiora_bench.exe"

function Invoke-Doctor {
    Write-Host "=========================="
    Write-Host " NEXIORA BUILD DOCTOR"
    Write-Host "=========================="

    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    $ctest = Get-Command ctest -ErrorAction SilentlyContinue
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue

    Write-Host ("CMake..................... {0}" -f ($(if ($cmake) {"OK"} else {"MISSING"})))
    Write-Host ("CTest..................... {0}" -f ($(if ($ctest) {"OK"} else {"MISSING"})))
    Write-Host ("Ninja..................... {0}" -f ($(if ($ninja) {"OK"} else {"MISSING or not in PATH"})))
    Write-Host "Windows................... $([Environment]::OSVersion.VersionString)"
    Write-Host "Processors................ $([Environment]::ProcessorCount)"

    if (-not $cmake) { throw "CMake no está disponible en PATH." }
    if (-not $ctest) { throw "CTest no está disponible en PATH." }
}

function Invoke-Configure {
    Push-Location $Root
    cmake --preset $Preset
    Pop-Location
}

function Invoke-Build {
    Push-Location $Root
    cmake --build --preset $BuildPreset
    Pop-Location
}

function Invoke-Tests {
    Push-Location $Root
    ctest --preset $TestPreset --output-on-failure
    Pop-Location
}

function Invoke-Benchmark {
    if (-not (Test-Path $BenchExe)) {
        throw "No existe $BenchExe. Ejecuta primero: .\Scripts\nxbuild.ps1 build"
    }
    Push-Location $Root
    & $BenchExe --iterations $Iterations
    Pop-Location
}

function Invoke-Approve {
    $report = Join-Path $Root "Benchmarks\Reports\latest_evidence_report.txt"
    if (-not (Test-Path $report)) {
        throw "No existe reporte para aprobar. Ejecuta primero: .\Scripts\nxbuild.ps1 benchmark"
    }

    $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $approvalDir = Join-Path $Root "Benchmarks\Approvals"
    New-Item -ItemType Directory -Force -Path $approvalDir | Out-Null
    $approvalFile = Join-Path $approvalDir "approval-$stamp.txt"

    "Nexiora manual approval" | Out-File -Encoding utf8 $approvalFile
    "ApprovedAt: $(Get-Date -Format o)" | Out-File -Encoding utf8 -Append $approvalFile
    "ApprovedBy: local-user" | Out-File -Encoding utf8 -Append $approvalFile
    "" | Out-File -Encoding utf8 -Append $approvalFile
    Get-Content $report | Out-File -Encoding utf8 -Append $approvalFile

    Write-Host "Approval written: $approvalFile"
}

switch ($Command) {
    "doctor" { Invoke-Doctor }
    "configure" { Invoke-Configure }
    "build" { Invoke-Build }
    "test" { Invoke-Tests }
    "benchmark" { Invoke-Benchmark }
    "approve" { Invoke-Approve }
    "all" {
        Invoke-Doctor
        Invoke-Configure
        Invoke-Build
        Invoke-Tests
        Invoke-Benchmark
    }
}
