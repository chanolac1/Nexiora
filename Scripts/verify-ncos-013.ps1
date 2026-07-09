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
Write-Host "== NCOS-013: repo =="
Write-Host "Repo root: $repo"

$required = @(
    "Include/Nexiora/NCOS/NxPatchApprovalEngine.h",
    "Source/NCOS/NxPatchApprovalEngine.c",
    "Tools/nexiora_patch_approval.c",
    "Tests/Unit/NxPatchApprovalEngineTests.c"
)
foreach ($file in $required) {
    if (!(Test-Path (Join-Path $repo $file))) { throw "Falta archivo requerido: $file. Revisa que el ZIP se haya descomprimido sobre la raiz del repo." }
}

Write-Host "== NCOS-013: cmake integration =="
$cmakePath = Join-Path $repo "CMakeLists.txt"
$cmake = Get-Content $cmakePath -Raw
$marker = "# NCOS-013 Patch Approval Engine"
if ($cmake -notlike "*$marker*") {
    $block = @'

# NCOS-013 Patch Approval Engine
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxPatchApprovalEngine.c
)

add_executable(nexiora_patch_approval Tools/nexiora_patch_approval.c)
target_link_libraries(nexiora_patch_approval PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxPatchApprovalEngineTests Tests/Unit/NxPatchApprovalEngineTests.c)
    target_link_libraries(NxPatchApprovalEngineTests PRIVATE NexioraNCP)
    add_test(NAME NxPatchApprovalEngineTests COMMAND NxPatchApprovalEngineTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host "NCOS-013 agregado a CMakeLists.txt"
} else {
    Write-Host "NCOS-013 ya estaba integrado en CMakeLists.txt"
}

Write-Host "== NCOS-013: configure =="
cmake --preset windows-msvc-release

Write-Host "== NCOS-013: build =="
cmake --build --preset release

Write-Host "== NCOS-013: focused test =="
ctest --test-dir .\Build\windows-msvc-release -R NxPatchApprovalEngineTests --output-on-failure

Write-Host "== NCOS-013: full tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== NCOS-013: smoke patch approval =="
$sampleDir = Join-Path $repo "Knowledge\NCOS\PatchApprovalSamples"
New-Item -ItemType Directory -Force -Path $sampleDir | Out-Null
$proposal = Join-Path $sampleDir "sample.patch.md"
@'
# NCOS Safe Patch Proposal
Status: waiting_for_human_approval
Action: manual review required
'@ | Set-Content -Path $proposal -Encoding UTF8

$exe = Join-Path $repo "Build\windows-msvc-release\bin\nexiora_patch_approval.exe"
if (!(Test-Path $exe)) { throw "No existe nexiora_patch_approval.exe" }

$out1 = & $exe request approval_demo $proposal
$out1 | Write-Host
$out2 = & $exe status approval_demo
$out2 | Write-Host
$out3 = & $exe approve approval_demo Jorge
$out3 | Write-Host
$out4 = & $exe status approval_demo
$out4 | Write-Host

$approval = Join-Path $repo "Knowledge\NCOS\PatchApprovals\approval_demo.approval.md"
if (!(Test-Path $approval)) { throw "No se genero registro de aprobacion: $approval" }
$content = Get-Content $approval -Raw
if ($content -notlike "*NCOS Patch Approval Record*") { throw "El registro de aprobacion no tiene encabezado esperado." }
if ($content -notlike "*Status: approved*") { throw "El registro no quedo aprobado." }
if ($content -notlike "*approved_for_application*") { throw "El registro no contiene decision de aplicacion aprobada." }
if (($out3 -join "`n") -notlike "*Patch Approval Engine*") { throw "No se ejecuto Patch Approval Engine." }

Write-Host "== NCOS-013 complete =="
