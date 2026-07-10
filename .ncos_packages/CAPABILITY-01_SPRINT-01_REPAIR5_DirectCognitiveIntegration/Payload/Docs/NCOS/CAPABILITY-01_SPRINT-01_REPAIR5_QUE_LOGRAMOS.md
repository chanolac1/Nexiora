# Qué logramos — CAPABILITY-01 Sprint-01 Repair5

- Eliminamos la ejecución frágil de `nexiora_multimodal_cognitive.exe` como proceso hijo.
- El Web Cognitive Pipeline invoca directamente `NxMmci_Analyze()` dentro de `NexioraNCP`.
- Los errores cognitivos conservan su diagnóstico real y se mapean a estados del pipeline.
- El identificador de análisis se obtiene del directorio persistido real.
- Añadimos regresión que convierte WebVTT, crea `nxknowledge/1` y completa integración cognitiva directa.
- Se mantiene la adquisición flexible de subtítulos y la procedencia de la URL.
