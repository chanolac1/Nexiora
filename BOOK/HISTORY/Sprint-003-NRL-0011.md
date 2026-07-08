# Sprint 003 — NRL-0011 Research Graph Query Engine

## Summary
Added a read-only query API for the Research Graph.

## Added

- Node type counting.
- Node lookup by type.
- Incoming edge query.
- Outgoing edge query.
- Connected node query.
- Buffer-too-small reporting with total match count.

## Principle preserved
Nexiora may recommend. Only a human may promote.

## Why this matters
The graph can now be used to explain recommendations by traversing relationships between experiments, evidence, journal entries, manifests, and promotion recommendations.
