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
