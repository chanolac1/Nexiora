$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$patchRoot = Join-Path $root ".nexiora_patches\NCOS-005_REPAIR2"
$src = Join-Path $patchRoot "Tests\Unit\NxConceptRegistryTests.c"
$dst = Join-Path $root "Tests\Unit\NxConceptRegistryTests.c"
if (!(Test-Path $src)) { throw "Patch source not found: $src" }
Copy-Item -Path $src -Destination $dst -Force
Write-Host "NCOS-005 repair2 applied: NxConceptRegistryTests is now isolated and repeatable."
