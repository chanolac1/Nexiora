$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$cmake = Join-Path $root "CMakeLists.txt"
$text = Get-Content $cmake -Raw

$block = @'

# ============================================================
# NCOS-005 — Graph Reasoning
# ============================================================

if(NOT TARGET nexiora_reason)
    target_sources(NexioraNCP PRIVATE
        Source/NCOS/NxGraphReasoner.c
    )

    add_executable(nexiora_reason
        Tools/NCOS/NxGraphReasonerTool.c
    )
    target_link_libraries(nexiora_reason PRIVATE NexioraNCP)

    if(NEXIORA_BUILD_TESTS)
        add_executable(NxGraphReasonerTests
            Tests/Unit/NxGraphReasonerTests.c
        )
        target_link_libraries(NxGraphReasonerTests PRIVATE NexioraNCP)
        add_test(NAME NxGraphReasonerTests COMMAND NxGraphReasonerTests)
        set_tests_properties(NxGraphReasonerTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()
'@

if ($text -notmatch "NCOS-005") {
    Add-Content -Path $cmake -Value $block
    Write-Host "NCOS-005 CMake integration added."
} else {
    Write-Host "NCOS-005 CMake integration already present."
}
