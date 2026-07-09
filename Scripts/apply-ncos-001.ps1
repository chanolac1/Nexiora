$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$cmake = Join-Path $root 'CMakeLists.txt'
if (!(Test-Path $cmake)) { throw 'No se encontro CMakeLists.txt' }
$text = Get-Content $cmake -Raw
if ($text -notmatch 'NCOS-001') {
$block = @'

# ============================================================
# NCOS-001 — Session Engine
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxSessionEngine.c
)

add_executable(nexiora_session
    Tools/NCOS/NxSessionTool.c
)
target_link_libraries(nexiora_session PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxSessionEngineTests
        Tests/Unit/NxSessionEngineTests.c
    )
    target_link_libraries(NxSessionEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxSessionEngineTests COMMAND NxSessionEngineTests)
endif()
'@
    Add-Content -Path $cmake -Value $block
    Write-Host 'NCOS-001 CMake integration added.'
} else {
    Write-Host 'NCOS-001 already integrated.'
}
