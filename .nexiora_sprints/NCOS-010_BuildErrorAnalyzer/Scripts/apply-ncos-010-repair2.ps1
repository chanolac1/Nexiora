$ErrorActionPreference = 'Stop'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repo = Resolve-Path (Join-Path $scriptDir "..\..\..")
if (!(Test-Path (Join-Path $repo "CMakePresets.json"))) {
    throw "No se encontro CMakePresets.json. Ejecuta desde el repositorio Nexiora."
}

$src = Join-Path $repo ".nexiora_sprints\NCOS-010_BuildErrorAnalyzer\Tests\Unit\NxBuildLogAnalyzerTests.c"
$dstDir = Join-Path $repo "Tests\Unit"
$dst = Join-Path $dstDir "NxBuildLogAnalyzerTests.c"
New-Item -ItemType Directory -Force -Path $dstDir | Out-Null
if ((Resolve-Path $src).Path -ne (Resolve-Path $dst -ErrorAction SilentlyContinue).Path) {
    Copy-Item -Path $src -Destination $dst -Force
}
Write-Host "NCOS-010 repair2 applied: NxBuildLogAnalyzerTests.c fixed."
