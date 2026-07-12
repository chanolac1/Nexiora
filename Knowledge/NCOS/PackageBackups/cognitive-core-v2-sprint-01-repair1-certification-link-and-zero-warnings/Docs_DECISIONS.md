# DECISIONS

## ADR-0024 — Persist knowledge gaps instead of fabricating answers

### Context

A cognitive system must distinguish missing evidence from a valid conclusion. Returning an answer when the available evidence is insufficient would create unsupported knowledge.

### Decision

Represent every unresolved knowledge gap as a persistent `nxgap/1` artifact containing the original question, subject, reason, proposed research queries, allowed sources and objective success criteria.

### Consequences

- Nexiora can reject unsupported answers explicitly.
- Future autonomous research can consume a stable, auditable contract.
- Research is not opened when the grounded reasoning engine reports sufficient evidence.
- Documentation validation remains part of the same transactional package application.

## ADR — Las CLI son artefactos obligatorios de certificación
Una prueba de biblioteca no certifica por sí sola una capacidad orientada al usuario. Cada CLI entregada debe ser dependencia explícita del objetivo Pure Apply correspondiente.

## DEC-CAP02-S05 — Orquestación cognitiva unificada
Nexiora expone una sola fachada de diálogo cognitivo. El motor responde con evidencia cuando puede, conserva contradicciones cuando las detecta y abre un plan `nxgap/1` cuando no existe respaldo suficiente. No se permite degradar una falta de evidencia a una respuesta especulativa.
## ADR NCOS-025 — Git condicionado a certificación
Ninguna entrega se publica hasta revisar un informe con recomendación `APPROVE_GIT`.

