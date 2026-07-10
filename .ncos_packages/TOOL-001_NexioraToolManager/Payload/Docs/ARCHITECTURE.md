# ARCHITECTURE

The Multimodal layer probes media, delegates extraction, normalizes evidence to `nxknowledge/1`, and stores it under `Knowledge/Multimodal`. Decoder adapters are isolated from the core.

## Decoder and OCR Adapter Layer

La capa `Source/Multimodal/NxDecoderAdapters.c` conecta formatos binarios con la frontera de ingestión. Los adaptadores generan archivos sidecar `.txt` verificables que después son consumidos por `NxMultimodalIngestion`. La ejecución se divide en descubrimiento, planificación, ejecución y validación de evidencia.

## Multimodal Cognitive Integration Engine

`nxknowledge/1 -> evidence chunks -> Concept Registry -> Concept Graph -> grounded query`.

Implementación:

- `Source/Multimodal/NxMultimodalCognitiveIntegration.c`
- `Tools/Multimodal/NxMultimodalCognitiveTool.c`

## Universal Cognitive Pipeline — Web Source Adapter

`NxWebCognitivePipeline` conecta una URL de YouTube con adquisición de subtítulos, normalización `nxknowledge/1` e integración cognitiva. `yt-dlp` es un adaptador externo opcional y auditable; el núcleo C23 conserva planificación, validación, conversión y procedencia.

## Tool Manager
`nexiora_tool` resuelve dependencias ejecutables de capacidades. Catálogo: `Config/Tools`; binarios: `Tools/Managed`; registros: `Tools/Registry`.
