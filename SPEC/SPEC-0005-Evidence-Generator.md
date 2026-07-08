# SPEC-0005 — Evidence Generator Integration

## Purpose
Convert experiment execution results into structured research evidence.

## Responsibilities
- Convert `NxExperimentRunResult` into `NxResearchEvidence`.
- Compare against a baseline when available.
- Generate a Markdown evidence report.
- Classify result as Valid, Inconclusive or Rejected.

## Non-Goals
- It does not approve candidates.
- It does not modify Runtime.
- It does not replace benchmark history.

## Acceptance Criteria
- Evidence can be generated from a completed run.
- Regression detection is represented.
- Report is written to `Research/Reports`.
