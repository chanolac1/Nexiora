$ErrorActionPreference = 'Stop'

function Find-RepoRoot {
    param([string]$Start)
    $dir = Resolve-Path $Start
    while ($true) {
        if (Test-Path (Join-Path $dir 'CMakePresets.json')) { return $dir }
        $parent = Split-Path $dir -Parent
        if ([string]::IsNullOrWhiteSpace($parent) -or $parent -eq $dir) {
            throw "No se encontro CMakePresets.json desde $Start"
        }
        $dir = $parent
    }
}

$root = Find-RepoRoot (Split-Path $PSScriptRoot -Parent)
Write-Host "Repo root: $root"

$testPath = Join-Path $root 'Tests\Unit\NxBuildLogAnalyzerTests.c'
if (!(Test-Path $testPath)) {
    throw "No existe $testPath. Primero aplica NCOS-010_BuildErrorAnalyzer.zip."
}

$text = Get-Content $testPath -Raw
if ($text -match 'system\s*\(' -and $text -notmatch '#include\s+<stdlib\.h>') {
    $text = $text -replace '(#include\s+<stdio\.h>\s*)', "`$1`n#include <stdlib.h>`n"
    Set-Content -Path $testPath -Value $text -Encoding UTF8
    Write-Host 'Agregado #include <stdlib.h> a NxBuildLogAnalyzerTests.c'
} else {
    Write-Host 'NxBuildLogAnalyzerTests.c ya esta correcto o no usa system().'
}

Write-Host 'NCOS-010 repair3 applied.'
