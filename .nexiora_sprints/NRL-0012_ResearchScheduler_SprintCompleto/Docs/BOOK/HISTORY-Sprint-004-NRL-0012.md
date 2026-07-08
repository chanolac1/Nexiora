# BOOK Update: NRL-0012 Research Scheduler

Nexiora now includes the first Autonomous Laboratory Foundation component: `NxResearchScheduler`.

The scheduler allows the Research Lab to decide which experiment-oriented task should run next based on deterministic priority, cooldown, max-run, and disabled-state rules.

This is a foundation for future Research Queue, Policy Engine, and Autonomous Research Loop work.

Invariant preserved:

> Nexiora may schedule and recommend. Only a human may promote runtime changes.
