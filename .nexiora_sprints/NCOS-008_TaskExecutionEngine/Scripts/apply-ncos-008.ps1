$ErrorActionPreference = "Stop"

function Find-RepoRoot([string]$start) {
    $dir = (Resolve-Path $start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir "CMakePresets.json")) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) {
            throw "No se encontro la raiz del repositorio Nexiora desde $start"
        }
        $dir = $parent
    }
}

$root = Find-RepoRoot $PSScriptRoot
$payload = Join-Path $root ".nexiora_sprints\NCOS-008_TaskExecutionEngine"

$dirs = @(
    "Include\Nexiora\NCOS",
    "Source\NCOS",
    "Tools\NCOS",
    "Tests\Unit",
    "Scripts"
)
foreach ($d in $dirs) {
    New-Item -ItemType Directory -Force -Path (Join-Path $root $d) | Out-Null
}

$copies = @(
    @("Include\Nexiora\NCOS\NxTaskExecutionEngine.h", "Include\Nexiora\NCOS\NxTaskExecutionEngine.h"),
    @("Source\NCOS\NxTaskExecutionEngine.c", "Source\NCOS\NxTaskExecutionEngine.c"),
    @("Tools\NCOS\NxTaskTool.c", "Tools\NCOS\NxTaskTool.c"),
    @("Tests\Unit\NxTaskExecutionEngineTests.c", "Tests\Unit\NxTaskExecutionEngineTests.c")
)
foreach ($pair in $copies) {
    $src = Join-Path $payload $pair[0]
    $dst = Join-Path $root $pair[1]
    if (!(Test-Path $src)) { continue }
    $srcPath = (Resolve-Path -LiteralPath $src).Path
    $dstPath = $null
    if (Test-Path $dst) { $dstPath = (Resolve-Path -LiteralPath $dst).Path }
    if ($dstPath -and ($srcPath -ieq $dstPath)) { continue }
    Copy-Item -Path $src -Destination $dst -Force
}

$cmakePath = Join-Path $root "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
if ($cmake -notmatch "NCOS-008") {
    $block = @'

# ============================================================
# NCOS-008 — Task Execution Engine
# ============================================================
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxTaskExecutionEngine.c
)

add_executable(nexiora_task
    Tools/NCOS/NxTaskTool.c
)
target_link_libraries(nexiora_task PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxTaskExecutionEngineTests
        Tests/Unit/NxTaskExecutionEngineTests.c
    )
    target_link_libraries(NxTaskExecutionEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxTaskExecutionEngineTests COMMAND NxTaskExecutionEngineTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
}

Write-Host "NCOS-008 applied."
