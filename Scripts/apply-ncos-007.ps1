$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$pkg = Join-Path $root ".ncos_packages\NCOS-007"

function Copy-Changed($srcRel, $dstRel) {
    $src = Join-Path $pkg $srcRel
    $dst = Join-Path $root $dstRel
    $dstDir = Split-Path $dst -Parent
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Path $dstDir -Force | Out-Null }
    $srcPath = (Resolve-Path -LiteralPath $src).Path
    $dstResolved = Resolve-Path -LiteralPath $dst -ErrorAction SilentlyContinue
    if ($dstResolved -and $srcPath -eq $dstResolved.Path) { return }
    Copy-Item -LiteralPath $src -Destination $dst -Force
}

Copy-Changed "Include\Nexiora\NCOS\NxIntentPlanner.h" "Include\Nexiora\NCOS\NxIntentPlanner.h"
Copy-Changed "Source\NCOS\NxIntentPlanner.c" "Source\NCOS\NxIntentPlanner.c"
Copy-Changed "Source\Tools\nexiora_intent_plan.c" "Source\Tools\nexiora_intent_plan.c"
Copy-Changed "Tests\Unit\NxIntentPlannerTests.c" "Tests\Unit\NxIntentPlannerTests.c"

$cmakePath = Join-Path $root "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
if ($cmake -notmatch "NxIntentPlanner") {
$block = @'

# NCOS-007: Intent-to-Plan Bridge
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/NCOS/NxIntentPlanner.c)
endif()

add_executable(NxIntentPlannerTests Tests/Unit/NxIntentPlannerTests.c)
target_link_libraries(NxIntentPlannerTests PRIVATE NexioraNCP)
add_test(NAME NxIntentPlannerTests COMMAND NxIntentPlannerTests)
set_target_properties(NxIntentPlannerTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(nexiora_intent_plan Source/Tools/nexiora_intent_plan.c)
target_link_libraries(nexiora_intent_plan PRIVATE NexioraNCP)
set_target_properties(nexiora_intent_plan PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host "NCOS-007 CMake integration added."
} else {
    Write-Host "NCOS-007 already integrated."
}
