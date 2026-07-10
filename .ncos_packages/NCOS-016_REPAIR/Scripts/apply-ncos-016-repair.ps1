$ErrorActionPreference = "Stop"

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir "CMakePresets.json")) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw "No se encontro CMakePresets.json" }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
$pkg = Join-Path $repo ".ncos_packages\NCOS-016_REPAIR"
$src = Join-Path $pkg "Payload\Tests\Unit\NxPackageManagerTests.c"
$dst = Join-Path $repo "Tests\Unit\NxPackageManagerTests.c"

if (!(Test-Path $src)) { throw "Falta archivo fuente de repair: $src" }

$srcResolved = (Resolve-Path $src).Path
$dstResolved = if (Test-Path $dst) { (Resolve-Path $dst).Path } else { $dst }

if ($srcResolved -ne $dstResolved) {
    Copy-Item -Path $src -Destination $dst -Force
    Write-Host "UPDATED: Tests/Unit/NxPackageManagerTests.c"
} else {
    Write-Host "SKIP: Tests/Unit/NxPackageManagerTests.c ya esta en destino"
}

Write-Host "NCOS-016 repair applied."
