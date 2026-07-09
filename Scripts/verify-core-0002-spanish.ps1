$ErrorActionPreference = 'Stop'
Write-Host '== CORE-0002: aprendizaje en espanol =='
cmake --preset windows-msvc-release
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
Write-Host '== CORE-0002: aprender Genexus =='
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
Write-Host '== CORE-0002: consultar Genexus =='
$out = .\Build\windows-msvc-release\bin\nexiora.exe que sabes Genexus
$out | Set-Content -Path .\core-0002-spanish-query.log -Encoding UTF8
Write-Host $out
if ($out -notmatch 'Tema: Genexus' -and $out -notmatch 'Tema: GeneXus') { throw 'La consulta no devolvio el tema Genexus.' }
if ($out -notmatch 'Idioma de respuesta: Espanol') { throw 'La respuesta no indica idioma espanol.' }
if ($out -notmatch 'plataforma|low-code|base de conocimiento|GeneXus') { throw 'La respuesta no contiene conocimiento util en espanol sobre GeneXus.' }
Write-Host '== CORE-0002 complete =='
