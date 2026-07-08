# EPIC-0005 — First Autonomous Research Execution

This epic connects the existing laboratory subsystems into the first runnable autonomous research flow.

## Main command

```powershell
.\Build\windows-msvc-release\bin\nexiora.exe research run
```

## One-command verification

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0005.ps1
```

The script applies the epic, configures CMake, builds, runs all tests, launches the first autonomous research execution, and opens the generated `graph.svg`.

## Generated session artifacts

```text
Research/Sessions/first_autonomous_execution/
  journal.jsonl
  manifest.snapshot.json
  knowledge.json
  graph.json
  graph.dot
  graph.svg
  summary.txt
  metrics.json
  report.md
```

## Commit

```powershell
git add .
git commit -m "EPIC-0005: First Autonomous Research Execution"
git push
```

## Principle

Nexiora may recommend. Only a human may promote.
