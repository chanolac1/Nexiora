# DECISIONS

## ADR-0008 — Razonamiento fundamentado antes de inferencia generativa

**Estado:** Aceptada

Nexiora debe construir respuestas a partir de evidencia persistida y verificable. La primera etapa usa ranking determinista, fusión de fragmentos y rechazo cuando la evidencia es insuficiente. Las inferencias futuras deberán conservar la trazabilidad de cada conclusión.

## ADR-CAP02-002 — Preserve competing hypotheses
Nexiora must retain supported and opposing hypotheses when evidence conflicts instead of selecting an arbitrary answer. Confidence is derived from relevant support and opposition evidence.
