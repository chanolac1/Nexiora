# Sprint 3 — NRL-0008 Experiment Discovery Engine

## Status
Completed.

## Summary
Added the Experiment Discovery Engine as the first catalog layer for autonomous research.

The Discovery Engine provides:
1. Static-storage initialization
2. Experiment descriptor registration
3. Duplicate ID rejection
4. Capacity enforcement
5. Indexed descriptor access
6. Lookup by experiment ID
7. Predicate-based discovery
8. Built-in predicates for all, ready, and component-filtered discovery

## Architectural Intent
The Runner must not know every experiment explicitly. Discovery becomes the bridge between a growing laboratory catalog and autonomous execution.

## Safety Rule
Discovery does not execute, promote, or mutate Runtime state. It only enumerates research candidates.

Nexiora may recommend. Only a human may promote.

## Tests
- Register and count
- Reject duplicate IDs
- Enforce capacity
- Get by index
- Find by ID
- Discover ready experiments
- Discover experiments by component
- Reset engine state

## Benchmark
Added a lightweight benchmark that registers 128 descriptors and repeatedly discovers ready experiments.
