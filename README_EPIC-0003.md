# EPIC-0003 — Hypothesis & Reasoning Engine Foundation

This epic introduces the first explicit reasoning layer for Nexiora Research Lab.

## Added

- `NxHypothesisEngine`: proposes hypotheses and updates their state from support or contradiction evidence.
- `NxReasoningEngine`: evaluates hypotheses against knowledge facts and emits reasoning conclusions.
- Conflict detection when the same hypothesis has both supporting and contradicting evidence.
- Unit tests for hypothesis lifecycle and reasoning reports.

## Verify

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0003.ps1
```

## Commit

```powershell
git add .
git commit -m "EPIC-0003: Hypothesis and Reasoning Engine Foundation"
git push
```
