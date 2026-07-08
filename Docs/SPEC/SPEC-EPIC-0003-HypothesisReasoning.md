# SPEC — EPIC-0003 Hypothesis & Reasoning Engine Foundation

## NxHypothesisEngine

Stores hypotheses with confidence, support count, contradiction count, and derived state.

States:

- Proposed
- Supported
- Contested
- Rejected

## NxReasoningEngine

Evaluates knowledge facts against hypotheses and produces conclusions:

- Fact supports hypothesis
- Fact contradicts hypothesis
- Conflict detected

The engine only reasons and reports. It never modifies the Runtime or executes promotion.
