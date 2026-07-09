$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir) { throw 'No se encontro la raiz del repositorio con CMakePresets.json.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
$sprint = Join-Path $repo '.nexiora_sprints\NCOS-009_CompilerEngine'

$copies = @(
    @('Include\Nexiora\NCOS\NxCompilerEngine.h','Include\Nexiora\NCOS\NxCompilerEngine.h'),
    @('Source\NCOS\NxCompilerEngine.c','Source\NCOS\NxCompilerEngine.c'),
    @('Tools\NCOS\NxCompilerTool.c','Tools\NCOS\NxCompilerTool.c'),
    @('Tests\Unit\NxCompilerEngineTests.c','Tests\Unit\NxCompilerEngineTests.c')
)

foreach ($pair in $copies) {
    $src = Join-Path $sprint $pair[0]
    $dst = Join-Path $repo $pair[1]
    New-Item -ItemType Directory -Force -Path (Split-Path $dst -Parent) | Out-Null
    if ((Test-Path $src) -and ((Resolve-Path $src).Path -ne (Resolve-Path (Split-Path $dst -Parent)).Path)) {
        Copy-Item -Path $src -Destination $dst -Force
    }
}

$docSrc = Join-Path $repo '.nexiora_sprints\NCOS-009_CompilerEngine\README_NCOS-009.md'
$docDst = Join-Path $repo 'README_NCOS-009.md'
if (Test-Path $docSrc) { Copy-Item $docSrc $docDst -Force }

$cmakePath = Join-Path $repo 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw

if ($cmake -notmatch [regex]::Escape('Source/NCOS/NxCompilerEngine.c')) {
    $needle = 'Source/Research/NxKnowledgeAcquisition.c'
    $cmake = $cmake.Replace($needle, "$needle`n    Source/NCOS/NxCompilerEngine.c")
}

if ($cmake -notmatch [regex]::Escape('add_executable(nexiora_compile')) {
    $block = @'

# ============================================================
# NCOS-009 — Compiler Engine
# ============================================================

add_executable(nexiora_compile
    Tools/NCOS/NxCompilerTool.c
)
target_link_libraries(nexiora_compile PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxCompilerEngineTests
        Tests/Unit/NxCompilerEngineTests.c
    )
    target_link_libraries(NxCompilerEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxCompilerEngineTests COMMAND NxCompilerEngineTests)
endif()
'@
    $cmake = $cmake.TrimEnd() + $block + "`n"
}

Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8
Write-Host 'NCOS-009 applied.'
