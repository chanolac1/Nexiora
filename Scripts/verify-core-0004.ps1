$ErrorActionPreference = 'Stop'
Write-Host '== CORE-0004: applying =='
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-core-0004.ps1
Write-Host '== CORE-0004: configure =='
cmake --preset windows-msvc-release
Write-Host '== CORE-0004: build =='
cmake --build --preset release
Write-Host '== CORE-0004: tests =='
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
Write-Host '== CORE-0004: learn Genexus =='
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
Write-Host '== CORE-0004: ask Knowledge Base =='
$kb = .\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Para que sirve una Knowledge Base?"
$kb | Tee-Object -FilePath core-0004-kb-answer.log
if ($kb -notmatch 'repositorio central') { throw 'La respuesta de Knowledge Base no fue especifica.' }
if ($kb -notmatch 'Transactions|Transaction') { throw 'La respuesta de Knowledge Base no menciono objetos de GeneXus.' }
Write-Host '== CORE-0004: ask Transaction =='
$tx = .\Build\windows-msvc-release\bin\nexiora.exe pregunta Genexus "Que es una Transaction?"
$tx | Tee-Object -FilePath core-0004-transaction-answer.log
if ($tx -notmatch 'entidad de negocio') { throw 'La respuesta de Transaction no fue especifica.' }
Write-Host '== CORE-0004 complete =='
