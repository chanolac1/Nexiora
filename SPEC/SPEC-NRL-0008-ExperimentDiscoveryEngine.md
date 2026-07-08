# SPEC NRL-0008 — Experiment Discovery Engine

## API

### NxDiscoveryEngine_Init
Initializes an engine with caller-owned descriptor storage.

### NxDiscoveryEngine_Register
Registers one descriptor. IDs must be unique.

### NxDiscoveryEngine_Count
Returns the number of registered descriptors.

### NxDiscoveryEngine_Get
Returns a descriptor by index.

### NxDiscoveryEngine_FindById
Returns a descriptor by stable experiment ID.

### NxDiscoveryEngine_Discover
Enumerates descriptors that match a predicate. The function returns the total number of matches even when the result buffer is smaller than the match count.

## Built-in Predicates
- `NxDiscoveryPredicate_All`
- `NxDiscoveryPredicate_Ready`
- `NxDiscoveryPredicate_Component`

## Constraints
- C-compatible API.
- No heap allocation.
- Deterministic iteration order.
- Runtime-safe; no execution or promotion side effects.
