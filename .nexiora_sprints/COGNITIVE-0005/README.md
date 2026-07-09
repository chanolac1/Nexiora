# COGNITIVE-0005 — Unknown Concept Guardrails + Auto Research

Agrega dos comportamientos:

1. Si el concepto especifico no existe en memoria, Nexiora no responde con evidencia lateral.
2. Antes de rendirse, busca una fuente local de auto-investigacion en:
   `Knowledge/Cognitive/AutoResearch/<tema>/<concepto>.txt`

Incluye una fuente de ejemplo para `Genexus/DataSelector`.

## Verificacion

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-cognitive-0005.ps1
```

## Uso

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Que es un DataSelector?"
```

## Commit

```powershell
git add .
git commit -m "COGNITIVE-0005: Add unknown concept auto research guardrails"
git push
```
