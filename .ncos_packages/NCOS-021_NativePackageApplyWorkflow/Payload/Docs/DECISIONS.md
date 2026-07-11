# DECISIONS

## ADR — Contextual follow-ups must change retrieval semantics
A conversational reference is not resolved by appending a label only. Nexiora must construct a query containing both the active subject and the newly introduced comparison concept, then rerank evidence. Demonstration datasets must represent the capability being tested.

## ADR-CAP02-S03-R3 — Intent-aware evidence thresholds
Focal questions select only evidence tied at the highest relevance score. Comparative questions may include evidence within 20 points of the top score so both compared concepts remain represented.

## ADR-NCOS-021 — Package certification belongs to the Package Manager
Installation, build, warning validation, tests and documentation checks form one transactional operation. External `verify_install.ps1` scripts are not an official installation mechanism. A post-install failure must roll back the exact transaction created by the same `apply` invocation.
