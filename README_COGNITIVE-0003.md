# COGNITIVE-0003 — Deduplicated Cognitive Answers

Corrige respuestas repetitivas del Cognitive Core. El motor ahora descarta fragmentos duplicados o casi duplicados antes de construir la respuesta.

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-cognitive-0003.ps1
```

## Prueba manual

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ingest-dir Genexus .\Samples\Cognitive\Batch --recursive
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Para que sirve una Knowledge Base?"
```

La salida debe incluir `Fragmentos unicos usados` y no repetir la misma idea varias veces.
