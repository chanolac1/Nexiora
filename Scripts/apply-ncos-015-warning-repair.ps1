$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    $dir = (Get-Location).Path
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ($parent -eq $dir -or [string]::IsNullOrWhiteSpace($parent)) { throw 'No se encontro CMakePresets.json.' }
        $dir = $parent
    }
}

$root = Find-RepoRoot
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkgRoot = Split-Path -Parent $scriptDir
$src = Join-Path $pkgRoot 'Source\NCOS\NxPatchVerificationGate.c'
$dst = Join-Path $root 'Source\NCOS\NxPatchVerificationGate.c'

if (!(Test-Path $src)) { throw "No existe source corregido: $src" }
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dst) | Out-Null
Copy-Item -Path $src -Destination $dst -Force
Write-Host 'NCOS-015 warning repair applied: NxPatchVerificationGate.c reemplazado con implementacion limpia.'
