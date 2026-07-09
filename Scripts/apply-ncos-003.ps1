$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$cmake = Join-Path $root "CMakeLists.txt"
if (!(Test-Path $cmake)) { throw "No se encontro CMakeLists.txt" }
$content = Get-Content $cmake -Raw
if ($content -notmatch "NCOS-003.*Concept Registry") {
$block = @'

# ============================================================
# NCOS-003 — Concept Registry
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxConceptRegistry.c
)

add_executable(nexiora_concept
    Tools/NCOS/NxConceptTool.c
)
target_link_libraries(nexiora_concept PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxConceptRegistryTests
        Tests/Unit/NxConceptRegistryTests.c
    )
    target_link_libraries(NxConceptRegistryTests PRIVATE NexioraNCP)
    add_test(NAME NxConceptRegistryTests COMMAND NxConceptRegistryTests)
endif()
'@
    Add-Content -Path $cmake -Value $block
    Write-Host "NCOS-003 CMake integration added."
} else {
    Write-Host "NCOS-003 already integrated."
}
