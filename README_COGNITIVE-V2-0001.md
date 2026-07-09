# COGNITIVE-V2-0001 — Intent + Strict Concept Answering

Primera entrega de Cognitive Core v2.

Incluye:

- Detección simple de intención: definición, propósito, comparación o consulta general.
- Extracción de concepto principal de la pregunta.
- Filtro estricto: si existe concepto principal, usa solo evidencia que lo menciona explícitamente.
- Auto-investigación local cuando falta evidencia específica.
- Respuesta estructurada con concepto, intención, evidencia, filtro aplicado y confianza.

Caso objetivo:

```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Que es un DataSelector?"
```

Debe responder sobre DataSelector y no mezclar fragmentos laterales de Transaction o Knowledge Base.
