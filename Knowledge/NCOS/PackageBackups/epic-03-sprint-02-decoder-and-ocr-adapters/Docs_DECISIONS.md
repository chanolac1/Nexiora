# DECISIONS

## ADR-0004 — Evidence-preserving multimodal ingestion

**Status:** Accepted

Nexiora will not claim understanding of compressed media without a verified decoder. The ingestion core normalizes deterministic content and requires explicit sidecar transcripts until specialized OCR, speech and video adapters are installed. Original source paths and extraction status remain part of the evidence.
