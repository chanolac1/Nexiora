# RFC-0024 — Research Graph Foundation

## Decision
Nexiora Research Lab will represent research knowledge as an explicit graph.

## Rationale
The lab already contains experiments, evidence, journals, manifests and promotion recommendations. Without a relationship model, Nexiora can record events but cannot explain why a recommendation exists.

The graph creates a foundation for explainability:

- What evidence supports this recommendation?
- Which experiment produced this evidence?
- Which manifest declared this experiment?
- Which journal entry recorded the run?

## Non-goals

- No graph query language yet.
- No persistence yet.
- No visualization yet.
- No automatic promotion.

## Principle
Nexiora may recommend. Only a human may promote.
