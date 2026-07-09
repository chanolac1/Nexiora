$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

powershell -ExecutionPolicy Bypass -File .\Scripts\apply-core-0001.ps1

Write-Host '== CORE-0001: configure =='
cmake --preset windows-msvc-release

Write-Host '== CORE-0001: build =='
cmake --build --preset release

Write-Host '== CORE-0001: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== CORE-0001: aprende Genexus =='
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus

Write-Host '== CORE-0001: que sabes Genexus =='
.\Build\windows-msvc-release\bin\nexiora.exe que sabes Genexus

if (!(Test-Path .\Knowledge\Topics\genexus\answer.txt)) {
    throw 'No se genero Knowledge\Topics\genexus\answer.txt'
}

Write-Host '== CORE-0001 complete =='
