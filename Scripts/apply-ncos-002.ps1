$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$payload = Join-Path $root ".nexiora_packages\NCOS-002"

if (!(Test-Path $payload)) {
    throw "No se encontro payload NCOS-002: $payload"
}

$items = @(
    "Include\Nexiora\NCOS\NxPlanningEngine.h",
    "Source\NCOS\NxPlanningEngine.c",
    "Tools\NCOS\NxPlanningTool.c",
    "Tests\Unit\NxPlanningEngineTests.c"
)

foreach ($rel in $items) {
    $src = Join-Path $payload $rel
    $dst = Join-Path $root $rel
    $dstDir = Split-Path $dst -Parent
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }
    if ((Resolve-Path $src).Path -ne (Resolve-Path $dst -ErrorAction SilentlyContinue).Path) {
        Copy-Item -Path $src -Destination $dst -Force
    }
}

$cmakePath = Join-Path $root "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
if ($cmake -notmatch "NCOS-002") {
    $block = @'

# ============================================================
# NCOS-002 — Planning Engine
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxPlanningEngine.c
)

add_executable(nexiora_plan
    Tools/NCOS/NxPlanningTool.c
)
target_link_libraries(nexiora_plan PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxPlanningEngineTests
        Tests/Unit/NxPlanningEngineTests.c
    )
    target_link_libraries(NxPlanningEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxPlanningEngineTests COMMAND NxPlanningEngineTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
}

Write-Host "NCOS-002 applied."
