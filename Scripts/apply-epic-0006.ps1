$ErrorActionPreference = "Stop"

Write-Host "== EPIC-0006: applying package (repair v2) =="

$root = (Get-Location).Path
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$packageRoot = Split-Path -Parent $scriptDir

function Same-Path($a, $b) {
    try {
        return ([System.IO.Path]::GetFullPath($a).TrimEnd('\') -ieq [System.IO.Path]::GetFullPath($b).TrimEnd('\'))
    } catch {
        return $false
    }
}

function Copy-Tree($relative) {
    $src = Join-Path $packageRoot $relative
    $dst = Join-Path $root $relative

    if (!(Test-Path $src)) {
        return
    }

    if (Same-Path $src $dst) {
        Write-Host "Skipping self-copy: $relative"
        return
    }

    New-Item -ItemType Directory -Force -Path $dst | Out-Null
    Get-ChildItem -Path $src -Force | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $dst -Recurse -Force
    }
}

Copy-Tree "Include"
Copy-Tree "Source"
Copy-Tree "Tests"
Copy-Tree "Docs"
Copy-Tree ".nexiora_sprints"

$readmeSrc = Join-Path $packageRoot "README_EPIC-0006.md"
$readmeDst = Join-Path $root "README_EPIC-0006.md"
if ((Test-Path $readmeSrc) -and !(Same-Path $readmeSrc $readmeDst)) {
    Copy-Item -Path $readmeSrc -Destination $readmeDst -Force
}

$cmakePath = Join-Path $root "CMakeLists.txt"
if (!(Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found. Run this script from the Nexiora repository root."
}

$backupPath = Join-Path $root "CMakeLists.txt.epic0006.bak"
if (!(Test-Path $backupPath)) {
    Copy-Item $cmakePath $backupPath -Force
}

$content = Get-Content $cmakePath -Raw

# Ensure source file is part of NexioraNCP exactly once.
if ($content -notmatch [regex]::Escape("Source/Research/NxResearchDashboard.c")) {
    $anchors = @(
        "Source/Research/NxAutonomousExecution.c",
        "Source/Research/NxDiscoveryEngine.c",
        "Source/Research/NxResearchKnowledge.c"
    )

    $inserted = $false
    foreach ($anchor in $anchors) {
        if (!$inserted -and $content -match [regex]::Escape($anchor)) {
            $content = $content -replace [regex]::Escape($anchor), "$anchor`r`n    Source/Research/NxResearchDashboard.c"
            $inserted = $true
        }
    }

    if (!$inserted) {
        throw "Could not find a safe source anchor in CMakeLists.txt for NxResearchDashboard.c"
    }
}

# Ensure test target exists exactly once.
if ($content -notmatch "add_executable\(NxResearchDashboardTests") {
    $testBlock = @"

if(NEXIORA_BUILD_TESTS)
    add_executable(NxResearchDashboardTests
        Tests/Unit/NxResearchDashboardTests.c
    )
    target_link_libraries(NxResearchDashboardTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchDashboardTests COMMAND NxResearchDashboardTests)
endif()
"@
    $content = $content.TrimEnd() + "`r`n" + $testBlock + "`r`n"
}

Set-Content -Path $cmakePath -Value $content -Encoding UTF8
Write-Host "EPIC-0006 applied/repaired. Backup: CMakeLists.txt.epic0006.bak"
