$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Set-Location $root

Write-Host '== CORE-0003: applying =='
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-core-0003.ps1

Write-Host '== CORE-0003: configure =='
cmake --preset windows-msvc-release

Write-Host '== CORE-0003: build =='
cmake --build --preset release

Write-Host '== CORE-0003: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host '== CORE-0003: learn Genexus =='
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus

Write-Host '== CORE-0003: ask Genexus =='
$output = .\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus 'Que es una Transaction?'
$output
if ($output -notmatch 'Respuesta desde memoria') { throw 'No se genero una respuesta desde memoria.' }
if ($output -notmatch 'Transaction') { throw 'La respuesta no uso el conocimiento esperado sobre Transaction.' }

if (!(Test-Path .\Knowledge\Topics\genexus\last_question_answer.txt)) {
    throw 'No se genero Knowledge\Topics\genexus\last_question_answer.txt'
}

Write-Host '== CORE-0003 complete =='
