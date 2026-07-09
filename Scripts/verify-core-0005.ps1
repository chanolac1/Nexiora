$ErrorActionPreference = 'Stop'
Write-Host '== CORE-0005: applying =='
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-core-0005.ps1
Write-Host '== CORE-0005: configure =='
cmake --preset windows-msvc-release
Write-Host '== CORE-0005: build =='
cmake --build --preset release
Write-Host '== CORE-0005: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
Write-Host '== CORE-0005: auto question =='
$out = .\Build\windows-msvc-release\bin\nexiora.exe pregunta-auto Genexus "Para que sirve una Knowledge Base?"
$out | Tee-Object -FilePath core-0005-auto-question.log
if ($out -notmatch 'Pregunta automatica') { throw 'No se ejecuto el modo pregunta-auto.' }
if ($out -notmatch 'Respuesta generada|Memoria suficiente') { throw 'No se genero respuesta en modo auto.' }
if ($out -notmatch 'Knowledge Base|Repositorio central|base de conocimiento') { throw 'La respuesta no fue especifica para Knowledge Base.' }
Write-Host '== CORE-0005 complete =='
