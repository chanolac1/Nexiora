# SPEC-0001 — Laboratory Kernel

## Purpose
Provide the first kernel of the Nexiora Research Lab.

## Public API
- `nx_research_kernel_initialize`
- `nx_research_kernel_shutdown`
- `nx_research_kernel_create_experiment`
- `nx_research_kernel_write_registry`
- `nx_research_kernel_append_journal`

## Contracts
- Experiments are registered only in an initialized kernel.
- Illegal state transitions return `NX_ERROR_UNSUPPORTED`.
- Registry and journal are append/persist mechanisms, not approval mechanisms.

## Acceptance Criteria
- Compiles on Windows and Linux.
- Unit tests pass.
- `nexiora_bench --module research` runs.
