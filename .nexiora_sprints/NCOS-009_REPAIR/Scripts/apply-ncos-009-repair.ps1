$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = (Resolve-Path $Start).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir) { throw 'No se encontro la raiz del repositorio con CMakePresets.json.' }
        $dir = $parent
    }
}

$repo = Find-RepoRoot $PSScriptRoot
$payload = Join-Path $repo '.nexiora_sprints\NCOS-009_REPAIR'

$files = @(
    @{ Src = 'Source\NCOS\NxCompilerEngine.c'; Dst = 'Source\NCOS\NxCompilerEngine.c' },
    @{ Src = 'Tests\Unit\NxCompilerEngineTests.c'; Dst = 'Tests\Unit\NxCompilerEngineTests.c' }
)

foreach ($f in $files) {
    $src = Join-Path $payload $f.Src
    $dst = Join-Path $repo $f.Dst
    if (!(Test-Path $src)) { throw "Payload faltante: $src" }
    New-Item -ItemType Directory -Force -Path (Split-Path $dst -Parent) | Out-Null
    if ((Resolve-Path $src).Path -ne (Resolve-Path (Split-Path $dst -Parent)).Path) {
        Copy-Item -Path $src -Destination $dst -Force
    }
}

Write-Host 'NCOS-009 repair applied: compiler sandbox root creation fixed.'
