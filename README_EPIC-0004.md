# EPIC-0004 — Evidence Correlation and Conflict Detection Foundation

This epic teaches Nexiora to evaluate whether collected evidence converges, conflicts, or remains insufficient for a hypothesis.

## Added modules

- `NxEvidenceCorrelationEngine`
  - Stores evidence observations associated with hypotheses.
  - Tracks support, contradiction, and neutral evidence.
  - Produces weighted summaries and a correlation verdict.

- `NxConflictDetector`
  - Classifies conflict severity from correlated evidence.
  - Produces human-review reports when evidence is contradictory.

## Principle preserved

Nexiora may recommend and classify evidence. Only a human may promote runtime changes.

## Verification

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\verify-epic-0004.ps1
```

## Commit

```powershell
git add .
git commit -m "EPIC-0004: Evidence Correlation and Conflict Detection Foundation"
git push
```
