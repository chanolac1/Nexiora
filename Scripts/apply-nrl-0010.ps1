$ErrorActionPreference = "Stop"

$cmakePath = Join-Path (Get-Location) "CMakeLists.txt"
if (!(Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found. Run this script from the Nexiora repository root."
}

$content = Get-Content $cmakePath -Raw

if ($content -notmatch [regex]::Escape("Source/Research/NxResearchGraphExporter.c")) {
    if ($content -match [regex]::Escape("Source/Research/NxResearchGraph.c")) {
        $content = $content.Replace(
            "Source/Research/NxResearchGraph.c",
            "Source/Research/NxResearchGraph.c`r`n    Source/Research/NxResearchGraphExporter.c")
    }
    elseif ($content -match [regex]::Escape("Source/Research/NxRegistry.c")) {
        $content = $content.Replace(
            "Source/Research/NxRegistry.c",
            "Source/Research/NxRegistry.c`r`n    Source/Research/NxResearchGraphExporter.c")
    }
    else {
        throw "Could not find a source insertion anchor in CMakeLists.txt. Add Source/Research/NxResearchGraphExporter.c manually inside add_library(NexioraNCP STATIC ...)."
    }
}

$testBlock = @"

    add_executable(NxResearchGraphExporterTests
        Tests/Unit/NxResearchGraphExporterTests.c
    )
    target_link_libraries(NxResearchGraphExporterTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphExporterTests COMMAND NxResearchGraphExporterTests)
"@

if ($content -notmatch [regex]::Escape("NxResearchGraphExporterTests")) {
    $marker = "endif()"
    $lastIndex = $content.LastIndexOf($marker)
    if ($lastIndex -lt 0) {
        throw "Could not find endif() to insert test target. Add the test block manually inside if(NEXIORA_BUILD_TESTS)."
    }

    $content = $content.Insert($lastIndex, $testBlock)
}

$sampleBlock = @"

add_executable(NxResearchGraphExportSample
    Tools/ResearchGraph/NxResearchGraphExportSample.c
)
target_link_libraries(NxResearchGraphExportSample PRIVATE NexioraNCP)

add_executable(NxResearchGraphExporterBenchmark
    Tests/Benchmark/NxResearchGraphExporterBenchmark.c
)
target_link_libraries(NxResearchGraphExporterBenchmark PRIVATE NexioraNCP)
"@

if ($content -notmatch [regex]::Escape("NxResearchGraphExportSample")) {
    $content += $sampleBlock
}

Set-Content -Path $cmakePath -Value $content -NoNewline
Write-Host "NRL-0010 CMake integration applied."
