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
Write-Host "Repo root: $repo"

$pkg = Join-Path $repo '.ncos_packages\NCOS-016_NexioraPackageManager_REPAIR'
$src = Join-Path $pkg 'Payload\Tests\Unit\NxPackageManagerTests.c'
$dst = Join-Path $repo 'Tests\Unit\NxPackageManagerTests.c'

if (!(Test-Path $src)) { throw "Falta payload de test: $src" }
$dstDir = Split-Path -Parent $dst
if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }

$srcResolved = (Resolve-Path $src).Path
$dstResolved = if (Test-Path $dst) { (Resolve-Path $dst).Path } else { $dst }
if ($srcResolved -eq $dstResolved) {
    Write-Host 'SKIP self-copy: Tests/Unit/NxPackageManagerTests.c'
} else {
    if (Test-Path $dst) {
        $srcHash = (Get-FileHash -Algorithm SHA256 -Path $src).Hash
        $dstHash = (Get-FileHash -Algorithm SHA256 -Path $dst).Hash
        if ($srcHash -eq $dstHash) {
            Write-Host 'OK identical: Tests/Unit/NxPackageManagerTests.c'
        } else {
            Copy-Item -Path $src -Destination $dst -Force
            Write-Host 'UPDATED: Tests/Unit/NxPackageManagerTests.c'
        }
    } else {
        Copy-Item -Path $src -Destination $dst -Force
        Write-Host 'CREATED: Tests/Unit/NxPackageManagerTests.c'
    }
}

Write-Host 'NCOS-016 repair applied.'
