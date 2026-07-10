# CAPABILITY-01 Sprint-01 Repair2 — Transcript Discovery

## Qué logramos

- Se corrigió la detección de subtítulos generados por yt-dlp con nombres variables como `VIDEO.es.es.vtt` y `VIDEO.en.es.vtt`.
- Se prioriza el idioma solicitado y se acepta cualquier WebVTT válido del mismo video como respaldo.
- Se fuerza una recompilación limpia de `nexiora_web_cognitive.exe` para evitar ejecutar binarios obsoletos por marcas de tiempo del paquete.
- El progreso se muestra cada cinco segundos para conservar visibilidad sin saturar la consola.
- Se añadió una prueba de regresión específica para el nombre real observado.
