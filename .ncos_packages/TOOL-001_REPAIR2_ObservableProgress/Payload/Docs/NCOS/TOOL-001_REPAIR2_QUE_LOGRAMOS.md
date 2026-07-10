# TOOL-001 Repair2 — Observable Progress

## Qué logramos

El Web Cognitive Pipeline ahora informa progreso observable durante aprendizaje desde YouTube.

- Etapas y porcentaje explícitos.
- Tiempo transcurrido durante procesos externos.
- Heartbeat cada segundo mientras yt-dlp o el análisis cognitivo continúan activos.
- Salida inmediata mediante stderr sin buffering.
- Ejecución directa sin shell preservada.
- Los fallos conservan visible la última etapa alcanzada.

No se simula porcentaje basado en bytes cuando la herramienta externa no ofrece una métrica estable; los porcentajes representan etapas deterministas del pipeline.
