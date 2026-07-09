$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$pkg = Join-Path $root ".nexiora_packages\NCOS-002-REPAIR"

$files = @(
    @{Src="Source\NCOS\NxPlanningEngine.c"; Dst="Source\NCOS\NxPlanningEngine.c"},
    @{Src="Tests\Unit\NxPlanningEngineTests.c"; Dst="Tests\Unit\NxPlanningEngineTests.c"}
)

foreach ($f in $files) {
    $src = Join-Path $pkg $f.Src
    $dst = Join-Path $root $f.Dst
    if (!(Test-Path $src)) { throw "No se encontro archivo de repair: $src" }
    $dstDir = Split-Path $dst -Parent
    if (!(Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }
    if ((Resolve-Path $src).Path -ne (Resolve-Path $dst -ErrorAction SilentlyContinue).Path) {
        Copy-Item -Path $src -Destination $dst -Force
    }
}

Write-Host "NCOS-002 repair applied."
