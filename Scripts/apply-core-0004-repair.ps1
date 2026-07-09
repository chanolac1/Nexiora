$ErrorActionPreference = 'Stop'
$root = (Get-Location).Path
$payload = Join-Path $root '_core0004_repair_payload'
if (!(Test-Path $payload)) { throw "Payload not found: $payload" }
$src = Join-Path $payload 'Source\Research\NxTopicQuestion.c'
$dst = Join-Path $root 'Source\Research\NxTopicQuestion.c'
if (!(Test-Path $src)) { throw "Missing source: $src" }
Copy-Item -Path $src -Destination $dst -Force
Write-Host 'CORE-0004 repair applied: NxTopicQuestion now emits explicit Fuentes registradas and Repositorio central.'
