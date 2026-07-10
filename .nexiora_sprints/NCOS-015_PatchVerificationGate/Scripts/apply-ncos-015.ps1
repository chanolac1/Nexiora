$ErrorActionPreference = 'Stop'
function Find-RepoRoot {
    $p = (Resolve-Path '.').Path
    while ($true) {
        if (Test-Path (Join-Path $p 'CMakePresets.json')) { return $p }
        $parent = Split-Path $p -Parent
        if ($parent -eq $p) { throw 'No se encontro CMakePresets.json para ubicar la raiz del repo.' }
        $p = $parent
    }
}
$root = Find-RepoRoot
$pkg = Split-Path $PSScriptRoot -Parent
if ((Split-Path $pkg -Leaf) -eq 'Scripts') { $pkg = Split-Path $pkg -Parent }
Write-Host "Repo root: $root"
$files = @(
    'Include/Nexiora/NCOS/NxPatchVerificationGate.h',
    'Source/NCOS/NxPatchVerificationGate.c',
    'Tools/nexiora_patch_verify.c',
    'Tests/Unit/NxPatchVerificationGateTests.c'
)
foreach ($rel in $files) {
    $src = Join-Path $pkg $rel
    if (!(Test-Path $src)) { $src = Join-Path $root $rel }
    if (!(Test-Path $src)) { throw "Falta archivo requerido: $rel" }
    $dst = Join-Path $root $rel
    New-Item -ItemType Directory -Force -Path (Split-Path $dst -Parent) | Out-Null
    if ((Resolve-Path $src).Path -ne (Resolve-Path (Split-Path $dst -Parent)).Path) {
        Copy-Item $src $dst -Force
    }
}
$cmakePath = Join-Path $root 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw
# Repair accidental concatenations around previous modules if present.
$cmake = $cmake -replace 'NxAutoFixProposalEngine\.cSource/', "NxAutoFixProposalEngine.c`n    Source/"
$cmake = $cmake -replace 'NxPatchVerificationGate\.cSource/', "NxPatchVerificationGate.c`n    Source/"
if (!$cmake.Contains('Source/NCOS/NxPatchVerificationGate.c')) {
    $marker = 'Source/NCOS/NxSafePatchEngine.c'
    if ($cmake.Contains($marker)) { $cmake = $cmake.Replace($marker, "$marker`n    Source/NCOS/NxPatchVerificationGate.c") }
    else { $cmake = $cmake -replace '(add_library\(NexioraNCP\s*)', "`$1`n    Source/NCOS/NxPatchVerificationGate.c`n" }
}
if (!$cmake.Contains('add_executable(nexiora_patch_verify')) {
    $cmake += "`nadd_executable(nexiora_patch_verify Tools/nexiora_patch_verify.c)`n"
    $cmake += "target_link_libraries(nexiora_patch_verify PRIVATE NexioraNCP)`n"
}
if (!$cmake.Contains('add_executable(NxPatchVerificationGateTests')) {
    $cmake += "`nadd_executable(NxPatchVerificationGateTests Tests/Unit/NxPatchVerificationGateTests.c)`n"
    $cmake += "target_link_libraries(NxPatchVerificationGateTests PRIVATE NexioraNCP)`n"
    $cmake += "add_test(NAME NxPatchVerificationGateTests COMMAND NxPatchVerificationGateTests)`n"
}
Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8
Write-Host 'NCOS-015 applied.'
