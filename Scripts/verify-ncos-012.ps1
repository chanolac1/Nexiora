$ErrorActionPreference = "Stop"

function Find-RepoRoot {
    param([string]$Start)
    $current = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $current "CMakePresets.json")) { return $current }
        $parent = Split-Path $current -Parent
        if ($parent -eq $current -or [string]::IsNullOrWhiteSpace($parent)) { throw "No se encontro CMakePresets.json desde $Start" }
        $current = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
Set-Location $repo
Write-Host "== NCOS-012: repo =="
Write-Host "Repo root: $repo"

$required = @(
    "Include/Nexiora/NCOS/NxSafePatchEngine.h",
    "Source/NCOS/NxSafePatchEngine.c",
    "Tools/nexiora_safe_patch.c",
    "Tests/Unit/NxSafePatchEngineTests.c"
)
foreach ($file in $required) {
    if (!(Test-Path (Join-Path $repo $file))) { throw "Falta archivo requerido: $file. Revisa que el ZIP se haya descomprimido sobre la raiz del repo." }
}

Write-Host "== NCOS-012: cmake integration =="
$cmakePath = Join-Path $repo "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
$marker = "# NCOS-012 Safe Patch Engine"
if ($cmake -notlike "*$marker*") {
    $block = @'

# NCOS-012 Safe Patch Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxSafePatchEngine.c
)

add_executable(nexiora_safe_patch Tools/nexiora_safe_patch.c)
target_link_libraries(nexiora_safe_patch PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxSafePatchEngineTests Tests/Unit/NxSafePatchEngineTests.c)
    target_link_libraries(NxSafePatchEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxSafePatchEngineTests COMMAND NxSafePatchEngineTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host "NCOS-012 agregado a CMakeLists.txt"
} else {
    Write-Host "NCOS-012 ya estaba integrado en CMakeLists.txt"
}

Write-Host "== NCOS-012: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-012: build =="
cmake --build --preset release

Write-Host "== NCOS-012: focused test =="
ctest --test-dir .\Build\windows-msvc-release -R NxSafePatchEngineTests --output-on-failure

Write-Host "== NCOS-012: full tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-012: smoke safe patch =="
$sampleDir = Join-Path $repo "Knowledge\NCOS\BuildAnalysisSamples"
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null
$sampleLog = Join-Path $sampleDir "sample-safe-patch.analysis.md"
@'
# Build Analysis
FAILED: CMakeFiles/demo.obj
D:/Nexiora/Source/demo.c:10:5: error: expected ';' before 'return'
D:/Nexiora/Source/demo.c:11:1: warning: unused variable 'x'
The following tests FAILED:
  12 - DemoTests (Failed)
'@ | Set-Content -Path $sampleLog -Encoding UTF8

$exe = Join-Path $repo "Build\windows-msvc-release\bin\nexiora_safe_patch.exe"
if (!(Test-Path $exe)) { throw "No existe nexiora_safe_patch.exe" }
$out = & $exe propose safe_patch_demo $sampleLog
$out | Write-Host

$proposal = Join-Path $repo "Knowledge\NCOS\PatchProposals\safe_patch_demo.patch.md"
if (!(Test-Path $proposal)) { throw "No se genero propuesta de parche seguro: $proposal" }
$content = Get-Content $proposal -Raw
if ($content -notlike "*NCOS Safe Patch Proposal*") { throw "La propuesta no tiene encabezado esperado." }
if ($content -notlike "*waiting_for_human_approval*") { throw "La propuesta no exige aprobacion humana." }
if ($content -notlike "*missing semicolon*") { throw "La propuesta no detecto la accion de sintaxis esperada." }
if ($out -join "`n" -notlike "*Safe Patch Engine*") { throw "No se ejecuto Safe Patch Engine." }

Write-Host "== NCOS-012 complete =="
