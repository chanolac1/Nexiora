$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw 'No se pudo encontrar la raiz del repo con CMakePresets.json.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
Write-Host "Repo root: $repo"

function Copy-IfDifferent {
    param([string]$RelativePath)
    $src = Join-Path $repo $RelativePath
    $dst = Join-Path $repo $RelativePath
    if (!(Test-Path $src)) {
        Write-Warning "No existe $RelativePath; se asume que el archivo ya fue aplicado o el paquete original no esta completo."
        return
    }
    $srcResolved = (Resolve-Path $src).Path
    $dstResolved = $srcResolved
    if ($srcResolved -eq $dstResolved) {
        Write-Host "Skip self-copy: $RelativePath"
        return
    }
    Copy-Item $srcResolved $dstResolved -Force
}

$expected = @(
    'Include/Nexiora/NCOS/NxPatchVerificationGate.h',
    'Source/NCOS/NxPatchVerificationGate.c',
    'Tools/nexiora_patch_verify.c',
    'Tests/Unit/NxPatchVerificationGateTests.c'
)

foreach ($f in $expected) {
    $p = Join-Path $repo $f
    if (Test-Path $p) { Write-Host "OK: $f" } else { Write-Warning "Falta $f" }
}

$cmakePath = Join-Path $repo 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw
$changed = $false

# Remove accidental duplicate bad concatenations if any.
$cmake = $cmake -replace 'Source/NCOS/NxPatchVerificationGate\.cSource/', "Source/NCOS/NxPatchVerificationGate.c`n    Source/"

if ($cmake -notlike '*Source/NCOS/NxPatchVerificationGate.c*') {
    $needle = 'Source/NCOS/NxApprovedPatchApplyEngine.c'
    if ($cmake.Contains($needle)) {
        $cmake = $cmake.Replace($needle, "$needle`n    Source/NCOS/NxPatchVerificationGate.c")
    } else {
        $cmake = $cmake -replace '(add_library\(NexioraNCP\s*)', "`$1`n    Source/NCOS/NxPatchVerificationGate.c`n"
    }
    $changed = $true
    Write-Host 'Agregado NxPatchVerificationGate.c a NexioraNCP.'
}

if ($cmake -notlike '*add_executable(nexiora_patch_verify*') {
    $cmake += @'

add_executable(nexiora_patch_verify
    Tools/nexiora_patch_verify.c
)
target_link_libraries(nexiora_patch_verify PRIVATE NexioraNCP)

add_executable(NxPatchVerificationGateTests
    Tests/Unit/NxPatchVerificationGateTests.c
)
target_link_libraries(NxPatchVerificationGateTests PRIVATE NexioraNCP)
add_test(NAME NxPatchVerificationGateTests COMMAND NxPatchVerificationGateTests)
'@
    $changed = $true
    Write-Host 'Agregada integracion NCOS-015 a CMakeLists.txt.'
}

if ($changed) { Set-Content -Path $cmakePath -Value $cmake -Encoding UTF8 }

Write-Host 'NCOS-015 repair applied.'
