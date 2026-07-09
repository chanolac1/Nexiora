$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $Root

Write-Host "== EPIC-0009: applying fixed integration =="

$cmakePath = Join-Path $Root "CMakeLists.txt"
if (-not (Test-Path $cmakePath)) {
    throw "No se encontro CMakeLists.txt en $Root"
}

Copy-Item $cmakePath "$cmakePath.epic0009.bak" -Force

# Validate package files are present after unzip.
$required = @(
    "Include/Nexiora/Research/NxKnowledgeAcquisition.h",
    "Source/Research/NxKnowledgeAcquisition.c",
    "Tests/Unit/NxKnowledgeAcquisitionTests.c",
    "Tools/Knowledge/NxKnowledgeAcquireTool.c"
)
foreach ($rel in $required) {
    if (-not (Test-Path (Join-Path $Root $rel))) {
        throw "Falta archivo de EPIC-0009: $rel. Descomprime el paquete completo sobre D:\Nexiora."
    }
}

$cmake = Get-Content $cmakePath -Raw

# Add library source exactly once.
if ($cmake -notmatch [regex]::Escape("Source/Research/NxKnowledgeAcquisition.c")) {
    $pattern = '(?s)(add_library\(NexioraNCP STATIC.*?)(\r?\n\))'
    $cmake = [regex]::Replace($cmake, $pattern, {
        param($m)
        return $m.Groups[1].Value + "`r`n    Source/Research/NxKnowledgeAcquisition.c" + $m.Groups[2].Value
    }, 1)
}

# Add acquisition CLI tool exactly once.
if ($cmake -notmatch 'add_executable\(nexiora_acquire') {
    $toolBlock = @'

# ============================================================
# EPIC-0009 — Knowledge Acquisition Engine
# ============================================================

add_executable(nexiora_acquire
    Tools/Knowledge/NxKnowledgeAcquireTool.c
)
target_link_libraries(nexiora_acquire PRIVATE NexioraNCP)
'@
    $cmake += $toolBlock
}

# Add unit test exactly once, guarded by NEXIORA_BUILD_TESTS.
if ($cmake -notmatch 'add_executable\(NxKnowledgeAcquisitionTests') {
    $testBlock = @'

if(NEXIORA_BUILD_TESTS)
    add_executable(NxKnowledgeAcquisitionTests
        Tests/Unit/NxKnowledgeAcquisitionTests.c
    )
    target_link_libraries(NxKnowledgeAcquisitionTests PRIVATE NexioraNCP)
    add_test(NAME NxKnowledgeAcquisitionTests COMMAND NxKnowledgeAcquisitionTests)
endif()
'@
    $cmake += $testBlock
}

Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8

Write-Host "EPIC-0009 applied. Backup: CMakeLists.txt.epic0009.bak"
