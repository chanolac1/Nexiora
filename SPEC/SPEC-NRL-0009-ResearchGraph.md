# SPEC-NRL-0009 — Research Graph Foundation

## Status
Draft for implementation.

## Purpose
The Research Graph is the first knowledge model for Nexiora Research Lab. It represents relationships among experiments, evidence, journal entries, manifests, hypotheses and promotion recommendations.

## Core Rule
The graph stores relationships and supports traceability. It does not promote runtime changes. Promotion remains a human decision.

## Nodes
Supported node types:

- Experiment
- Evidence
- Journal Entry
- Manifest
- Promotion Recommendation
- Hypothesis
- Unknown

## Edges
Supported edge types:

- produces
- supports
- records
- declares
- recommends
- relates_to
- unknown

## API
The API must allow:

- graph initialization and shutdown
- node insertion
- edge insertion
- node lookup by id
- node lookup by name
- edge lookup by id
- incoming edge counting
- outgoing edge counting

## Constraints

- C API only.
- No Runtime coupling.
- No automatic promotion.
- No filesystem dependency.
- No global mutable graph.

## Success Criteria

- Unit tests pass.
- Graph rejects edges whose endpoints do not exist.
- Graph grows beyond initial capacity.
- Graph can represent an experiment -> evidence -> promotion chain.
