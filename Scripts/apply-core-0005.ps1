$ErrorActionPreference = 'Stop'
$root = (Get-Location).Path
$payload = Join-Path $root '_core0005_payload'
if (!(Test-Path $payload)) { throw "Payload not found: $payload" }
$src = Join-Path $payload 'Source\NxBootstrap.c'
$dst = Join-Path $root 'Source\NxBootstrap.c'
if (!(Test-Path $src)) { throw "Missing source: $src" }
Copy-Item -Path $src -Destination $dst -Force
Write-Host 'CORE-0005 applied: pregunta-auto / auto integrated into nexiora.exe.'
