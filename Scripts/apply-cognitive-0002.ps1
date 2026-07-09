$ErrorActionPreference = "Stop"
$root = (Get-Location).Path

$cmake = Join-Path $root "CMakeLists.txt"
if (!(Test-Path $cmake)) { throw "No se encontro CMakeLists.txt. Ejecuta desde la raiz de Nexiora." }

$content = Get-Content $cmake -Raw
if ($content -notmatch "nexiora_cognitive_batch") {
@'

# COGNITIVE-0002: Cognitive directory ingest tool
if(NOT TARGET nexiora_cognitive_batch)
    add_executable(nexiora_cognitive_batch
        Tools/Cognitive/NxCognitiveBatchTool.c
    )
    target_link_libraries(nexiora_cognitive_batch PRIVATE NexioraNCP)
    set_target_properties(nexiora_cognitive_batch PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
'@ | Add-Content $cmake
}

Write-Host "COGNITIVE-0002 applied."
