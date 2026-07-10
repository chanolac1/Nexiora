# ARCHITECTURE

The Multimodal layer probes media, delegates extraction, normalizes evidence to `nxknowledge/1`, and stores it under `Knowledge/Multimodal`. Decoder adapters are isolated from the core.

## Decoder and OCR Adapter Layer

La capa `Source/Multimodal/NxDecoderAdapters.c` conecta formatos binarios con la frontera de ingestión. Los adaptadores generan archivos sidecar `.txt` verificables que después son consumidos por `NxMultimodalIngestion`. La ejecución se divide en descubrimiento, planificación, ejecución y validación de evidencia.
