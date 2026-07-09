$ErrorActionPreference = "Stop"

$root = (Get-Location).Path
$cmakePath = Join-Path $root "CMakeLists.txt"

if (!(Test-Path $cmakePath)) {
    throw "No se encontro CMakeLists.txt. Ejecuta este script desde la raiz de Nexiora."
}

Write-Host "== EPIC-0010: applying =="

# Files are copied by unzip. This script only integrates CMake idempotently.
$cmake = Get-Content $cmakePath -Raw

function Add-Text-After-Once {
    param(
        [string]$Text,
        [string]$Needle,
        [string]$Insert,
        [string]$Already
    )

    if ($Text.Contains($Already)) {
        return $Text
    }

    if (!$Text.Contains($Needle)) {
        return $Text + "`r`n" + $Insert + "`r`n"
    }

    return $Text.Replace($Needle, $Needle + "`r`n" + $Insert)
}

$cmake = Add-Text-After-Once `
    -Text $cmake `
    -Needle "    Source/Research/NxConflictDetector.c" `
    -Insert "    Source/Research/NxProgressEngine.c" `
    -Already "Source/Research/NxProgressEngine.c"

$toolBlock = @'

# ============================================================
# EPIC-0010 — Research Progress Framework
# ============================================================

add_executable(nexiora_progress_demo
    Tools/Research/NxProgressDemo.c
)
target_link_libraries(nexiora_progress_demo PRIVATE NexioraNCP)
'@

if (!$cmake.Contains("nexiora_progress_demo")) {
    $cmake = $cmake + "`r`n" + $toolBlock + "`r`n"
}

$testBlock = @'

if(NEXIORA_BUILD_TESTS)
    add_executable(NxProgressEngineTests
        Tests/Unit/NxProgressEngineTests.c
    )
    target_link_libraries(NxProgressEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxProgressEngineTests COMMAND NxProgressEngineTests)
endif()
'@

if (!$cmake.Contains("NxProgressEngineTests")) {
    $cmake = $cmake + "`r`n" + $testBlock + "`r`n"
}

Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8
Write-Host "EPIC-0010 aplicado."
