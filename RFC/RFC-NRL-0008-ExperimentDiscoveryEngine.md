# RFC NRL-0008 — Experiment Discovery Engine

## Status
Accepted

## Motivation
The autonomous research pipeline cannot scale if experiments are hard-coded into the runner. Nexiora needs a catalog layer that can enumerate experiments before execution.

## Decision
Introduce `NxDiscoveryEngine`, a small C module that stores `NxExperimentDescriptor` entries in caller-owned memory and exposes registration, lookup, and predicate-based discovery APIs.

## Non-Goals
- No dynamic allocation.
- No filesystem scanning.
- No plugin loading.
- No automatic promotion.
- No Runtime mutation.

## Safety
The module only catalogs experiments. It does not execute experiments and cannot promote research into Runtime.

## Future Work
NRL-0009 will connect this engine to the autonomous research pipeline so discovery becomes the first real pipeline phase.
