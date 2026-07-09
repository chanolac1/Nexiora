# COGNITIVE-0004 — Relevant Evidence Ranking

Mejora el motor cognitivo para ordenar respuestas por relevancia respecto a la pregunta.

Cambios:
- Ranking de fragmentos por terminos de la pregunta.
- Boost cuando varios terminos aparecen en el mismo fragmento.
- Reporte de relevancia por evidencia.
- Mantiene deduplicacion de respuestas.

Prueba principal:
```powershell
.\Build\windows-msvc-release\bin\nexiora_cognitive_batch.exe ask Genexus "Para que sirve una Knowledge Base?"
```

Debe mostrar `Evidencia usada`, `relevancia`, `Confianza` y `Fragmentos unicos usados`.
