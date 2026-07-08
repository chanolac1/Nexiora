# SPEC-NRL-0010 — Research Graph Exporter

## Status
Draft for implementation.

## Purpose
The Research Graph Exporter turns an in-memory `NxResearchGraph` into files that can be inspected by humans and consumed by future tools.

## API
The exporter must provide:

```c
NxResearchGraph_ExportDot(...)
NxResearchGraph_ExportJson(...)
NxResearchGraph_ExportSvg(...)
```

## Requirements
- Accept a constant graph pointer.
- Never mutate the graph.
- Return explicit error codes.
- Reject null arguments.
- Produce readable DOT.
- Produce machine-readable JSON.
- Produce immediately viewable SVG without requiring external tooling.

## Success Criteria
- Unit tests pass.
- DOT, JSON and SVG files are generated.
- A sample tool generates `Artifacts/ResearchGraph/research_graph.svg`.
- The generated SVG can be opened directly on Windows.

## Constraint
The exporter visualizes recommendations and evidence chains. It does not approve or promote runtime changes.
