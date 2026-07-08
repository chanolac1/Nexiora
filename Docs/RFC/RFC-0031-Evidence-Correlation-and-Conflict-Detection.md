# RFC-0031 — Evidence Correlation and Conflict Detection

## Status
Accepted

## Context
The hypothesis and reasoning engines can propose and score ideas, but a mature autonomous laboratory must also identify when evidence agrees, disagrees, or remains inconclusive.

## Decision
Introduce a dedicated evidence correlation layer and conflict detector.

The correlation layer summarizes evidence by hypothesis and polarity. The conflict detector classifies contradictory evidence into severity levels and explicitly marks cases that require human review.

## Non-goals

- No automatic promotion.
- No model-based judgment.
- No hidden mutation of runtime state.

## Rule
Evidence can change confidence. Conflict can request review. Promotion remains human-guided.
