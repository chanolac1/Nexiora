$ErrorActionPreference = "Stop"

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($dir -and $dir.Length -gt 3) {
        if (Test-Path (Join-Path $dir "CMakePresets.json")) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir) { break }
        $dir = $parent
    }
    throw "No se encontro CMakePresets.json; ejecuta desde D:\Nexiora o dentro del repo."
}

function Ensure-Dir($path) {
    if (!(Test-Path $path)) { New-Item -ItemType Directory -Force -Path $path | Out-Null }
}

function Ensure-FileFromPackage($repo, $relative) {
    $dst = Join-Path $repo $relative
    if (Test-Path $dst) { return }
    $src = Join-Path $PSScriptRoot ("..\" + $relative)
    if (!(Test-Path $src)) { throw "Falta archivo del paquete: $src" }
    Ensure-Dir (Split-Path $dst -Parent)
    Copy-Item -Path $src -Destination $dst -Force
}

function Add-Line-Before($text, $needle, $line) {
    if ($text.Contains($line)) { return $text }
    $bad = $line + $needle
    if ($text.Contains($bad)) { return $text.Replace($bad, $line + "`r`n    " + $needle) }
    if ($text.Contains($needle)) { return $text.Replace($needle, $line + "`r`n    " + $needle) }
    throw "No se pudo insertar $line porque no se encontro $needle"
}

$repo = Find-RepoRoot
Write-Host "Repo root: $repo"

Ensure-FileFromPackage $repo "Include\Nexiora\NCOS\NxAutoFixProposalEngine.h"
Ensure-FileFromPackage $repo "Source\NCOS\NxAutoFixProposalEngine.c"
Ensure-FileFromPackage $repo "Tools\nexiora_fix_proposal.c"
Ensure-FileFromPackage $repo "Tests\Unit\NxAutoFixProposalEngineTests.c"

$cmakePath = Join-Path $repo "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw

# Fix known corruption from earlier partial repair.
$cmake = $cmake.Replace("Source/NCOS/NxAutoFixProposalEngine.cSource/Research/NxKnowledgeStore.c", "Source/NCOS/NxAutoFixProposalEngine.c`r`n    Source/Research/NxKnowledgeStore.c")
$cmake = $cmake.Replace("Source/NCOS/NxAutoFixProposal.cSource/Research/NxKnowledgeStore.c", "Source/NCOS/NxAutoFixProposalEngine.c`r`n    Source/Research/NxKnowledgeStore.c")

$cmake = Add-Line-Before $cmake "Source/Research/NxKnowledgeStore.c" "Source/NCOS/NxAutoFixProposalEngine.c"

if (!$cmake.Contains("add_executable(nexiora_fix_proposal")) {
    $cmake += "`r`n# NCOS-011 Auto-Fix Proposal Engine`r`n"
    $cmake += "add_executable(nexiora_fix_proposal Tools/nexiora_fix_proposal.c)`r`n"
    $cmake += "target_link_libraries(nexiora_fix_proposal PRIVATE NexioraNCP)`r`n"
}

if (!$cmake.Contains("add_executable(NxAutoFixProposalEngineTests")) {
    $cmake += "`r`nadd_executable(NxAutoFixProposalEngineTests Tests/Unit/NxAutoFixProposalEngineTests.c)`r`n"
    $cmake += "target_link_libraries(NxAutoFixProposalEngineTests PRIVATE NexioraNCP)`r`n"
}

if (!$cmake.Contains("add_test(NAME NxAutoFixProposalEngineTests")) {
    $cmake += "add_test(NAME NxAutoFixProposalEngineTests COMMAND NxAutoFixProposalEngineTests)`r`n"
}

Set-Content -Path $cmakePath -Value $cmake -NoNewline
Write-Host "NCOS-011 repair3 applied."
