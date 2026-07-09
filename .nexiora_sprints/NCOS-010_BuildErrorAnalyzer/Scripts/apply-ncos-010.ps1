$ErrorActionPreference = 'Stop'
function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $p = Split-Path $dir -Parent
        if ($p -eq $dir) { throw 'No se encontro raiz del repo.' }
        $dir = $p
    }
}
$repo = Find-RepoRoot $PSScriptRoot
$sprint = Join-Path $repo '.nexiora_sprints\NCOS-010_BuildErrorAnalyzer'

$copies = @(
    @('Include\Nexiora\NCOS\NxBuildLogAnalyzer.h','Include\Nexiora\NCOS\NxBuildLogAnalyzer.h'),
    @('Source\NCOS\NxBuildLogAnalyzer.c','Source\NCOS\NxBuildLogAnalyzer.c'),
    @('Tools\NCOS\NxBuildLogAnalyzerTool.c','Tools\NCOS\NxBuildLogAnalyzerTool.c'),
    @('Tests\Unit\NxBuildLogAnalyzerTests.c','Tests\Unit\NxBuildLogAnalyzerTests.c')
)
foreach ($pair in $copies) {
    $src = Join-Path $sprint $pair[0]
    $dst = Join-Path $repo $pair[1]
    if (Test-Path $src) {
        New-Item -ItemType Directory -Force -Path (Split-Path $dst -Parent) | Out-Null
        $same = $false
        if (Test-Path $dst) {
            $same = ((Resolve-Path $src).Path -eq (Resolve-Path $dst).Path)
        }
        if (-not $same) { Copy-Item $src $dst -Force }
    }
}

$readmeSrc = Join-Path $sprint 'README_NCOS-010.md'
if (Test-Path $readmeSrc) {
    Copy-Item $readmeSrc (Join-Path $repo 'README_NCOS-010.md') -Force
}

$cmakePath = Join-Path $repo 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw

if ($cmake -notmatch [regex]::Escape('Source/NCOS/NxBuildLogAnalyzer.c')) {
    $anchor = 'Source/NCOS/NxCompilerEngine.c'
    if ($cmake.Contains($anchor)) {
        $cmake = $cmake.Replace($anchor, "$anchor`n    Source/NCOS/NxBuildLogAnalyzer.c")
    } else {
        $fallback = 'Source/Research/NxKnowledgeAcquisition.c'
        $cmake = $cmake.Replace($fallback, "$fallback`n    Source/NCOS/NxBuildLogAnalyzer.c")
    }
}

if ($cmake -notmatch [regex]::Escape('add_executable(nexiora_analyze_log')) {
    $block = @'

# ============================================================
# NCOS-010 — Build Error Analyzer
# ============================================================

add_executable(nexiora_analyze_log
    Tools/NCOS/NxBuildLogAnalyzerTool.c
)
target_link_libraries(nexiora_analyze_log PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxBuildLogAnalyzerTests
        Tests/Unit/NxBuildLogAnalyzerTests.c
    )
    target_link_libraries(NxBuildLogAnalyzerTests PRIVATE NexioraNCP)
    add_test(NAME NxBuildLogAnalyzerTests COMMAND NxBuildLogAnalyzerTests)
endif()
'@
    $cmake = $cmake.TrimEnd() + $block + "`n"
}

Set-Content $cmakePath $cmake -Encoding UTF8
Write-Host 'NCOS-010 applied.'
