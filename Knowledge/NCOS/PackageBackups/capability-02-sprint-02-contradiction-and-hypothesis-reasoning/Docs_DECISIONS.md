# DECISIONS

## ADR-0008 — Razonamiento fundamentado antes de inferencia generativa

**Estado:** Aceptada

Nexiora debe construir respuestas a partir de evidencia persistida y verificable. La primera etapa usa ranking determinista, fusión de fragmentos y rechazo cuando la evidencia es insuficiente. Las inferencias futuras deberán conservar la trazabilidad de cada conclusión.

## ADR-CAP02-001 — Full test binary materialization gate
Before executing the complete CTest registry, Nexiora must build every registered test executable through the `nexiora_test_suite` target. Partial builds are valid only for focused testing and cannot certify a release.
