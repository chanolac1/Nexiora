# RFC-0028: Autonomous Laboratory Foundation

## Status
Accepted

## Context
Nexiora has experiment execution, evidence generation, promotion review, scheduling, and research graph primitives. The next step is to coordinate those pieces into an autonomous laboratory foundation without crossing the human-promotion boundary.

## Decision
Introduce four small modules:

- `NxResearchQueue`: ordered queue of experiment work items.
- `NxResearchPolicy`: rule evaluation before execution.
- `NxResearchSession`: summary of one bounded research session.
- `NxAutonomousResearchLoop`: bounded loop that consumes queue items through policy and callbacks.

## Non-goals

- No automatic runtime promotion.
- No LLM integration.
- No unbounded background daemon.
- No persistent database yet.

## Consequences

Nexiora can now execute a bounded autonomous research cycle and produce a session summary while preserving human control over promotion.
