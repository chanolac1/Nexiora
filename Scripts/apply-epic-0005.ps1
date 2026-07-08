$ErrorActionPreference = "Stop"

$root = (Get-Location).Path
$sprint = Join-Path $root ".nexiora_sprints\EPIC-0005_FirstAutonomousExecution"

function Copy-File($relative) {
    $src = Join-Path $sprint $relative
    $dst = Join-Path $root $relative
    $dir = Split-Path $dst -Parent
    if (!(Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    Copy-Item -Force $src $dst
}

if (!(Test-Path $sprint)) {
    throw "Sprint payload not found: $sprint. Descomprime el ZIP sobre la raíz de Nexiora."
}

Copy-File "Include\Nexiora\Research\NxAutonomousExecution.h"
Copy-File "Source\Research\NxAutonomousExecution.c"
Copy-File "Tests\Unit\NxAutonomousExecutionTests.c"
Copy-File "Source\NxBootstrap.c"
Copy-File "Docs\RFC\RFC-0031-First-Autonomous-Execution.md"
Copy-File "Docs\SPEC\SPEC-EPIC-0005-FirstAutonomousExecution.md"
Copy-File "Docs\BOOK\HISTORY-EPIC-0005.md"
Copy-File "README_EPIC-0005.md"

$cmakePath = Join-Path $root "CMakeLists.txt"
if (!(Test-Path $cmakePath)) { throw "CMakeLists.txt not found" }
$cmake = Get-Content $cmakePath -Raw
Copy-Item $cmakePath "$cmakePath.epic0005.bak" -Force

function Add-After($text, $needle, $addition) {
    if ($text.Contains($addition.Trim())) { return $text }
    if ($text.Contains($needle)) { return $text.Replace($needle, "$needle`r`n    $addition") }
    return $text
}

$cmake = Add-After $cmake "Source/Research/NxResearchScheduler.c" "Source/Research/NxDiscoveryEngine.c"
$cmake = Add-After $cmake "Source/Research/NxResearchScheduler.c" "Source/Research/NxAutonomousExecution.c"

if (!$cmake.Contains("NxAutonomousExecutionTests")) {
$cmake += @"

# ============================================================
# EPIC-0005 — First Autonomous Research Execution
# ============================================================

if(NEXIORA_BUILD_TESTS)
    add_executable(NxAutonomousExecutionTests
        Tests/Unit/NxAutonomousExecutionTests.c
    )
    target_link_libraries(NxAutonomousExecutionTests PRIVATE NexioraNCP)
    add_test(NAME NxAutonomousExecutionTests COMMAND NxAutonomousExecutionTests)
endif()
"@
}

Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8
Write-Host "EPIC-0005 applied. Backup: CMakeLists.txt.epic0005.bak"
