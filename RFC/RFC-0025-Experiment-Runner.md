# RFC-0025 — Experiment Runner

## Decision
Nexiora Research Lab shall include a generic Experiment Runner that does not know module-specific internals.

## Motivation
The laboratory needs a single execution pipeline for all future experiments: memory, strings, containers, tensors, compiler and AI.

## Consequences
- Experiments become executable entities.
- Journal and report generation become automatic.
- Module-specific logic will later be added through research plugins.
