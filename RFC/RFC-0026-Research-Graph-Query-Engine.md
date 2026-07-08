# RFC-0026 — Research Graph Query Engine

## Status
Accepted for Sprint NRL-0011.

## Context
NRL-0009 created the internal research graph and NRL-0010 made it exportable. Nexiora now needs a stable query surface so future systems can ask why a recommendation exists, which evidence supports it, and what experiments produced that evidence.

## Decision
Add a small C API named `NxResearchGraphQuery` that performs read-only queries over `NxResearchGraph` without owning memory and without mutating the graph.

## API principles

- No hidden allocation.
- Caller-owned output buffers.
- Deterministic traversal order.
- Compatible with MSVC C17 and modern C toolchains.
- `UNKNOWN` node/edge type acts as wildcard.

## Consequences
The query engine becomes the bridge between the graph foundation and future components such as:

- Graph Query CLI
- Evidence Explanation Engine
- Promotion Justification Reports
- Autonomous Research Loop
