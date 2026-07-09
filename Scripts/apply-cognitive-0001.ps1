$ErrorActionPreference = "Stop"

$root = (Get-Location).Path
$cmakePath = Join-Path $root "CMakeLists.txt"
if (!(Test-Path $cmakePath)) {
    throw "Ejecuta este script desde la raiz de Nexiora."
}

$cmake = Get-Content $cmakePath -Raw
if ($cmake -notmatch "COGNITIVE-0001") {
    $block = @'

# ============================================================
# COGNITIVE-0001 — Cognitive Core: local file ingestion + QA
# ============================================================

target_sources(NexioraNCP PRIVATE
    Source/Cognitive/NxCognitiveCore.c
)

add_executable(nexiora_cognitive
    Tools/Cognitive/NxCognitiveTool.c
)
target_link_libraries(nexiora_cognitive PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxCognitiveCoreTests
        Tests/Unit/NxCognitiveCoreTests.c
    )
    target_link_libraries(NxCognitiveCoreTests PRIVATE NexioraNCP)
    add_test(NAME NxCognitiveCoreTests COMMAND NxCognitiveCoreTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host "COGNITIVE-0001 agregado a CMakeLists.txt"
} else {
    Write-Host "COGNITIVE-0001 ya estaba registrado en CMakeLists.txt"
}
