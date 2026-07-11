# DECISIONS

## ADR-CAP02-003 — Persist conversational context outside the reasoning engine
Conversation state is stored in an auditable `nxconversation/1` file. Follow-up questions are resolved before invoking grounded reasoning, preserving determinism and traceability.
