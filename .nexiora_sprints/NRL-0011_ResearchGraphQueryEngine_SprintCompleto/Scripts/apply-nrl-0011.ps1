$ErrorActionPreference = "Stop"

function Step($message) {
    Write-Host ""
    Write-Host "==> $message" -ForegroundColor Cyan
}

function Copy-File($source, $target) {
    $dir = Split-Path $target -Parent
    if (!(Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
    }
    Copy-Item $source $target -Force
}

function Add-UniqueLineAfter($text, $anchor, $line) {
    if ($text.Contains($line)) { return $text }
    if ($text.Contains($anchor)) {
        return $text.Replace($anchor, "$anchor`r`n    $line")
    }
    return $text
}

function Patch-CMakeLists {
    $cmakePath = "CMakeLists.txt"
    if (!(Test-Path $cmakePath)) {
        throw "CMakeLists.txt not found. Run this script from the Nexiora root directory."
    }

    $backupPath = "CMakeLists.txt.nrl0011.bak"
    if (!(Test-Path $backupPath)) {
        Copy-Item $cmakePath $backupPath -Force
    }

    $text = Get-Content $cmakePath -Raw

    # Normalize accidental glued endif cases from previous patch attempts.
    $text = $text -replace "\)endif\(\)", ")`r`nendif()"
    $text = $text -replace "\)\s*endif\(\)", ")`r`nendif()"

    $sourceLine = "Source/Research/NxResearchGraphQuery.c"
    if (!$text.Contains($sourceLine)) {
        if ($text.Contains("Source/Research/NxResearchGraphExporter.c")) {
            $text = Add-UniqueLineAfter $text "Source/Research/NxResearchGraphExporter.c" $sourceLine
        }
        elseif ($text.Contains("Source/Research/NxResearchGraph.c")) {
            $text = Add-UniqueLineAfter $text "Source/Research/NxResearchGraph.c" $sourceLine
        }
        else {
            throw "Unable to find a Research Graph source anchor in CMakeLists.txt."
        }
    }

    $testBlock = @"

    add_executable(NxResearchGraphQueryTests
        Tests/Unit/NxResearchGraphQueryTests.c
    )
    target_link_libraries(NxResearchGraphQueryTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchGraphQueryTests COMMAND NxResearchGraphQueryTests)
"@

    if (!$text.Contains("NxResearchGraphQueryTests")) {
        $lastEndif = $text.LastIndexOf("endif()")
        if ($lastEndif -lt 0) {
            throw "Unable to find final endif() for test insertion."
        }
        $text = $text.Insert($lastEndif, $testBlock + "`r`n")
    }

    Set-Content -Path $cmakePath -Value $text -NoNewline
}

$root = (Get-Location).Path
$sprintRoot = Join-Path $root ".nexiora_sprints\NRL-0011_ResearchGraphQueryEngine_SprintCompleto"

if (!(Test-Path $sprintRoot)) {
    $scriptPath = $MyInvocation.MyCommand.Path
    $candidate = Split-Path (Split-Path $scriptPath -Parent) -Parent
    if (Test-Path (Join-Path $candidate "Include\Nexiora\Research\NxResearchGraphQuery.h")) {
        $sprintRoot = $candidate
    }
    else {
        throw "Sprint payload not found. Expected .nexiora_sprints\NRL-0011_ResearchGraphQueryEngine_SprintCompleto or direct extracted sprint folder."
    }
}

Step "Copy NRL-0011 files"
Copy-File (Join-Path $sprintRoot "Include\Nexiora\Research\NxResearchGraphQuery.h") "Include\Nexiora\Research\NxResearchGraphQuery.h"
Copy-File (Join-Path $sprintRoot "Source\Research\NxResearchGraphQuery.c") "Source\Research\NxResearchGraphQuery.c"
Copy-File (Join-Path $sprintRoot "Tests\Unit\NxResearchGraphQueryTests.c") "Tests\Unit\NxResearchGraphQueryTests.c"
Copy-File (Join-Path $sprintRoot "Docs\RFC\RFC-0026-Research-Graph-Query-Engine.md") "RFC\RFC-0026-Research-Graph-Query-Engine.md"
Copy-File (Join-Path $sprintRoot "Docs\SPEC\SPEC-NRL-0011-ResearchGraphQueryEngine.md") "SPEC\SPEC-NRL-0011-ResearchGraphQueryEngine.md"
Copy-File (Join-Path $sprintRoot "Docs\BOOK\HISTORY-Sprint-003-NRL-0011.md") "BOOK\HISTORY\Sprint-003-NRL-0011.md"

Step "Patch CMakeLists.txt"
Patch-CMakeLists

Write-Host ""
Write-Host "NRL-0011 applied successfully." -ForegroundColor Green
