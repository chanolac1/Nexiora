# SPEC-0003 — Journal Engine

## Status
Draft implemented in Nexiora 0.0.10.

## Purpose
The Journal Engine records structured research events for Nexiora Research Lab.

## Responsibilities
- Create structured journal events.
- Persist events in append-only text format.
- Count persisted events for validation.
- Preserve experiment traceability.

## Non-Goals
- It is not a general logger.
- It is not the final binary journal format.
- It does not replace the Evidence Engine.

## API
- `nx_journal_open`
- `nx_journal_write_event`
- `nx_journal_read_count`
- `nx_journal_close`
- `nx_journal_make_event`

## Acceptance Criteria
- Unit tests pass.
- Journal writes are append-only.
- Research benchmark can measure journal write cost.
