# CHANGELOG

## 2026-07-10 — EPIC-03 Sprint-01

### Added
- Multimodal format detection by signature and extension.
- Deterministic ingestion for text and Markdown.
- Conservative extraction for simple PDF literal strings.
- WAV metadata parsing.
- Sidecar transcript ingestion for audio, video and images.
- nxknowledge/1 normalized persistence.
- nexiora_ingest CLI and regression tests.

## EPIC-03 Sprint-02 — Decoder and OCR Adapters

- Se agregó descubrimiento verificable de Tesseract, FFmpeg y whisper-cli.
- Se agregó planificación segura de OCR para imágenes.
- Se agregó transcripción de audio mediante whisper-cli cuando existe un modelo local.
- Se agregó extracción de audio de video mediante FFmpeg y transcripción posterior.
- Se agregó la herramienta `nexiora_decode` y pruebas de regresión para herramientas ausentes y argumentos inseguros.

## EPIC-03 Sprint-03 — Multimodal Cognitive Integration

- Segmentación determinista de conocimiento multimodal.
- Evidencia persistente por fragmento y procedencia.
- Integración con Concept Registry y Concept Graph.
- Consulta fundamentada con fragmento, ruta y confianza.
- CLI `nexiora_multimodal_cognitive`.

## CAPABILITY-01 Sprint-01 — Web Cognitive Pipeline Foundation

- Ingestión cognitiva inicial desde URLs de YouTube mediante subtítulos verificables.
- Plan auditable basado en `yt-dlp`, sin descarga silenciosa de video.
- Conversión determinista WebVTT a `nxknowledge/1` con procedencia URL.
- Integración con el Multimodal Cognitive Integration Engine.
- CLI `nexiora_web_cognitive` y pruebas de regresión.

## TOOL-001 — Nexiora Tool Manager
- Catálogo local de herramientas.
- Instalación administrada de yt-dlp.
- Registro de versión, origen y SHA-256.
- Verificación, eliminación y rollback local del binario anterior.
