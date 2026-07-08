# RFC-0031 — First Autonomous Research Execution

## Status
Accepted

## Context
Nexiora already contains isolated research subsystems: discovery, scheduler, runner, evidence, journal, graph, and promotion. The next architectural milestone is to connect these pieces into the first end-to-end autonomous research run.

## Decision
Introduce `NxAutonomousExecution` as an orchestration layer for the first autonomous research command.

The first execution is intentionally conservative:

- it runs built-in seed experiments;
- it writes a reproducible session folder;
- it generates evidence and promotion reviews;
- it exports the research graph;
- it stops at the human approval gate.

## Non-goals

- No automatic Runtime mutation.
- No LLM integration.
- No unattended promotion.

## Principle
Nexiora may recommend. Only a human may promote.
