# SPEC-0006 — Promotion Pipeline

## Purpose
Convert structured research evidence into a promotion review.

## Responsibilities
- Decide whether an experiment is a candidate.
- Reject regressions automatically.
- Mark inconclusive evidence as needing more data.
- Require human approval for valid promotion candidates.
- Generate a Markdown promotion review.

## Non-Goals
- It does not modify Runtime.
- It does not perform Git commits.
- It does not approve candidates automatically.

## Acceptance Criteria
- Valid evidence becomes Candidate.
- Rejected evidence becomes Rejected.
- Inconclusive evidence becomes NeedsMoreData.
- Candidate always requires human approval.
