# CORE-0001 — Real Learning Core

Agrega un núcleo de aprendizaje inicial que sí intenta adquirir información externa mediante PowerShell:

- Wikipedia OpenSearch + REST summary
- DuckDuckGo Instant Answer
- hints controlados para temas de ingeniería como GeneXus
- memoria persistente en `Knowledge/Topics/<tema>/memory.jsonl`
- respuesta consultable en `Knowledge/Topics/<tema>/answer.txt`

## Comandos

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe aprende Genexus
.\Build\windows-msvc-release\bin\nexiora.exe que sabes Genexus
```

## Verificación

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-core-0001.ps1
```

## Commit

```powershell
git add .
git commit -m "CORE-0001: Add real learning core"
git push
```
