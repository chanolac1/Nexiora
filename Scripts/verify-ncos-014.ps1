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
Write-Host "== NCOS-014: repo =="
Write-Host "Repo root: $repo"

$required = @(
    "Include/Nexiora/NCOS/NxApprovedPatchApplyEngine.h",
    "Source/NCOS/NxApprovedPatchApplyEngine.c",
    "Tools/nexiora_apply_approved_patch.c",
    "Tests/Unit/NxApprovedPatchApplyEngineTests.c"
)
foreach ($file in $required) {
    if (!(Test-Path (Join-Path $repo $file))) { throw "Falta archivo requerido: $file. Descomprime el ZIP sobre la raiz del repo." }
}

Write-Host "== NCOS-014: cmake integration =="
$cmakePath = Join-Path $repo "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
$marker = "# NCOS-014 Approved Patch Apply Engine"
if ($cmake -notlike "*$marker*") {
    $block = @'

# NCOS-014 Approved Patch Apply Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxApprovedPatchApplyEngine.c
)

add_executable(nexiora_apply_approved_patch Tools/nexiora_apply_approved_patch.c)
target_link_libraries(nexiora_apply_approved_patch PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxApprovedPatchApplyEngineTests Tests/Unit/NxApprovedPatchApplyEngineTests.c)
    target_link_libraries(NxApprovedPatchApplyEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxApprovedPatchApplyEngineTests COMMAND NxApprovedPatchApplyEngineTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host "NCOS-014 agregado a CMakeLists.txt"
} else {
    Write-Host "NCOS-014 ya estaba integrado en CMakeLists.txt"
}

Write-Host "== NCOS-014: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-014: build =="
cmake --build --preset release

Write-Host "== NCOS-014: focused test =="
ctest --test-dir .\Build\windows-msvc-release -R NxApprovedPatchApplyEngineTests --output-on-failure

Write-Host "== NCOS-014: full tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-014: smoke approved patch apply =="
$sampleDir = Join-Path $repo "Knowledge\NCOS\PatchApplySamples"
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null
$proposal = Join-Path $sampleDir "approved.patch.md"
@'
# Safe Patch Proposal

This is a reviewable patch proposal artifact.
'@ | Set-Content -Path $proposal -Encoding UTF8

$approvalDir = Join-Path $repo "Knowledge\NCOS\PatchApprovals"
New-Item -ItemType Directory -Force -Path $approvalDir | Out-Null
$approval = Join-Path $approvalDir "apply_demo.approval.md"
@"
# NCOS Patch Approval Record

Run ID: apply_demo
Status: approved
Reviewer: Jorge
Proposal: $proposal
Decision: approved_for_application
"@ | Set-Content -Path $approval -Encoding UTF8

$exe = Join-Path $repo "Build\windows-msvc-release\bin\nexiora_apply_approved_patch.exe"
if (!(Test-Path $exe)) { throw "No existe nexiora_apply_approved_patch.exe" }

$out = & $exe apply apply_demo
$out | Write-Host
$status = & $exe status apply_demo
$status | Write-Host

$application = Join-Path $repo "Knowledge\NCOS\AppliedPatches\apply_demo.applied.md"
if (!(Test-Path $application)) { throw "No se genero registro de aplicacion: $application" }
$content = Get-Content $application -Raw
if ($content -notlike "*Approved Patch Application Record*") { throw "El registro de aplicacion no tiene encabezado esperado." }
if ($content -notlike "*Status: staged_for_review*") { throw "El registro no quedo staged_for_review." }
if (($out -join "`n") -notlike "*staged_for_review*") { throw "El motor no reporto staged_for_review." }
if (($status -join "`n") -notlike "*staged_for_review*") { throw "El status no reporto staged_for_review." }

Write-Host "== NCOS-014 complete =="
