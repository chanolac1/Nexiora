$ErrorActionPreference = "Stop"

$cmakePath = Join-Path (Get-Location) "CMakeLists.txt"
if (!(Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found. Run this script from the Nexiora repository root."
}

$backupPath = "$cmakePath.nrl0010.bak"
Copy-Item $cmakePath $backupPath -Force

$content = Get-Content $cmakePath -Raw

# Fix the common malformed token produced by the integration script:
# CMake needs endif() on its own line, not glued after a closing paren.
$content = $content -replace "\)\s*endif\(\)", ")`r`nendif()"

# Fix accidental glued source entries if any were produced.
$content = $content -replace "NxResearchGraphExporter\.c\s+Source/", "NxResearchGraphExporter.c`r`n    Source/"

# Ensure the exporter source exists inside NexioraNCP source list.
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
        throw "Could not find a safe source anchor. Add Source/Research/NxResearchGraphExporter.c manually inside add_library(NexioraNCP STATIC ...)."
    }
}

# Add test target only if it does not already exist.
if ($content -notmatch [regex]::Escape("NxResearchGraphExporterTests")) {
    $testBlock = @"

    add_executable(NxResearchGraphExporterTests
        Tests/Unit/NxResearchGraphExporterTests.c
    )
    target_link_libraries(NxResearchGraphExporterTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphExporterTests COMMAND NxResearchGraphExporterTests)
"@

    $lastIndex = $content.LastIndexOf("endif()")
    if ($lastIndex -lt 0) {
        throw "Could not find endif() to insert test target."
    }
    $content = $content.Insert($lastIndex, $testBlock)
}

# Add sample tool only if missing.
if ($content -notmatch [regex]::Escape("NxResearchGraphExportSample")) {
    $content += @"

add_executable(NxResearchGraphExportSample
    Tools/ResearchGraph/NxResearchGraphExportSample.c
)
target_link_libraries(NxResearchGraphExportSample PRIVATE NexioraNCP)
"@
}

# Add benchmark only if missing.
if ($content -notmatch [regex]::Escape("NxResearchGraphExporterBenchmark")) {
    $content += @"

add_executable(NxResearchGraphExporterBenchmark
    Tests/Benchmark/NxResearchGraphExporterBenchmark.c
)
target_link_libraries(NxResearchGraphExporterBenchmark PRIVATE NexioraNCP)
"@
}

Set-Content -Path $cmakePath -Value $content -NoNewline
Write-Host "NRL-0010 CMakeLists.txt repaired. Backup saved to CMakeLists.txt.nrl0010.bak"
