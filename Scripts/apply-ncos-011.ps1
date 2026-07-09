$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw 'No se encontro CMakePresets.json para determinar la raiz del repositorio.' }
        $dir = $parent
    }
}

function Add-Once([string]$Path, [string]$Needle, [string]$Block) {
    if (!(Test-Path $Path)) { throw "No existe: $Path" }
    $text = Get-Content $Path -Raw
    if ($text.Contains($Needle)) {
        Write-Host "Ya integrado: $Needle"
        return
    }
    Add-Content -Path $Path -Value "`n$Block`n"
    Write-Host "Integrado: $Needle"
}

$root = Find-RepoRoot
Set-Location $root
Write-Host "Repo root: $root"

$cmake = Join-Path $root 'CMakeLists.txt'

# NCOS-011 source files should already be present after extracting the original package.
# This applicator only performs robust CMake integration and avoids fragile regex syntax.
$required = @(
    'Include/Nexiora/NCOS/NxAutoFixProposalEngine.h',
    'Source/NCOS/NxAutoFixProposalEngine.c',
    'Tools/nexiora_fix_proposal.c',
    'Tests/Unit/NxAutoFixProposalEngineTests.c'
)

foreach ($rel in $required) {
    $p = Join-Path $root $rel
    if (!(Test-Path $p)) {
        Write-Host "ADVERTENCIA: falta $rel. Si falta el build fallara; verifica que el paquete NCOS-011 original este descomprimido."
    }
}

$text = Get-Content $cmake -Raw

if (!$text.Contains('Source/NCOS/NxAutoFixProposalEngine.c')) {
    $text = $text -replace '(Source/NCOS/NxBuildLogAnalyzer\.c\s*)', "`$1`n    Source/NCOS/NxAutoFixProposalEngine.c"
    if (!$text.Contains('Source/NCOS/NxAutoFixProposalEngine.c')) {
        # Fallback: append to first library/target source block may not be possible safely, so append marker block.
        Add-Content -Path $cmake -Value "`n# NCOS-011 source note: Source/NCOS/NxAutoFixProposalEngine.c must be part of NexioraNCP in the canonical source list.`n"
    } else {
        Set-Content -Path $cmake -Value $text
        Write-Host 'Agregado Source/NCOS/NxAutoFixProposalEngine.c a CMakeLists.txt'
    }
}

$cmakeText = Get-Content $cmake -Raw
if (!$cmakeText.Contains('add_executable(nexiora_fix_proposal')) {
$block = @'

# NCOS-011 Auto-Fix Proposal Engine
add_executable(nexiora_fix_proposal Tools/nexiora_fix_proposal.c)
target_link_libraries(nexiora_fix_proposal PRIVATE NexioraNCP)
set_target_properties(nexiora_fix_proposal PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(NxAutoFixProposalEngineTests Tests/Unit/NxAutoFixProposalEngineTests.c)
target_link_libraries(NxAutoFixProposalEngineTests PRIVATE NexioraNCP)
set_target_properties(NxAutoFixProposalEngineTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(NAME NxAutoFixProposalEngineTests COMMAND NxAutoFixProposalEngineTests)
'@
    Add-Content -Path $cmake -Value $block
    Write-Host 'Agregada integracion NCOS-011 a CMakeLists.txt'
} else {
    Write-Host 'NCOS-011 ya estaba integrado en CMakeLists.txt'
}

Write-Host 'NCOS-011 repair2 applied.'
