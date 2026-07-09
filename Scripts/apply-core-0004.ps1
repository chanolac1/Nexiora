$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$payload = Join-Path $root '_core0004_payload'
if (!(Test-Path $payload)) { throw "No se encontro payload CORE-0004: $payload" }
$src = Join-Path $payload 'Source/Research/NxTopicQuestion.c'
$dst = Join-Path $root 'Source/Research/NxTopicQuestion.c'
if (!(Test-Path $src)) { throw "No se encontro $src" }
Copy-Item -Path $src -Destination $dst -Force
Write-Host 'CORE-0004 applied: NxTopicQuestion concept-aware QA updated.'
