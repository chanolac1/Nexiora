$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path -Parent $dir
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $dir) { throw 'No se encontro CMakePresets.json para ubicar la raiz del repo.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot -Start $PSScriptRoot
$pkg = Join-Path $repo '.ncos_packages\NCOS-016_NexioraPackageManager'
$payload = Join-Path $pkg 'Payload'

if (!(Test-Path $payload)) { throw "No existe payload NCOS-016: $payload" }

$files = @(
    'Include/Nexiora/NCOS/NxPackageManager.h',
    'Source/NCOS/NxPackageManager.c',
    'Tools/NCOS/NxPackageTool.c',
    'Tests/Unit/NxPackageManagerTests.c'
)

foreach ($rel in $files) {
    $src = Join-Path $payload $rel
    $dst = Join-Path $repo $rel
    if (!(Test-Path $src)) { throw "Falta payload: $src" }
    $dstDir = Split-Path -Parent $dst
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }

    $srcResolved = (Resolve-Path $src).Path
    $dstResolved = if (Test-Path $dst) { (Resolve-Path $dst).Path } else { $dst }
    if ($srcResolved -eq $dstResolved) {
        Write-Host "SKIP self-copy: $rel"
        continue
    }

    if (Test-Path $dst) {
        $srcHash = (Get-FileHash -Algorithm SHA256 -Path $src).Hash
        $dstHash = (Get-FileHash -Algorithm SHA256 -Path $dst).Hash
        if ($srcHash -eq $dstHash) {
            Write-Host "OK identical: $rel"
            continue
        }
    }

    Copy-Item -Path $src -Destination $dst -Force
    Write-Host "UPDATED: $rel"
}

$cmakePath = Join-Path $repo 'CMakeLists.txt'
$cmake = Get-Content $cmakePath -Raw
if (!$cmake.Contains('NCOS-016 - Nexiora Package Manager')) {
    $block = @'

# NCOS-016 - Nexiora Package Manager
target_sources(NexioraNCP PRIVATE
    Source/NCOS/NxPackageManager.c
)

add_executable(nexiora_package
    Tools/NCOS/NxPackageTool.c
)
target_link_libraries(nexiora_package PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    add_executable(NxPackageManagerTests
        Tests/Unit/NxPackageManagerTests.c
    )
    target_link_libraries(NxPackageManagerTests PRIVATE NexioraNCP)
    add_test(NAME NxPackageManagerTests COMMAND NxPackageManagerTests)
endif()
'@
    Add-Content -Path $cmakePath -Value $block
    Write-Host 'Agregada integracion NCOS-016 a CMakeLists.txt.'
} else {
    Write-Host 'CMakeLists.txt ya contiene NCOS-016.'
}

Write-Host 'NCOS-016 applied.'
