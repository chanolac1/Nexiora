# DECISIONS

## ADR-0004 — Evidence-preserving multimodal ingestion

**Status:** Accepted

Nexiora will not claim understanding of compressed media without a verified decoder. The ingestion core normalizes deterministic content and requires explicit sidecar transcripts until specialized OCR, speech and video adapters are installed. Original source paths and extraction status remain part of the evidence.

## ADR-0005 — Adaptadores multimodales externos verificables

**Estado:** Aceptada

Nexiora integra decodificadores especializados como adaptadores locales, no como capacidades simuladas. Cada adaptador debe descubrir su herramienta, construir un plan reproducible, producir evidencia textual y fallar de forma explícita cuando la dependencia o la salida no estén disponibles. Tesseract se usa para OCR, FFmpeg para demultiplexado de video y whisper-cli para transcripción local.

## ADR-EPIC03-003 — Integración cognitiva basada en evidencia

El contenido multimodal normalizado no se considera comprendido hasta ser segmentado en evidencia persistente. Los conceptos y relaciones derivados conservan la ruta de origen. Las respuestas se rechazan cuando no existe un fragmento coincidente. La extracción inicial es simbólica y determinista; no pretende reemplazar modelos semánticos posteriores.
