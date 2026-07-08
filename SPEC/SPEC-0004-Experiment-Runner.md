# SPEC-0004 — Experiment Runner

## Status
Implemented as lifecycle skeleton in Nexiora 0.0.11.

## Purpose
The Experiment Runner executes research experiment lifecycles and connects execution to journal and report generation.

## Responsibilities
- Initialize runner context.
- Execute an experiment by ID.
- Emit structured journal events.
- Produce a Markdown report.
- Track completed and failed runs.

## Non-Goals
- It does not compile experiment plugins yet.
- It does not promote candidates.
- It does not replace the benchmark or evidence engines.

## API
- `nx_experiment_runner_initialize`
- `nx_experiment_runner_execute`
- `nx_experiment_runner_write_report`
- `nx_experiment_runner_shutdown`

## Acceptance Criteria
- Unit test validates lifecycle.
- Journal receives execution events.
- Report is generated.
- Research benchmark includes runner lifecycle.
