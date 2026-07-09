# COGNITIVE-0002 - Directory Ingest

## Verify

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-cognitive-0002.ps1
```

## Exercises

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ingest-dir Genexus .\Samples\Cognitive\Batch --recursive
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Para que sirve una Knowledge Base?"
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Que es una Transaction?"
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Que es un Data Provider?"
```

## Your own folder

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ingest-dir MiTema .\Docs\MiTema --recursive
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask MiTema "Que aprendiste?"
```

## Commit

```powershell
git add .
git commit -m "COGNITIVE-0002: Add cognitive directory ingest"
git push
```
