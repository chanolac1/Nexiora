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
