# SPEC — EPIC-0005 First Autonomous Research Execution

## Public API

```c
NxAutonomousExecutionStatus NxAutonomousExecution_RunDefault(
    const char* workspace_root,
    NxAutonomousExecutionResult* result_out);
```

## CLI

```text
nexiora research run
```

## Session output

The command creates:

```text
Research/Sessions/first_autonomous_execution/
```

Required artifacts:

- `journal.jsonl`
- `manifest.snapshot.json`
- `knowledge.json`
- `graph.json`
- `graph.dot`
- `graph.svg`
- `summary.txt`
- `metrics.json`
- `report.md`

## Safety

The command may produce promotion candidates, but no code is promoted automatically.
