#include "Nexiora/Research/NxResearchScheduler.h"

#include <stdio.h>

static int ExpectStatus(
    const char* name,
    NxResearchSchedulerStatus actual,
    NxResearchSchedulerStatus expected)
{
    if (actual != expected)
    {
        printf("%s failed: expected status %d, got %d\n", name, (int)expected, (int)actual);
        return 1;
    }

    return 0;
}

static int TestPrioritySelection(void)
{
    NxResearchScheduler scheduler;
    NxResearchTaskDescriptor task;
    NxResearchTaskDescriptor selected;

    if (ExpectStatus("Init", NxResearchScheduler_Init(&scheduler, 4), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        return 1;
    }

    task.id = 1;
    task.name = "low_priority_experiment";
    task.priority = 10;
    task.max_runs = 1;
    task.completed_runs = 0;
    task.next_tick = 0;
    task.state = NX_RESEARCH_TASK_STATE_PENDING;
    (void)NxResearchScheduler_Add(&scheduler, &task);

    task.id = 2;
    task.name = "high_priority_experiment";
    task.priority = 90;
    task.max_runs = 1;
    task.completed_runs = 0;
    task.next_tick = 0;
    task.state = NX_RESEARCH_TASK_STATE_PENDING;
    (void)NxResearchScheduler_Add(&scheduler, &task);

    if (ExpectStatus("SelectNext", NxResearchScheduler_SelectNext(&scheduler, &selected), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    if (selected.id != 2)
    {
        printf("Expected highest priority task to be selected\n");
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    NxResearchScheduler_Destroy(&scheduler);
    return 0;
}

static int TestCooldownAndCompletion(void)
{
    NxResearchScheduler scheduler;
    NxResearchTaskDescriptor task;
    NxResearchTaskDescriptor selected;

    if (ExpectStatus("Init", NxResearchScheduler_Init(&scheduler, 2), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        return 1;
    }

    task.id = 7;
    task.name = "cooldown_experiment";
    task.priority = 50;
    task.max_runs = 2;
    task.completed_runs = 0;
    task.next_tick = 0;
    task.state = NX_RESEARCH_TASK_STATE_PENDING;

    (void)NxResearchScheduler_Add(&scheduler, &task);

    if (ExpectStatus("First select", NxResearchScheduler_SelectNext(&scheduler, &selected), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    if (ExpectStatus("Mark completed", NxResearchScheduler_MarkCompleted(&scheduler, 7, 5), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    if (ExpectStatus("Cooldown blocks task", NxResearchScheduler_SelectNext(&scheduler, &selected), NX_RESEARCH_SCHEDULER_STATUS_NO_ELIGIBLE_TASK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    NxResearchScheduler_AdvanceTick(&scheduler, 5);

    if (ExpectStatus("Second select", NxResearchScheduler_SelectNext(&scheduler, &selected), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    if (ExpectStatus("Second completion", NxResearchScheduler_MarkCompleted(&scheduler, 7, 0), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    if (ExpectStatus("Completed task is exhausted", NxResearchScheduler_SelectNext(&scheduler, &selected), NX_RESEARCH_SCHEDULER_STATUS_NO_ELIGIBLE_TASK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    NxResearchScheduler_Destroy(&scheduler);
    return 0;
}

static int TestDisable(void)
{
    NxResearchScheduler scheduler;
    NxResearchTaskDescriptor task;
    NxResearchTaskDescriptor selected;

    if (ExpectStatus("Init", NxResearchScheduler_Init(&scheduler, 2), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        return 1;
    }

    task.id = 11;
    task.name = "disabled_experiment";
    task.priority = 100;
    task.max_runs = 0;
    task.completed_runs = 0;
    task.next_tick = 0;
    task.state = NX_RESEARCH_TASK_STATE_PENDING;

    (void)NxResearchScheduler_Add(&scheduler, &task);

    if (ExpectStatus("Disable", NxResearchScheduler_Disable(&scheduler, 11), NX_RESEARCH_SCHEDULER_STATUS_OK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    if (ExpectStatus("Disabled task not selected", NxResearchScheduler_SelectNext(&scheduler, &selected), NX_RESEARCH_SCHEDULER_STATUS_NO_ELIGIBLE_TASK))
    {
        NxResearchScheduler_Destroy(&scheduler);
        return 1;
    }

    NxResearchScheduler_Destroy(&scheduler);
    return 0;
}

int main(void)
{
    if (TestPrioritySelection())
    {
        return 1;
    }

    if (TestCooldownAndCompletion())
    {
        return 1;
    }

    if (TestDisable())
    {
        return 1;
    }

    return 0;
}
