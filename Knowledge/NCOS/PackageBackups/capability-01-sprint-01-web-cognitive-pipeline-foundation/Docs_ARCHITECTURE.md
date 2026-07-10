# ARCHITECTURE

The Multimodal layer probes media, delegates extraction, normalizes evidence to `nxknowledge/1`, and stores it under `Knowledge/Multimodal`. Decoder adapters are isolated from the core.

## Decoder and OCR Adapter Layer

La capa `Source/Multimodal/NxDecoderAdapters.c` conecta formatos binarios con la frontera de ingestión. Los adaptadores generan archivos sidecar `.txt` verificables que después son consumidos por `NxMultimodalIngestion`. La ejecución se divide en descubrimiento, planificación, ejecución y validación de evidencia.

## Multimodal Cognitive Integration Engine

`nxknowledge/1 -> evidence chunks -> Concept Registry -> Concept Graph -> grounded query`.

Implementación:

- `Source/Multimodal/NxMultimodalCognitiveIntegration.c`
- `Tools/Multimodal/NxMultimodalCognitiveTool.c`
