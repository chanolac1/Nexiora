# DECISIONS

## ADR — Contextual follow-ups must change retrieval semantics
A conversational reference is not resolved by appending a label only. Nexiora must construct a query containing both the active subject and the newly introduced comparison concept, then rerank evidence. Demonstration datasets must represent the capability being tested.

## ADR-CAP02-S03-R3 — Intent-aware evidence thresholds
Focal questions select only evidence tied at the highest relevance score. Comparative questions may include evidence within 20 points of the top score so both compared concepts remain represented.

## DEC-CAP03-001 — Las creencias no se sobrescriben
Toda actualización crea una nueva versión activa y preserva la versión anterior como `superseded`. La procedencia y la confianza forman parte del contrato persistente.

## ADR — Consolidación auditable y no destructiva
La consolidación crea una nueva versión; nunca reescribe ni elimina la historia. Las contradicciones se conservan y bloquean el refuerzo automático.
