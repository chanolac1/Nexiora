$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$pkg = Join-Path $root ".ncos_packages\NCOS-006"

function Copy-Changed($srcRel, $dstRel) {
    $src = Join-Path $pkg $srcRel
    $dst = Join-Path $root $dstRel
    $dstDir = Split-Path $dst -Parent
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Path $dstDir -Force | Out-Null }
    if ((Resolve-Path -LiteralPath $src).Path -eq (Resolve-Path -LiteralPath $dst -ErrorAction SilentlyContinue).Path) { return }
    Copy-Item -LiteralPath $src -Destination $dst -Force
}

Copy-Changed "Include\Nexiora\NCOS\NxAnswerComposer.h" "Include\Nexiora\NCOS\NxAnswerComposer.h"
Copy-Changed "Source\NCOS\NxAnswerComposer.c" "Source\NCOS\NxAnswerComposer.c"
Copy-Changed "Source\Tools\nexiora_answer.c" "Source\Tools\nexiora_answer.c"
Copy-Changed "Tests\Unit\NxAnswerComposerTests.c" "Tests\Unit\NxAnswerComposerTests.c"

$cmakePath = Join-Path $root "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
if ($cmake -notmatch "NxAnswerComposer") {
$block = @'

# NCOS-006: Answer Composer
if(TARGET NexioraNCP)
    target_sources(NexioraNCP PRIVATE Source/NCOS/NxAnswerComposer.c)
endif()

add_executable(NxAnswerComposerTests Tests/Unit/NxAnswerComposerTests.c)
target_link_libraries(NxAnswerComposerTests PRIVATE NexioraNCP)
add_test(NAME NxAnswerComposerTests COMMAND NxAnswerComposerTests)
set_target_properties(NxAnswerComposerTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(nexiora_answer Source/Tools/nexiora_answer.c)
target_link_libraries(nexiora_answer PRIVATE NexioraNCP)
set_target_properties(nexiora_answer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host "NCOS-006 CMake integration added."
} else {
    Write-Host "NCOS-006 already integrated."
}
