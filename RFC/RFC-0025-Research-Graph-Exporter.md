# RFC-0025 — Research Graph Exporter

## Decision
Nexiora Research Lab will export its internal research graph to external formats.

## Formats
- DOT for Graphviz-compatible tooling.
- JSON for automation, inspection and future query tooling.
- SVG for immediate human-readable visualization.

## Rationale
NRL-0009 created the internal graph model. NRL-0010 makes that graph visible so humans can inspect the evidence chain behind recommendations.

## Non-goals
- No graph query language.
- No persistence database.
- No automatic promotion.
- No runtime coupling.

## Principle
A visible graph improves explainability, but it remains advisory.

Nexiora may recommend. Only a human may promote.
