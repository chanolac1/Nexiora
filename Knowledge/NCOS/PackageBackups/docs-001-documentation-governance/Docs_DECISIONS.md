# DECISIONS

## ADR-0023 — Staged Package Manager self-update

### Context

Windows prevents replacing an executable while it is running. Package certification must also complete before the active Package Manager is replaced.

### Decision

Build the candidate as `nexiora_package.next.exe`, certify it and the full suite, then perform a separate atomic replacement after the original process has exited. The final bootstrap may use the old explicit installation flow once; subsequent packages use automatic discovery and native apply.

### Consequences

- No active executable is linked over.
- Failed certification leaves the current Package Manager intact.
- The bootstrap remains auditable and reversible.

## ADR-0023 — Process-isolated package discovery tests

Package discovery tests MUST use a process-unique repository root. Fixed test roots are prohibited because Package Manager registries intentionally persist and can alter subsequent test outcomes.
