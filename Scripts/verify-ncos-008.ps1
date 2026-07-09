$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $root
powershell -ExecutionPolicy Bypass -File .\.nexiora_sprints\NCOS-008_TaskExecutionEngine\Scripts\verify-ncos-008.ps1
