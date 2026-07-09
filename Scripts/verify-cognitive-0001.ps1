$ErrorActionPreference = "Stop"

Write-Host "== COGNITIVE-0001: applying =="
powershell -ExecutionPolicy Bypass -File .\Scripts\apply-cognitive-0001.ps1

Write-Host "== COGNITIVE-0001: configuring =="
cmake --preset windows-msvc-release

Write-Host "== COGNITIVE-0001: building =="
cmake --build --preset release

Write-Host "== COGNITIVE-0001: tests =="
ctest --test-dir .\Build\windows-msvc-release --output-on-failure

Write-Host "== COGNITIVE-0001: creating sample document =="
New-Item -ItemType Directory -Force -Path .\Samples\Cognitive | Out-Null
@"
GeneXus es una plataforma low-code orientada al desarrollo de aplicaciones empresariales.
Una Knowledge Base sirve como repositorio central del conocimiento de una aplicacion.
En una Knowledge Base se definen Transactions, Procedures, Data Providers, APIs, reglas y objetos.
Una Transaction representa una entidad de negocio y ayuda a GeneXus a inferir tablas, relaciones y pantallas.
Un Data Provider sirve para construir y devolver estructuras de datos de forma declarativa.
"@ | Set-Content -Encoding UTF8 .\Samples\Cognitive\genexus_intro.txt

Write-Host "== COGNITIVE-0001: ingest sample =="
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ingest Genexus .\Samples\Cognitive\genexus_intro.txt

Write-Host "== COGNITIVE-0001: ask sample =="
.\Build\windows-msvc-release\bin\nexiora_cognitive.exe ask Genexus "Para que sirve una Knowledge Base?"

if (!(Test-Path .\Knowledge\Cognitive\Topics\genexus\answer.txt)) {
    throw "No se genero Knowledge\Cognitive\Topics\genexus\answer.txt"
}

$answer = Get-Content .\Knowledge\Cognitive\Topics\genexus\answer.txt -Raw
if ($answer -notmatch "Knowledge Base" -or $answer -notmatch "repositorio central") {
    throw "La respuesta cognitiva no contiene la evidencia esperada."
}

Write-Host "== COGNITIVE-0001 complete =="
