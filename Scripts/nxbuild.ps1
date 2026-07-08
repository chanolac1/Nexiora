param(
    [Parameter(Position=0)]
    [ValidateSet("doctor", "configure", "build", "test", "benchmark", "approve", "clean", "all")]
    [string]$Command = "all",

    [Parameter(Position=1)]
    [string]$Module = "all",

    [string]$Preset = "windows-msvc-debug",
    [string]$BuildPreset = "debug",
    [string]$TestPreset = "debug-tests",
    [int]$Iterations = 1000000,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$RemainingArgs
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "Build\windows-msvc-debug"
$Bin = Join-Path $BuildDir "bin"
$BenchExe = Join-Path $Bin "nexiora_bench.exe"
$MainExe = Join-Path $Bin "nexiora.exe"

function Resolve-BenchmarkModule {
    param([string]$InitialModule, [string[]]$Args)

    $resolved = $InitialModule

    if ($Args) {
        for ($i = 0; $i -lt $Args.Count; $i++) {
            if ($Args[$i] -eq "--module" -and ($i + 1) -lt $Args.Count) {
                $resolved = $Args[$i + 1]
            }
            elseif ($Args[$i] -eq "--iterations" -and ($i + 1) -lt $Args.Count) {
                $script:Iterations = [int]$Args[$i + 1]
            }
        }
    }

    if ($resolved -eq "--module" -and $Args -and $Args.Count -gt 0) {
        $resolved = $Args[0]
    }

    if ([string]::IsNullOrWhiteSpace($resolved)) {
        $resolved = "all"
    }

    return $resolved.ToLowerInvariant()
}

function Invoke-Doctor {
    Write-Host "=========================="
    Write-Host " NEXIORA BUILD DOCTOR"
    Write-Host "=========================="

    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    $ctest = Get-Command ctest -ErrorAction SilentlyContinue
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    $git = Get-Command git -ErrorAction SilentlyContinue

    Write-Host ("CMake..................... {0}" -f ($(if ($cmake) {"OK"} else {"MISSING"})))
    Write-Host ("CTest..................... {0}" -f ($(if ($ctest) {"OK"} else {"MISSING"})))
    Write-Host ("Ninja..................... {0}" -f ($(if ($ninja) {"OK"} else {"MISSING or not in PATH"})))
    Write-Host ("Git....................... {0}" -f ($(if ($git) {"OK"} else {"MISSING or not in PATH"})))
    Write-Host "Windows................... $([Environment]::OSVersion.VersionString)"
    Write-Host "Processors................ $([Environment]::ProcessorCount)"
    Write-Host "Root...................... $Root"

    if (-not $cmake) { throw "CMake no esta disponible en PATH." }
    if (-not $ctest) { throw "CTest no esta disponible en PATH." }
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
    param([string]$SelectedModule)

    if (-not (Test-Path $BenchExe)) {
        throw "No existe $BenchExe. Ejecuta primero: .\Scripts\nxbuild.ps1 build"
    }

    Push-Location $Root
    & $BenchExe --iterations $Iterations --module $SelectedModule
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

function Invoke-Clean {
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "Removed: $BuildDir"
    } else {
        Write-Host "Build directory does not exist: $BuildDir"
    }
}

$ResolvedModule = Resolve-BenchmarkModule -InitialModule $Module -Args $RemainingArgs

switch ($Command) {
    "doctor" { Invoke-Doctor }
    "configure" { Invoke-Configure }
    "build" { Invoke-Build }
    "test" { Invoke-Tests }
    "benchmark" { Invoke-Benchmark -SelectedModule $ResolvedModule }
    "approve" { Invoke-Approve }
    "clean" { Invoke-Clean }
    "all" {
        Invoke-Doctor
        Invoke-Configure
        Invoke-Build
        Invoke-Tests
        Invoke-Benchmark -SelectedModule $ResolvedModule
    }
}
