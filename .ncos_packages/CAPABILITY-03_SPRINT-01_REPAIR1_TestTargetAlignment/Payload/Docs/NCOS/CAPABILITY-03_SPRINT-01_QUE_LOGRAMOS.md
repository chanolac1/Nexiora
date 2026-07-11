# Qué logramos — CAPABILITY-03 Sprint-01

Nexiora puede conservar una creencia semántica, actualizarla con nueva evidencia y explicar:

- qué creía antes;
- qué cree ahora;
- qué fuente produjo cada versión;
- qué confianza tiene cada versión;
- por qué cambió la creencia;
- qué versión permanece activa.

No se sobrescribe el pasado: cada cambio queda enlazado mediante `supersedes_id`.


## Repair1 — Test Target Alignment

Pure Apply usa `nexiora_test_suite`, objetivo disponible y estable en el árbol actual. La CLI `nexiora_semantic_memory` y `NxVersionedSemanticMemoryTests` son dependencias obligatorias del objetivo.
