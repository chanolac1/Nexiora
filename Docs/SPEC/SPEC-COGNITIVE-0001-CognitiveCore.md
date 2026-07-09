# SPEC COGNITIVE-0001 — Cognitive Core

## Alcance

Primera versión funcional del núcleo cognitivo de Nexiora.

Capacidades incluidas:

- Ingesta de archivos locales textuales (`.txt`, `.md`, `.srt`, `.vtt`, `.csv`, `.json`, código fuente).
- Registro de media (`.png`, `.jpg`, `.mp4`, `.mp3`, etc.) como metadata-only.
- Lectura de sidecars para media (`archivo.mp4.txt`, `archivo.mp4.srt`, `archivo.txt`).
- Extracción básica de fragmentos.
- Extracción básica de conceptos por frecuencia.
- Memoria local por tema.
- Preguntas sobre un tema usando evidencia local.

## Comandos

```powershell
nexiora_cognitive ingest <tema> <archivo>
nexiora_cognitive ask <tema> "<pregunta>"
nexiora_cognitive inspect <tema>
```

## Limitaciones conocidas

- No incluye OCR nativo.
- No incluye transcripción nativa de audio/video.
- No usa embeddings todavía.
- La respuesta se basa en coincidencia léxica simple.

## Criterio de aceptación

Después de ingerir un documento textual, Nexiora debe responder preguntas usando fragmentos de ese documento y guardar la respuesta en `Knowledge/Cognitive/Topics/<tema>/answer.txt`.
