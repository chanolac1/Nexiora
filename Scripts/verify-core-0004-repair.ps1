$ErrorActionPreference = 'Stop'
Write-Host '== CORE-0004 REPAIR: applying =='
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-core-0004-repair.ps1
Write-Host '== CORE-0004 REPAIR: configure =='
cmake --preset windows-msvc-release
Write-Host '== CORE-0004 REPAIR: build =='
cmake --build --preset release
Write-Host '== CORE-0004 REPAIR: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
Write-Host '== CORE-0004 REPAIR: learn Genexus =='
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
Write-Host '== CORE-0004 REPAIR: ask Knowledge Base =='
$kb = .\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Para que sirve una Knowledge Base?"
$kb | Tee-Object -FilePath core-0004-kb-answer.log
if ($kb -notmatch 'Repositorio central|repositorio central') { throw 'La respuesta de Knowledge Base no fue especifica.' }
if ($kb -notmatch 'Transactions|Transaction') { throw 'La respuesta de Knowledge Base no menciono objetos de GeneXus.' }
if ($kb -notmatch 'Fuentes registradas|Fuente') { throw 'La respuesta no incluyo fuentes.' }
Write-Host '== CORE-0004 REPAIR: ask Transaction =='
$tx = .\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Que es una Transaction?"
$tx | Tee-Object -FilePath core-0004-transaction-answer.log
if ($tx -notmatch 'entidad de negocio') { throw 'La respuesta de Transaction no fue especifica.' }
if ($tx -notmatch 'Fuentes') { throw 'La respuesta de Transaction no incluyo fuentes.' }
Write-Host '== CORE-0004 REPAIR complete =='
