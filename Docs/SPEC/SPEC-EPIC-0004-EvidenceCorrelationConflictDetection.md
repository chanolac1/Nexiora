# SPEC — EPIC-0004 Evidence Correlation and Conflict Detection

## Components

### NxEvidenceCorrelationEngine

Responsibilities:

- Add evidence observations.
- Associate observations with a hypothesis identifier.
- Preserve evidence keys for traceability.
- Summarize support, contradiction, and neutral evidence.
- Derive a verdict: insufficient, convergent, conflicting, or neutral.

### NxConflictDetector

Responsibilities:

- Classify conflict severity.
- Build reports suitable for review by later autonomous loops and CLI commands.
- Mark medium and high conflicts as requiring human review.

## Safety

Conflict detection never promotes code. It only produces structured reports.
