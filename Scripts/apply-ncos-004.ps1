$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$cmake = Join-Path $root "CMakeLists.txt"
$text = Get-Content $cmake -Raw

$block = @'

# ============================================================
# NCOS-004 — Concept Graph
# ============================================================

if(NOT TARGET nexiora_graph)
    target_sources(NexioraNCP PRIVATE
        Source/NCOS/NxConceptGraph.c
    )

    add_executable(nexiora_graph
        Tools/NCOS/NxConceptGraphTool.c
    )
    target_link_libraries(nexiora_graph PRIVATE NexioraNCP)

    if(NEXIORA_BUILD_TESTS)
        add_executable(NxConceptGraphTests
            Tests/Unit/NxConceptGraphTests.c
        )
        target_link_libraries(NxConceptGraphTests PRIVATE NexioraNCP)
        add_test(NAME NxConceptGraphTests COMMAND NxConceptGraphTests)
        set_tests_properties(NxConceptGraphTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()
'@

if ($text -notmatch "NCOS-004") {
    Add-Content -Path $cmake -Value $block
    Write-Host "NCOS-004 CMake integration added."
} else {
    Write-Host "NCOS-004 CMake integration already present."
}
