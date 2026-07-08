# RFC-0027: Research Scheduler

## Status
Accepted for Sprint NRL-0012.

## Context
Nexiora can run experiments, generate evidence, publish graph data, and query relationships. The next step is to decide which research task should run next without hardcoding execution order.

## Decision
Introduce `NxResearchScheduler`, a small deterministic scheduler for laboratory tasks. It selects pending tasks by priority, respects cooldown ticks, caps repeated runs, and allows tasks to be disabled.

## Non-goals
- No background threads.
- No automatic runtime promotion.
- No AI-driven prioritization yet.
- No persistent queue storage yet.

## Principle
The scheduler may choose work. Promotion remains human-controlled.
