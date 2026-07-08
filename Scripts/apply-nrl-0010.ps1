$ErrorActionPreference = "Stop"

function Write-Step($message) {
    Write-Host ""
    Write-Host "==> $message" -ForegroundColor Cyan
}

$root = Get-Location
$cmakePath = Join-Path $root "CMakeLists.txt"
if (!(Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found. Run this script from the Nexiora repository root."
}

Write-Step "Backing up and repairing CMakeLists.txt"
$backupPath = Join-Path $root "CMakeLists.txt.nrl0010.full.bak"
Copy-Item $cmakePath $backupPath -Force

$content = Get-Content $cmakePath -Raw

# Repair malformed text left by older partial NRL-0010 script executions.
$content = $content -replace "\)\s*endif\(\)", ")`r`nendif()"
$content = $content -replace "\)\s*if\(", ")`r`nif("
$content = $content -replace "\)\s*add_executable\(", ")`r`n`r`nadd_executable("

# Add exporter source to NexioraNCP exactly once.
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
        throw "Could not find a source insertion anchor. Add Source/Research/NxResearchGraphExporter.c inside add_library(NexioraNCP STATIC ...)."
    }
}

# Remove any malformed or duplicate NRL-0010 target fragments from previous attempts.
$lines = $content -split "`r?`n"
$clean = New-Object System.Collections.Generic.List[string]
$skip = $false
$parenDepth = 0
foreach ($line in $lines) {
    if ($line -match "add_executable\(NxResearchGraphExporterTests" -or
        $line -match "add_executable\(NxResearchGraphExportSample" -or
        $line -match "add_executable\(NxResearchGraphExporterBenchmark") {
        $skip = $true
        $parenDepth = ([regex]::Matches($line, "\(")).Count - ([regex]::Matches($line, "\)")).Count
        if ($parenDepth -le 0) { $skip = $false }
        continue
    }

    if ($skip) {
        $parenDepth += ([regex]::Matches($line, "\(")).Count
        $parenDepth -= ([regex]::Matches($line, "\)")).Count
        if ($parenDepth -le 0) { $skip = $false }
        continue
    }

    if ($line -match "NxResearchGraphExporterTests" -or
        $line -match "NxResearchGraphExportSample" -or
        $line -match "NxResearchGraphExporterBenchmark") {
        continue
    }

    $clean.Add($line)
}
$content = ($clean -join "`r`n").TrimEnd() + "`r`n"

# Append NRL-0010 targets at the end. Keeping them outside the test if-block avoids fragile endif insertion.
$targetBlock = @"

# ============================================================
# NRL-0010 — Research Graph Exporter
# ============================================================

add_executable(NxResearchGraphExporterTests
    Tests/Unit/NxResearchGraphExporterTests.c
)
target_link_libraries(NxResearchGraphExporterTests PRIVATE NexioraNCP)
add_test(NAME NxResearchGraphExporterTests COMMAND NxResearchGraphExporterTests)

add_executable(NxResearchGraphExportSample
    Tools/ResearchGraph/NxResearchGraphExportSample.c
)
target_link_libraries(NxResearchGraphExportSample PRIVATE NexioraNCP)

add_executable(NxResearchGraphExporterBenchmark
    Tests/Benchmark/NxResearchGraphExporterBenchmark.c
)
target_link_libraries(NxResearchGraphExporterBenchmark PRIVATE NexioraNCP)
"@

$content += $targetBlock
Set-Content -Path $cmakePath -Value $content -Encoding UTF8

Write-Host "NRL-0010 applied successfully. Backup: $backupPath" -ForegroundColor Green
