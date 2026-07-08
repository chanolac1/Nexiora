# SPEC NRL-0012: Research Scheduler

## Module
`NxResearchScheduler`

## Responsibilities
- Store research task descriptors.
- Select the next eligible task by priority.
- Track completed runs.
- Respect max-run limits.
- Respect cooldown ticks.
- Disable tasks explicitly.

## Selection rule
Among eligible tasks, the task with the highest numeric priority is selected.

A task is eligible when:
- its state is pending,
- it is not disabled,
- it has not reached `max_runs`, unless `max_runs == 0`,
- `next_tick <= current_tick`.

## API
- `NxResearchScheduler_Init`
- `NxResearchScheduler_Destroy`
- `NxResearchScheduler_Add`
- `NxResearchScheduler_Count`
- `NxResearchScheduler_Get`
- `NxResearchScheduler_SelectNext`
- `NxResearchScheduler_MarkCompleted`
- `NxResearchScheduler_Disable`
- `NxResearchScheduler_AdvanceTick`

## Safety
This module does not allocate after initialization and does not start asynchronous work.
