$ErrorActionPreference = "Stop"

function Step($message) {
    Write-Host ""
    Write-Host "==> $message" -ForegroundColor Cyan
}

function Ensure-Directory($path) {
    if (-not (Test-Path $path)) {
        New-Item -ItemType Directory -Path $path | Out-Null
    }
}

function Add-Line-AfterAnchor($content, $lineToAdd, $anchors) {
    if ($content.Contains($lineToAdd)) {
        return $content
    }

    foreach ($anchor in $anchors) {
        if ($content.Contains($anchor)) {
            return $content.Replace($anchor, "$anchor`r`n    $lineToAdd")
        }
    }

    throw "Unable to patch CMakeLists.txt. Anchor not found for: $lineToAdd"
}

function Add-Block-BeforeSectionEnd($content, $sectionName, $block, $marker) {
    if ($content.Contains($marker)) {
        return $content
    }

    $pattern = "(?s)(if\s*\(\s*$sectionName\s*\).*?)(\r?\nendif\s*\(\s*\))"
    $match = [regex]::Match($content, $pattern)

    if (-not $match.Success) {
        throw "Unable to find CMake section: $sectionName"
    }

    $replacement = $match.Groups[1].Value.TrimEnd() + "`r`n`r`n" + $block.TrimEnd() + $match.Groups[2].Value
    return $content.Substring(0, $match.Index) + $replacement + $content.Substring($match.Index + $match.Length)
}

$root = (Get-Location).Path
$payload = Join-Path $root ".nexiora_sprints\NRL-0012_ResearchScheduler_SprintCompleto"

if (-not (Test-Path $payload)) {
    throw "Sprint payload not found: $payload"
}

Step "Copy Sprint files"
Ensure-Directory "Include\Nexiora\Research"
Ensure-Directory "Source\Research"
Ensure-Directory "Tests\Unit"
Ensure-Directory "Tests\Benchmark"
Ensure-Directory "Docs\RFC"
Ensure-Directory "Docs\SPEC"
Ensure-Directory "Docs\BOOK"

Copy-Item "$payload\Include\Nexiora\Research\NxResearchScheduler.h" "Include\Nexiora\Research\NxResearchScheduler.h" -Force
Copy-Item "$payload\Source\Research\NxResearchScheduler.c" "Source\Research\NxResearchScheduler.c" -Force
Copy-Item "$payload\Tests\Unit\NxResearchSchedulerTests.c" "Tests\Unit\NxResearchSchedulerTests.c" -Force
Copy-Item "$payload\Tests\Benchmark\NxResearchSchedulerBench.c" "Tests\Benchmark\NxResearchSchedulerBench.c" -Force
Copy-Item "$payload\Docs\RFC\RFC-0027-Research-Scheduler.md" "Docs\RFC\RFC-0027-Research-Scheduler.md" -Force
Copy-Item "$payload\Docs\SPEC\SPEC-NRL-0012-ResearchScheduler.md" "Docs\SPEC\SPEC-NRL-0012-ResearchScheduler.md" -Force
Copy-Item "$payload\Docs\BOOK\HISTORY-Sprint-004-NRL-0012.md" "Docs\BOOK\HISTORY-Sprint-004-NRL-0012.md" -Force

Step "Patch CMakeLists.txt safely"
$cmakePath = Join-Path $root "CMakeLists.txt"
if (-not (Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found"
}

$backupPath = Join-Path $root "CMakeLists.txt.nrl0012.bak"
if (-not (Test-Path $backupPath)) {
    Copy-Item $cmakePath $backupPath
}

$content = Get-Content $cmakePath -Raw

# Repair common accidental one-line merges from previous script runs.
$content = $content -replace "\)\s*endif\s*\(", ")`r`nendif("
$content = $content -replace "\)\s*add_executable", ")`r`n`r`nadd_executable"

$content = Add-Line-AfterAnchor $content `
    "Source/Research/NxResearchScheduler.c" `
    @(
        "Source/Research/NxResearchGraphQuery.c",
        "Source/Research/NxResearchGraphExport.c",
        "Source/Research/NxResearchGraph.c",
        "Source/Research/NxRegistry.c"
    )

$testBlock = @"
    add_executable(NxResearchSchedulerTests
        Tests/Unit/NxResearchSchedulerTests.c
    )
    target_link_libraries(NxResearchSchedulerTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchSchedulerTests COMMAND NxResearchSchedulerTests)
"@

$content = Add-Block-BeforeSectionEnd $content "NEXIORA_BUILD_TESTS" $testBlock "NxResearchSchedulerTests"

$benchBlock = @"
    add_executable(NxResearchSchedulerBench
        Tests/Benchmark/NxResearchSchedulerBench.c
    )
    target_link_libraries(NxResearchSchedulerBench PRIVATE NexioraNCP)
"@

$content = Add-Block-BeforeSectionEnd $content "NEXIORA_ENABLE_BENCHMARKS" $benchBlock "NxResearchSchedulerBench"

Set-Content -Path $cmakePath -Value $content -Encoding UTF8

Write-Host "NRL-0012 applied." -ForegroundColor Green
