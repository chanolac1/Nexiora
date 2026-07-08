$ErrorActionPreference = "Stop"

$cmakePath = Join-Path (Get-Location) "CMakeLists.txt"
if (!(Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found. Run this script from the Nexiora repository root."
}

$content = Get-Content $cmakePath -Raw

if ($content -notmatch [regex]::Escape("Source/Research/NxResearchGraph.c")) {
    $anchor = "Source/Research/NxRegistry.c"
    if ($content -match [regex]::Escape($anchor)) {
        $content = $content -replace [regex]::Escape($anchor), "$anchor`r`n    Source/Research/NxResearchGraph.c"
    } else {
        throw "Could not find Source/Research/NxRegistry.c anchor. Add Source/Research/NxResearchGraph.c manually inside add_library(NexioraNCP STATIC ...)."
    }
}

if ($content -notmatch "NxResearchGraphTests") {
    $testBlock = @"

    add_executable(NxResearchGraphTests
        Tests/Unit/NxResearchGraphTests.c
    )
    target_link_libraries(NxResearchGraphTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphTests COMMAND NxResearchGraphTests)
"@

    $lastTest = "add_test(NAME NxPromotionTests COMMAND NxPromotionTests)"
    if ($content -match [regex]::Escape($lastTest)) {
        $content = $content -replace [regex]::Escape($lastTest), "$lastTest$testBlock"
    } else {
        $runnerTest = "add_test(NAME NxExperimentRunnerTests COMMAND NxExperimentRunnerTests)"
        if ($content -match [regex]::Escape($runnerTest)) {
            $content = $content -replace [regex]::Escape($runnerTest), "$runnerTest$testBlock"
        } else {
            throw "Could not find a test insertion anchor. Add NxResearchGraphTests manually using CMakeLists.additions.txt."
        }
    }
}

if ($content -notmatch "NxResearchGraphBenchmark") {
    $benchmarkBlock = @"

if(NEXIORA_ENABLE_BENCHMARKS)
    add_executable(NxResearchGraphBenchmark
        Tests/Benchmark/NxResearchGraphBenchmark.c
    )
    target_link_libraries(NxResearchGraphBenchmark PRIVATE NexioraNCP)
endif()
"@
    $content = $content + $benchmarkBlock
}

Set-Content -Path $cmakePath -Value $content -NoNewline
Write-Host "NRL-0009 CMake integration applied."
